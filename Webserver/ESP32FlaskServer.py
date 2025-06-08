from flask import Flask, request, jsonify, render_template
from datetime import datetime
import json

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

@app.route('/')
def index():
    return render_template('index.html', devices=devices_data)

@app.route('/data', methods=['POST'])
def receive_data():
    data = request.json
    scanner_id = data.get("Scanner name")
    beacons_payload = data.get("beacons") # Get the nested beacons object
    movement_payload = data.get("movement")

    if not scanner_id:
        return jsonify({"status": "error", "message": "Missing 'Scanner name' in payload"}), 400
    
    # beacons_payload can be an empty dict if no beacons are seen, which is fine.
    # movement_payload can be 0 or a positive number.

    if scanner_id not in devices_data:
        devices_data[scanner_id] = {
            "beacons_observed": {},
            # Initialize movement for new scanners, can be overwritten if payload has it
            "movement": 0 
        }
    
    devices_data[scanner_id]["timestamp"] = datetime.utcnow().isoformat()
    
    # Update movement if present in payload
    if movement_payload is not None: # Check if movement data was sent
        devices_data[scanner_id]["movement"] = movement_payload
    elif "movement" not in devices_data[scanner_id]: # Ensure it has a default if never sent
        devices_data[scanner_id]["movement"] = 0

    # Process beacons if the payload exists and is a dictionary
    if isinstance(beacons_payload, dict):
        # Clear previous beacons for this scanner before adding new ones for this update, 
        # or only update existing ones if preferred.
        # For simplicity, let's assume the payload is the complete current view of beacons.
        # devices_data[scanner_id]["beacons_observed"].clear() # Optional: if payload is always exhaustive

        for beacon_key, beacon_info in beacons_payload.items():
            rssi = beacon_info.get("RSSI")
            beacon_name = beacon_info.get("Beacon name")
            
            if rssi is None or beacon_name is None:
                print(f"Warning: Missing RSSI or Beacon name for beacon '{beacon_key}' from scanner '{scanner_id}'")
                continue
            
            devices_data[scanner_id]["beacons_observed"][beacon_key] = {
                "rssi": rssi,
                "beacon_name": beacon_name,
                "distance": RSSI_to_distance(rssi)
            }
            print(f"Scanner {scanner_id} updated beacon {beacon_key}: RSSI={rssi}, Name={beacon_name}, Movement: {devices_data[scanner_id].get('movement')}")
    elif beacons_payload is None:
        # It's valid for no beacons to be in range.
        # Log if movement data is present without beacon data.
        if movement_payload is not None:
             print(f"Scanner {scanner_id} reported movement: {devices_data[scanner_id].get('movement')} (No beacon data in this payload)")
    else:
        # If beacons_payload is not a dict (e.g. malformed)
        print(f"Scanner {scanner_id} sent invalid beacon data format. Payload: {data}")

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
    app.run(host='0.0.0.0', port=5000, debug=True)

    # Know issue here: This does not work for some reason use: 
    # flask --app esp32 run --host=0.0.0.0 --port=5000
    # To run the server instead

