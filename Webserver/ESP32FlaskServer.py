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
    
    # Check for Beacon name and remove from the device data
    beacon_name = data.get("Scanner name")
    if beacon_name:
        del data["Scanner name"]
    
    # Iterate over devices in the received data
    for device_id, device_info in data.items():
        rssi = device_info.get("RSSI")
        beacon_name = device_info.get("Beacon name")
        
        if rssi is None or beacon_name is None:
            return jsonify({"status": "error", "message": f"Missing data for {device_id}"}), 400
        
        # Store or update device data
        devices_data[device_id] = {
            "rssi": rssi,
            "beacon_name": beacon_name,
            "timestamp": datetime.utcnow().isoformat(),
            "distance" : RSSI_to_distance(rssi)
        }

        print(f"Updated data for {device_id}: RSSI={rssi}, BLE Name={beacon_name}")

    return jsonify({"status": "success"}), 200

@app.route('/')
def index():
    return render_template('index.html', devices=devices_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)

    # Know issue here: This does not work for some reason use: 
    # flask --app esp32 run --host=0.0.0.0 --port=5000
    # To run the server instead

