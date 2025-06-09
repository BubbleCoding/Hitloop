from flask import Blueprint, request, jsonify, render_template
from datetime import datetime, timedelta
import time
from threading import Lock
from . import db
from .models import Scanner, Beacon, RssiValue, ScannerMovement
from sqlalchemy import func

main_bp = Blueprint('main', __name__)

# --- In-memory data store & synchronization ---
devices_data = {}
device_configs = {}
SCAN_INTERVAL_SECONDS = 8
next_slot_time_ms = int(time.time() * 1000)
time_lock = Lock()


def RSSI_to_distance(RSSI):
    """Converts RSSI value to a qualitative distance."""
    distance = ""
    RSSI = RSSI * -1
    if RSSI <= 65:
        distance = "close"
    elif 65 < RSSI < 80:
        distance = "midrange"
    else: # RSSI >= 80
        distance = "far"
    return distance

@main_bp.route('/')
def index():
    return render_template('index.html', devices=devices_data)

@main_bp.route('/data', methods=['POST'])
def receive_data():
    global next_slot_time_ms
    data = request.json
    
    # Standardize scanner ID lookup
    scanner_id = data.get("scanner_id") or data.get("Scanner name")

    beacons_payload = data.get("beacons")
    movement_payload = data.get("movement")

    if not scanner_id:
        print(f"Received invalid data payload: {data}")
        return jsonify({"status": "error", "message": "Missing scanner identifier in payload"}), 400

    if not data.get("simulated"):
        scanner = Scanner.query.filter_by(name=scanner_id).first()
        if not scanner:
            scanner = Scanner(name=scanner_id)
            db.session.add(scanner)
            db.session.commit()

        # Only store dict-based movement payloads in the DB
        if isinstance(movement_payload, dict):
            avg_angle_xz = movement_payload.get('avgAngleXZ')
            avg_angle_yz = movement_payload.get('avgAngleYZ')
            total_movement = movement_payload.get('totalMovement')
            if avg_angle_xz is not None and avg_angle_yz is not None and total_movement is not None:
                movement_record = ScannerMovement(
                    avg_angle_xz=avg_angle_xz,
                    avg_angle_yz=avg_angle_yz,
                    total_movement=total_movement,
                    scanner_id=scanner.id
                )
                db.session.add(movement_record)

        # Only store list-based beacon payloads in the DB
        if isinstance(beacons_payload, list):
            for beacon_info in beacons_payload:
                rssi = beacon_info.get("rssi")
                beacon_name = beacon_info.get("name")
                if rssi is not None and beacon_name is not None:
                    beacon = Beacon.query.filter_by(name=beacon_name).first()
                    if not beacon:
                        beacon = Beacon(name=beacon_name)
                        db.session.add(beacon)
                        db.session.commit()
                    
                    rssi_record = RssiValue(rssi=rssi, scanner_id=scanner.id, beacon_id=beacon.id)
                    db.session.add(rssi_record)
        
        db.session.commit()

    # --- Standardize and update in-memory data for live view ---
    if scanner_id not in devices_data:
        devices_data[scanner_id] = {"beacons_observed": {}, "movement": {}}
    
    devices_data[scanner_id]["timestamp"] = datetime.utcnow().isoformat() + "Z"
    
    # Standardize movement payload
    if isinstance(movement_payload, dict):
        devices_data[scanner_id]["movement"] = movement_payload
    elif isinstance(movement_payload, (int, float)):
        # Convert simulated movement number to the dict format expected by the frontend
        devices_data[scanner_id]["movement"] = {
            "avgAngleXZ": 0,
            "avgAngleYZ": 0,
            "totalMovement": movement_payload
        }

    # Standardize beacons payload
    devices_data[scanner_id]["beacons_observed"].clear()
    if isinstance(beacons_payload, dict): # From simulation
        for beacon_key, beacon_info in beacons_payload.items():
            rssi = beacon_info.get("RSSI")
            if rssi is not None:
                devices_data[scanner_id]["beacons_observed"][beacon_key] = {
                    "rssi": rssi,
                    "beacon_name": beacon_info.get("Beacon name", beacon_key),
                    "distance": RSSI_to_distance(rssi)
                }
    elif isinstance(beacons_payload, list): # From real device
        for beacon_info in beacons_payload:
            rssi = beacon_info.get("rssi")
            beacon_name = beacon_info.get("name")
            if rssi is not None and beacon_name is not None:
                devices_data[scanner_id]["beacons_observed"][beacon_name] = {
                    "rssi": rssi,
                    "beacon_name": beacon_name,
                    "distance": RSSI_to_distance(rssi)
                }

    with time_lock:
        current_time_ms = int(time.time() * 1000)
        
        if next_slot_time_ms < current_time_ms:
            slots_passed = current_time_ms // (SCAN_INTERVAL_SECONDS * 1000)
            next_slot_time_ms = (slots_passed + 1) * (SCAN_INTERVAL_SECONDS * 1000)

        wait_ms = next_slot_time_ms - current_time_ms
        next_slot_time_ms += SCAN_INTERVAL_SECONDS * 1000

    control_payload = {
        "wait_ms": wait_ms
    }

    # If a configuration exists for this device, send it once and then remove it.
    if scanner_id in device_configs:
        config_to_send = device_configs.pop(scanner_id)
        if 'led_behavior' in config_to_send:
            control_payload['led_behavior'] = config_to_send['led_behavior']
        if 'vibration_behavior' in config_to_send:
            control_payload['vibration_behavior'] = config_to_send['vibration_behavior']

    return jsonify(control_payload), 200



@main_bp.route('/control')
def control_page():
    return render_template('control.html')

@main_bp.route('/simulation')
def simulation_page():
    return render_template('simulation.html')

@main_bp.route('/configure/<string:scanner_id>', methods=['POST'])
def configure_scanner(scanner_id):
    """
    Sets the LED and vibration behavior for a specific scanner.
    """
    data = request.json
    led_behavior = data.get('led_behavior')
    vibration_behavior = data.get('vibration_behavior')

    if not led_behavior and not vibration_behavior:
        return jsonify({"status": "error", "message": "No configuration data provided"}), 400

    if scanner_id not in device_configs:
        device_configs[scanner_id] = {}

    if led_behavior:
        device_configs[scanner_id]['led_behavior'] = led_behavior
    if vibration_behavior:
        device_configs[scanner_id]['vibration_behavior'] = vibration_behavior

    return jsonify({
        "status": "success",
        "message": f"Configuration for {scanner_id} updated.",
        "new_config": device_configs[scanner_id]
    }), 200

@main_bp.route('/devices', methods=['GET'])
def get_all_devices():
    """
    Returns a unified list of active devices.
    - Real devices are fetched from the database based on recent activity.
    - Simulated devices are pulled from the in-memory dictionary.
    - In-memory data takes precedence to ensure the live view is current.
    """
    five_minutes_ago = datetime.utcnow() - timedelta(minutes=5)

    # 1. Get real, active scanners from the database
    recent_rssi_ids = db.session.query(RssiValue.scanner_id).filter(RssiValue.timestamp >= five_minutes_ago).distinct()
    recent_movement_ids = db.session.query(ScannerMovement.scanner_id).filter(ScannerMovement.timestamp >= five_minutes_ago).distinct()
    active_db_scanner_ids = recent_rssi_ids.union(recent_movement_ids)
    active_db_scanners = db.session.query(Scanner).filter(Scanner.id.in_(active_db_scanner_ids)).all()

    db_devices = {}
    for scanner in active_db_scanners:
        scanner_data = {
            "timestamp": None,
            "movement": {},
            "beacons_observed": {}
        }
        latest_movement = ScannerMovement.query.filter_by(scanner_id=scanner.id).order_by(ScannerMovement.timestamp.desc()).first()
        latest_rssi_timestamp = db.session.query(func.max(RssiValue.timestamp)).filter_by(scanner_id=scanner.id).scalar()

        if latest_movement:
            scanner_data["movement"] = {
                "avgAngleXZ": latest_movement.avg_angle_xz,
                "avgAngleYZ": latest_movement.avg_angle_yz,
                "totalMovement": latest_movement.total_movement,
            }
        if latest_rssi_timestamp:
            latest_rssi_records = RssiValue.query.filter_by(scanner_id=scanner.id, timestamp=latest_rssi_timestamp).all()
            for record in latest_rssi_records:
                beacon = Beacon.query.get(record.beacon_id)
                if beacon:
                    scanner_data["beacons_observed"][beacon.name] = {
                        "rssi": record.rssi,
                        "beacon_name": beacon.name,
                        "distance": RSSI_to_distance(record.rssi),
                    }
        
        last_update_time = max(
            latest_movement.timestamp if latest_movement else datetime.min,
            latest_rssi_timestamp if latest_rssi_timestamp else datetime.min
        )
        if last_update_time != datetime.min:
            scanner_data["timestamp"] = last_update_time.isoformat() + "Z"
        
        db_devices[scanner.name] = scanner_data

    # 2. Merge with in-memory data (which includes simulated devices)
    # The spread operator `**` merges dictionaries, with keys from the second
    # dictionary (devices_data) overwriting keys from the first (db_devices).
    unified_devices = {**db_devices, **devices_data}
    
    return jsonify(unified_devices)

@main_bp.route('/scanners', methods=['GET'])
def get_all_scanners():
    """
    Returns a list of scanners that were active in the last 5 minutes,
    with their latest movement and beacon data.
    """
    five_minutes_ago = datetime.utcnow() - timedelta(seconds=30)

    # Subquery for scanners with recent RSSI values
    recent_rssi_scanner_ids = db.session.query(RssiValue.scanner_id).filter(RssiValue.timestamp >= five_minutes_ago).distinct()

    # Subquery for scanners with recent movements
    recent_movement_scanner_ids = db.session.query(ScannerMovement.scanner_id).filter(ScannerMovement.timestamp >= five_minutes_ago).distinct()
    
    # Union of scanner_ids from both subqueries
    active_scanner_ids = recent_rssi_scanner_ids.union(recent_movement_scanner_ids)
    
    # Query for the actual scanner objects that have an ID in the active list
    active_scanners = db.session.query(Scanner).filter(Scanner.id.in_(active_scanner_ids)).all()

    devices_payload = {}
    for scanner in active_scanners:
        scanner_data = {
            "timestamp": None,
            "movement": {},
            "beacons_observed": {}
        }

        # Get latest movement for this scanner
        latest_movement = ScannerMovement.query.filter_by(scanner_id=scanner.id).order_by(ScannerMovement.timestamp.desc()).first()

        # Get the timestamp of the last RSSI update for this scanner
        latest_rssi_timestamp = db.session.query(func.max(RssiValue.timestamp)).filter_by(scanner_id=scanner.id).scalar()

        if latest_movement:
            scanner_data["movement"] = {
                "avgAngleXZ": latest_movement.avg_angle_xz,
                "avgAngleYZ": latest_movement.avg_angle_yz,
                "totalMovement": latest_movement.total_movement,
            }

        if latest_rssi_timestamp:
            # Get all records from the latest beacon update for this scanner
            latest_rssi_records = RssiValue.query.filter_by(
                scanner_id=scanner.id, timestamp=latest_rssi_timestamp
            ).all()
            for record in latest_rssi_records:
                beacon = Beacon.query.get(record.beacon_id)
                if beacon:
                    scanner_data["beacons_observed"][beacon.name] = {
                        "rssi": record.rssi,
                        "beacon_name": beacon.name,
                        "distance": RSSI_to_distance(record.rssi),
                    }

        # Determine the most recent timestamp for this scanner's activity
        last_update_time = None
        if latest_movement and latest_rssi_timestamp:
            last_update_time = max(latest_movement.timestamp, latest_rssi_timestamp)
        elif latest_movement:
            last_update_time = latest_movement.timestamp
        elif latest_rssi_timestamp:
            last_update_time = latest_rssi_timestamp

        if last_update_time:
            scanner_data["timestamp"] = last_update_time.isoformat() + "Z"

        devices_payload[scanner.name] = scanner_data
    
    return jsonify(devices_payload)

@main_bp.route('/configure_device')
def configure_device_page():
    return render_template('configure_device.html')

@main_bp.route('/reset_devices', methods=['GET'])
def reset_devices_data():
    global devices_data, device_configs
    devices_data.clear()
    device_configs.clear()
    print("All device data and configurations have been cleared.")
    return jsonify({"status": "success", "message": "All device data and configurations cleared."}), 200 