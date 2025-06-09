from flask import Blueprint, request, jsonify, render_template
from datetime import datetime
import time
from threading import Lock
from . import db
from .models import Scanner, Beacon, RssiValue, ScannerMovement

main_bp = Blueprint('main', __name__)

# --- In-memory data store & synchronization ---
devices_data = {}
device_configs = {}
SCAN_INTERVAL_SECONDS = 5
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
    scanner_id = data.get("scanner_id")
    beacons_payload = data.get("beacons")
    movement_payload = data.get("movement")

    if not scanner_id:
        print(f"Received invalid data payload: {data}")
        return jsonify({"status": "error", "message": "Missing 'scanner_id' in payload"}), 400

    if not data.get("simulated"):
        scanner = Scanner.query.filter_by(name=scanner_id).first()
        if not scanner:
            scanner = Scanner(name=scanner_id)
            db.session.add(scanner)
            db.session.commit()

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

    if scanner_id not in devices_data:
        devices_data[scanner_id] = {"beacons_observed": {}, "movement": {}}
    
    devices_data[scanner_id]["timestamp"] = datetime.utcnow().isoformat() + "Z"
    
    if isinstance(movement_payload, dict):
        devices_data[scanner_id]["movement"] = movement_payload
    
    if isinstance(beacons_payload, list):
        devices_data[scanner_id]["beacons_observed"].clear()
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

@main_bp.route('/devices', methods=['GET'])
def get_all_devices():
    return jsonify(devices_data)

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