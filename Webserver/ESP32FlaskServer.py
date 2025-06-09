from flask import Flask, request, jsonify, render_template
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import os

app = Flask(__name__)

# --- Database Configuration ---
basedir = os.path.abspath(os.path.dirname(__file__))
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + os.path.join(basedir, 'hitloop.db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

# --- Database Models ---
class Scanner(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), unique=True, nullable=False)
    rssi_values = db.relationship('RssiValue', backref='scanner', lazy=True)
    movements = db.relationship('ScannerMovement', backref='scanner', lazy=True)

class Beacon(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), unique=True, nullable=False)
    rssi_values = db.relationship('RssiValue', backref='beacon', lazy=True)

class RssiValue(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    rssi = db.Column(db.Integer, nullable=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    scanner_id = db.Column(db.Integer, db.ForeignKey('scanner.id'), nullable=False)
    beacon_id = db.Column(db.Integer, db.ForeignKey('beacon.id'), nullable=False)

class ScannerMovement(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    avg_angle_xz = db.Column(db.Float, nullable=False)
    avg_angle_yz = db.Column(db.Float, nullable=False)
    total_movement = db.Column(db.Float, nullable=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    scanner_id = db.Column(db.Integer, db.ForeignKey('scanner.id'), nullable=False)


# Create the database tables if they don't exist.
# This is safe to run on every startup, as it won't recreate existing tables.
with app.app_context():
    db.create_all()


# Stores all devices with their last known RSSI, BLE device name, and timestamp
devices_data = {}


# Sample values of RSSI to distance
def RSSI_to_distance(RSSI):
    distance = ""
    RSSI = RSSI * -1
    if RSSI <= 65:
        distance = "close"
    if RSSI > 65 and RSSI < 80:
        distance = "midrange"
    if RSSI >= 80:
        distance = "far"
    return distance

@app.route('/')
def index():
    return render_template('index.html', devices=devices_data)

@app.route('/data', methods=['POST'])
def receive_data():
    data = request.json
    # Align the key with what the firmware is sending
    scanner_id = data.get("scanner_id")
    beacons_payload = data.get("beacons")
    movement_payload = data.get("movement")

    if not scanner_id:
        # Add logging to see the invalid payload
        print(f"Received invalid data payload: {data}")
        return jsonify({"status": "error", "message": "Missing 'scanner_id' in payload"}), 400

    # --- Step 1: Store data in the database (if not simulated) ---
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

        if isinstance(beacons_payload, list): # The firmware now sends a list
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

    # --- Step 2: Update in-memory data for live view ---
    if scanner_id not in devices_data:
        devices_data[scanner_id] = {"beacons_observed": {}, "movement": {}}
    
    devices_data[scanner_id]["timestamp"] = datetime.utcnow().isoformat() + "Z"
    
    if isinstance(movement_payload, dict):
        devices_data[scanner_id]["movement"] = movement_payload
    
    if isinstance(beacons_payload, list): # The firmware now sends a list
        devices_data[scanner_id]["beacons_observed"].clear() # Keep the live view current
        for beacon_info in beacons_payload:
            rssi = beacon_info.get("rssi")
            beacon_name = beacon_info.get("name")
            if rssi is not None and beacon_name is not None:
                # Use beacon_name as the key for uniqueness in the live view
                devices_data[scanner_id]["beacons_observed"][beacon_name] = {
                    "rssi": rssi,
                    "beacon_name": beacon_name,
                    "distance": RSSI_to_distance(rssi)
                }

    # --- Step 3: Send back control commands ---
    control_payload = {
        "led_behavior": {
            "type": "HeartBeat",
            "params": {
                "color": "#0000FF"  # Blue
            }
        },
        "vibration_behavior": {
            "type": "Off",
            "params": {
                "intensity": 200,
                "frequency": 2
            }
        }
    }
    return jsonify(control_payload), 200

@app.route('/devices', methods=['GET'])
def get_all_devices():
    return jsonify(devices_data)

@app.route('/simulation')
def simulation_page():
    return render_template('simulation.html')

@app.route('/configure_device')
def configure_device_page():
    return render_template('configure_device.html')

@app.route('/reset_devices', methods=['GET'])
def reset_devices_data():
    global devices_data
    devices_data.clear()
    print("All device data has been cleared.")
    return jsonify({"status": "success", "message": "All device data cleared."}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)

    # Know issue here: This does not work for some reason use: 
    # flask --app esp32 run --host=0.0.0.0 --port=5000
    # To run the server instead

