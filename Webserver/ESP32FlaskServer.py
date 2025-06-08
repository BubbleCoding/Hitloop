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
    accel_x = db.Column(db.Float, nullable=False)
    accel_y = db.Column(db.Float, nullable=False)
    accel_z = db.Column(db.Float, nullable=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    scanner_id = db.Column(db.Integer, db.ForeignKey('scanner.id'), nullable=False)


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
    scanner_name = data.get("Scanner name")
    beacons_payload = data.get("beacons")
    movement_payload = data.get("movement")

    if not scanner_name:
        return jsonify({"status": "error", "message": "Missing 'Scanner name' in payload"}), 400

    # --- Step 1: Store data in the database (if not simulated) ---
    if not data.get("simulated"):
        scanner = Scanner.query.filter_by(name=scanner_name).first()
        if not scanner:
            scanner = Scanner(name=scanner_name)
            db.session.add(scanner)
            db.session.commit()

        if isinstance(movement_payload, dict):
            accel_x = movement_payload.get('accelerationX')
            accel_y = movement_payload.get('accelerationY')
            accel_z = movement_payload.get('accelerationZ')
            if accel_x is not None and accel_y is not None and accel_z is not None:
                movement_record = ScannerMovement(
                    accel_x=accel_x, 
                    accel_y=accel_y, 
                    accel_z=accel_z, 
                    scanner_id=scanner.id
                )
                db.session.add(movement_record)

        if isinstance(beacons_payload, dict):
            for beacon_name, beacon_info in beacons_payload.items():
                rssi = beacon_info.get("RSSI")
                
                beacon = Beacon.query.filter_by(name=beacon_name).first()
                if not beacon:
                    beacon = Beacon(name=beacon_name)
                    db.session.add(beacon)
                    db.session.commit()
                
                rssi_record = RssiValue(rssi=rssi, scanner_id=scanner.id, beacon_id=beacon.id)
                db.session.add(rssi_record)

        db.session.commit()

    # --- Step 2: Update in-memory data for live view ---
    if scanner_name not in devices_data:
        devices_data[scanner_name] = {"beacons_observed": {}, "movement": {}}
    
    devices_data[scanner_name]["timestamp"] = datetime.utcnow().isoformat()
    
    if isinstance(movement_payload, dict):
        devices_data[scanner_name]["movement"] = movement_payload
    
    if isinstance(beacons_payload, dict):
        devices_data[scanner_name]["beacons_observed"].clear() # Keep the live view current
        for beacon_key, beacon_info in beacons_payload.items():
            rssi = beacon_info.get("RSSI")
            beacon_name = beacon_info.get("Beacon name")
            if rssi is not None and beacon_name is not None:
                devices_data[scanner_name]["beacons_observed"][beacon_key] = {
                    "rssi": rssi,
                    "beacon_name": beacon_name,
                    "distance": RSSI_to_distance(rssi)
                }

    return jsonify({"status": "success"}), 200

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
    with app.app_context():
        db.create_all()
    app.run(host='0.0.0.0', port=5000, debug=True)

    # Know issue here: This does not work for some reason use: 
    # flask --app esp32 run --host=0.0.0.0 --port=5000
    # To run the server instead

