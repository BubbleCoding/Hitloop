# API Reference

This document details the API endpoints for the HitLoop server.

---

### 1. `POST /data`

This is the primary endpoint used by scanners to report their findings to the server. It's designed to be flexible and can accept data from both real hardware and the simulation.

**Request Body:**

The request body must be a JSON object.

- `scanner_id` or `Scanner name` (string, required): The unique identifier of the scanner. The server accepts both keys.
- `beacons` (list or object, optional):
  - For **real devices**, this should be a `list` of beacon objects: `[{ "name": "Beacon-A", "rssi": -55 }]`
  - For the **simulation**, this can be an `object` where keys are beacon names: `{ "beacon-NW": { "RSSI": -53, "Beacon name": "NW" } }`
- `movement` (object or number, optional):
  - For **real devices**, this should be an `object`: `{ "avgAngleXZ": 12.3, "avgAngleYZ": -5.1, "totalMovement": 34.8 }`
  - For the **simulation**, this can be a single `number` representing total movement.
- `simulated` (boolean, optional): If `true`, the data is not persisted to the database.

**Responses:**

- **200 OK:** Indicates the data was successfully received. The response body contains commands for the scanner.
  - **Response Body:**
    - `wait_ms` (integer): The number of milliseconds the scanner should wait before its next scan.
    - `led_behavior` (object, optional): A new LED behavior configuration, if one is pending for this scanner.
    - `vibration_behavior` (object, optional): A new vibration behavior configuration, if one is pending.
- **400 Bad Request:** Indicates a missing scanner identifier in the payload.

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

**Responses:**

- **200 OK:** The configuration has been successfully stored on the server.
- **400 Bad Request:** The request body did not contain any valid behavior configuration.

---

### 3. `GET /devices`

Returns a unified JSON object of all active devices. This endpoint is designed for live-view pages like the index.

It works by:
1. Fetching all recently active **real** devices from the database.
2. Merging this list with the **in-memory** data, which includes all **simulated** devices.
3. If a device exists in both, the in-memory data takes precedence, ensuring the view is always current.

**Responses:**

- **200 OK:** A JSON object where each key is a `scanner_id`. The value contains the latest known data for that scanner.

---

### 4. `GET /scanners`

Returns a JSON object containing the most recent data for all **real** scanners that have been active within the last 5 minutes. This endpoint **only** queries the database and will not include simulated devices. It is used by the Control page.

**Responses:**

- **200 OK:**
  - **Response Body:** A JSON object where each key is a `scanner_id`. The value contains the latest movement data, a list of observed beacons, and the timestamp from the database.

---

### 5. `GET /reset_devices`

Clears all **in-memory** scanner data and pending device configurations on the server. Note: This does **not** clear the historical data from the database.

**Responses:**

- **200 OK:** A success message.

---

### 6. HTML Page Routes

These routes serve the user-facing web pages.

- **`GET /`**: Serves the main `index.html` dashboard, which provides a simple table view of live device data.
- **`GET /control`**: Serves the main `control.html` page, which provides the real-time grid view of active scanners and the controls for configuring them.
- **`GET /simulation`**: Serves the `simulation.html` page for testing the system with simulated scanner data.
- **`GET /configure_device`**: Serves the `configure_device.html` page, which allows setting initial firmware credentials via WebSerial. 