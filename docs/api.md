# API Reference

This document details the API endpoints for the HitLoop server.

---

### 1. `POST /data`

This is the primary endpoint used by scanners to report their findings to the server.

**Request Body:**

The request body must be a JSON object.

- `scanner_id` (string, required): The unique MAC address or identifier of the scanner.
- `beacons` (list, optional): A list of JSON objects, where each object represents a detected BLE beacon.
  - `name` (string, required): The name of the beacon.
  - `rssi` (integer, required): The Received Signal Strength Indicator.
- `movement` (object, optional): An object containing data from the scanner's IMU.
  - `avgAngleXZ` (float): Average angle on the XZ plane.
  - `avgAngleYZ` (float): Average angle on the YZ plane.
  - `totalMovement` (float): A metric representing total movement during the scan interval.
- `simulated` (boolean, optional): If `true`, the data is treated as coming from a simulation and is not persisted to the database.

**Example Request Body:**

```json
{
    "scanner_id": "64:E8:33:84:CB:84",
    "beacons": [
        { "name": "Beacon-A", "rssi": -55 },
        { "name": "Beacon-B", "rssi": -72 }
    ],
    "movement": {
        "avgAngleXZ": 12.3,
        "avgAngleYZ": -5.1,
        "totalMovement": 34.8
    }
}
```

**Responses:**

- **200 OK:** Indicates the data was successfully received. The response body contains commands for the scanner.
  - **Response Body:**
    - `wait_ms` (integer): The number of milliseconds the scanner should wait before starting its next scan to stay synchronized with the server's time slots.
    - `led_behavior` (object, optional): If a new LED behavior has been configured for this scanner, it will be included here.
    - `vibration_behavior` (object, optional): If a new vibration behavior has been configured, it will be included here.
  - **Example Response:**
    ```json
    {
        "wait_ms": 7854,
        "led_behavior": {
            "type": "HeartBeat",
            "params": { "color": "#FF0000", "pulse_interval": 2000 }
        }
    }
    ```
- **400 Bad Request:** Indicates a missing `scanner_id` in the payload.

---

### 2. `POST /configure/<scanner_id>`

Sets a pending LED or vibration behavior configuration for a specific scanner. The configuration is sent to the scanner the next time it sends data to the `/data` endpoint.

- `scanner_id` (string, URL parameter): The ID of the scanner to configure.

**Request Body:**

A JSON object containing the behavior to set.

- `led_behavior` (object, optional): The LED behavior configuration.
  - `type` (string): The name of the behavior (e.g., "Solid", "HeartBeat").
  - `params` (object): A key-value map of parameters for the behavior (e.g., `color`, `pulse_duration`).
- `vibration_behavior` (object, optional): The vibration behavior configuration.

**Example Request Body:**

```json
{
    "led_behavior": {
        "type": "Solid",
        "params": { "color": "#00FF00" }
    }
}
```

**Responses:**

- **200 OK:** The configuration has been successfully stored on the server and will be sent to the device on its next check-in.
- **400 Bad Request:** The request body did not contain any valid behavior configuration.

---

### 3. `GET /scanners`

Returns a JSON object containing the most recent data for all scanners that have been active within the last 5 minutes. This endpoint queries the database.

**Responses:**

- **200 OK:**
  - **Response Body:** A JSON object where each key is a `scanner_id`. The value contains the latest movement data, a list of observed beacons from the last scan, and the timestamp of the most recent activity.
  - **Example Response Body:**
    ```json
    {
        "64:E8:33:84:CB:84": {
            "timestamp": "2023-11-01T12:00:00Z",
            "movement": {
                "avgAngleXZ": 12.3,
                "avgAngleYZ": -5.1,
                "totalMovement": 34.8
            },
            "beacons_observed": {
                "Beacon-A": {
                    "rssi": -55,
                    "beacon_name": "Beacon-A",
                    "distance": "close"
                }
            }
        }
    }
    ```

---

### 4. `GET /reset_devices`

Clears all in-memory scanner data and pending device configurations on the server. Note: This does **not** clear the historical data from the database.

**Responses:**

- **200 OK:**
    ```json
    {
        "status": "success",
        "message": "All device data and configurations cleared."
    }
    ```

---

### 5. HTML Page Routes

These routes serve the user-facing web pages.

- **`GET /`**: Serves the main `index.html` dashboard, which is now deprecated in favor of the `/control` page.
- **`GET /control`**: Serves the main `control.html` page, which provides the real-time grid view of active scanners and the controls for configuring them.
- **`GET /simulation`**: Serves the `simulation.html` page for testing the system with simulated scanner data.
- **`GET /configure_device`**: Serves the `configure_device.html` page, which allows setting initial firmware credentials via WebSerial. 