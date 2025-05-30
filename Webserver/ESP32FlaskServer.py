from flask import Flask, request, jsonify, render_template_string
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
    html_template = '''
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="refresh" content="5">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>ESP32 RSSI Monitor</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                padding: 20px;
                background-color: #f0f2f5;
            }
            h1 {
                color: #333;
            }
            table {
                border-collapse: collapse;
                width: 100%;
                margin-top: 20px;
            }
            th, td {
                text-align: left;
                padding: 8px;
                border-bottom: 1px solid #ddd;
            }
            th {
                background-color: #f4f4f4;
            }
        </style>
    </head>
    <body>
        <h1>Last Known RSSI of each beacon in range </h1>
        {% if devices %}
            <table>
                <thead>
                    <tr>
                        <th>Device ID</th>
                        <th>Beacon name</th>
                        <th>RSSI</th>
                        <th>Distance</th>
                        <th>Last Updated</th>
                    </tr>
                </thead>
                <tbody>
                    {% for device_id, info in devices.items() %}
                        <tr>
                            <td>{{ device_id }}</td>
                            <td>{{ info.beacon_name }}</td>
                            <td>{{ info.rssi }}</td>
                            <td>{{ info.distance}} </td>
                            <td>{{ info.timestamp }}</td>
                        </tr>
                    {% endfor %}
                </tbody>
            </table>
        {% else %}
            <p>No device data received yet.</p>
        {% endif %}
    </body>
    </html>
    '''
    return render_template_string(html_template, devices=devices_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)

    # Know issue here: This does not work for some reason use: 
    # flask --app esp32 run --host=0.0.0.0 --port=5000
    # To run the server instead

