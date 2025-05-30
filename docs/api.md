## API Endpoints

The server exposes the following endpoints:

### 1. `POST /data`

This endpoint is used by simulated or real beacon scanners to send their observed data to the server.

**Request Body:**

The request body must be a JSON object with the following structure:

- `Scanner name` (string, required): A unique identifier for the scanner sending the data.
- `movement` (float, optional): The total distance (e.g., in simulated pixels) the scanner has moved since its last report. Defaults to 0 if not provided.
- `beacons` (object, required): An object where each key is a unique beacon ID (e.g., `"beacon-NW"`) and the value is an object containing:
  - `RSSI` (integer, required): The Received Signal Strength Indicator for this scanner relative to this beacon.
  - `Beacon name` (string, required): A friendly name for the beacon (e.g., `"NW"`).

**Example Request Body:**

```json
{
    "Scanner name": "Scanner-A",
    "movement": 15.75,
    "beacons": {
        "beacon-NW": {
            "RSSI": -55,
            "Beacon name": "NW"
        },
        "beacon-SE": {
            "RSSI": -70,
            "Beacon name": "SE"
        }
    }
}
```

**Responses:**

- **200 OK:**
    Indicates that the data was successfully received and processed.
  - Body:
        ```json
        {
            "status": "success"
        }
        ```
- **400 Bad Request:**
    Indicates an error in the request data, such as missing `Scanner name`, or malformed `beacons` object, or missing `RSSI`/`Beacon name` for a beacon entry.
  - Body (example):
        ```json
        {
            "status": "error",
            "message": "Missing 'Scanner name' in payload"
        }
        ```

**Server-Side Logic:**

- The server uses the `Scanner name` as the primary key to store or update data in its internal `devices_data` dictionary.
- For each scanner, it stores:
  - `timestamp`: The UTC timestamp (ISO format) when the data was received.
  - `movement`: The reported movement value from the payload. If a scanner is new and no movement is reported, it defaults to 0.
  - `beacons_observed`: An object where each key is a `beacon_id` (e.g., `beacon-NW`). The value contains:
    - `rssi`: The received RSSI value.
    - `beacon_name`: The friendly name of the beacon.
    - `distance`: A string indicating the approximate distance ("close", "midrange", "far") calculated based on the RSSI.
      - "close": RSSI >= -65
      - "midrange": -80 < RSSI < -65
      - "far": RSSI <= -80
- If `RSSI` or `Beacon name` is missing for any beacon in the `beacons` object, a warning is logged on the server, and that specific beacon entry is skipped for that update.

### 2. `GET /`

This endpoint serves the main HTML dashboard page. This page displays a live table summarizing the status of all known scanners and the RSSI values to each beacon they have observed.

**Request Body:**

None.

**Responses:**

- **200 OK:**
  - Content-Type: `text/html`
  - Body: An HTML page. The page uses JavaScript to periodically fetch data from the `/devices` endpoint and dynamically update a table. The table typically shows:
    - Scanner Name
    - RSSI to Beacon 1 (e.g., NW)
    - RSSI to Beacon 2 (e.g., NE)
    - ... (for all detected beacons)
    - Movement (since last scanner report)
    - Last Update (timestamp for the scanner's last report)
  - If no scanner data has been received, it displays a message indicating so.

### 3. `GET /devices`

This endpoint returns a JSON object containing all current data for all known scanners.

**Request Body:**

None.

**Responses:**

- **200 OK:**
  - Content-Type: `application/json`
  - Body: A JSON object where each key is a `Scanner name`. The value for each scanner is an object containing:
    - `timestamp` (string): ISO format UTC timestamp of the last update from this scanner.
    - `movement` (float): The last reported movement value for this scanner.
    - `beacons_observed` (object): An object detailing each beacon observed by the scanner, with `rssi`, `beacon_name`, and calculated `distance`.
  **Example Response Body:**
    ```json
    {
        "Scanner-A": {
            "timestamp": "2023-10-27T10:30:00.123Z",
            "movement": 25.5,
            "beacons_observed": {
                "beacon-NW": {
                    "rssi": -60,
                    "beacon_name": "NW",
                    "distance": "close"
                }
            }
        },
        "Scanner-B": {
            "timestamp": "2023-10-27T10:30:05.456Z",
            "movement": 10.2,
            "beacons_observed": {
                "beacon-NW": {
                    "rssi": -70,
                    "beacon_name": "NW",
                    "distance": "midrange"
                },
                "beacon-SE": {
                    "rssi": -85,
                    "beacon_name": "SE",
                    "distance": "far"
                }
            }
        }
    }
    ```

### 4. `GET /reset_devices`

This endpoint clears all stored scanner data on the server. This is useful for resetting the simulation or data collection.

**Request Body:**

None.

**Responses:**

- **200 OK:**
  - Content-Type: `application/json`
  - Body:
    ```json
    {
        "status": "success",
        "message": "All device data cleared."
    }
    ``` 