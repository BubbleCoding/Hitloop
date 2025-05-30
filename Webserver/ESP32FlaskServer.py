from flask import Flask, request, jsonify, render_template
from datetime import datetime

app = Flask(__name__)

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

@app.route('/data', methods=['POST'])
def receive_data():
    data = request.json
    scanner_id = data.get("Scanner name")
    beacons_payload = data.get("beacons") # Expect a 'beacons' object in the payload

    if not scanner_id:
        return jsonify({"status": "error", "message": "Missing 'Scanner name' in payload"}), 400
    
    if not isinstance(beacons_payload, dict):
        return jsonify({"status": "error", "message": "'beacons' field must be an object"}), 400

    # Initialize scanner entry if it's new
    if scanner_id not in devices_data:
        devices_data[scanner_id] = {
            "beacons_observed": {}
        }
    
    # Update timestamp for the scanner
    devices_data[scanner_id]["timestamp"] = datetime.utcnow().isoformat()

    # Process each beacon reported by the scanner
    for beacon_key, beacon_info in beacons_payload.items():
        rssi = beacon_info.get("RSSI")
        beacon_name = beacon_info.get("Beacon name")
        
        if rssi is None or beacon_name is None:
            print(f"Warning: Missing RSSI or Beacon name for beacon '{beacon_key}' from scanner '{scanner_id}'")
            continue  # Skip this beacon if data is incomplete
        
        devices_data[scanner_id]["beacons_observed"][beacon_key] = {
            "rssi": rssi,
            "beacon_name": beacon_name,
            "distance": RSSI_to_distance(rssi) # Calculate distance based on RSSI
        }
        # print(f"Scanner {scanner_id} updated beacon {beacon_key}: RSSI={rssi}, Name={beacon_name}")

    # print(f"Updated data for {scanner_id}: {devices_data[scanner_id]}")
    return jsonify({"status": "success"}), 200

@app.route('/')
def index():
    return render_template('index.html', devices=devices_data)

@app.route('/devices', methods=['GET'])
def get_all_devices():
    return jsonify(devices_data)

@app.route('/simulation')
def simulation_page():
    return render_template('simulation.html')

@app.route('/reset_devices', methods=['POST'])
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

