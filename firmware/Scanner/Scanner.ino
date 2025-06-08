/* 
Deze code moet:
BLE ontvangen van de beacons.                             |Check
De RSSI en de naam van de beacon naar de server sturen.   |Check
Data ontvangen van de server.                             |
Commands van de server uitvoeren.                         |
*/
#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "config.h"
#include "Configuration.h"

struct Timer {
    unsigned long last_update;
    unsigned long interval;

    Timer(unsigned long an_interval) : interval(an_interval), last_update(0) {}

    bool hasElapsed() {
        return millis() - last_update > interval;
    }

    bool checkAndReset() {
        if (hasElapsed()) {
            reset();
            return true;
        }
        return false;
    }

    void reset() {
        last_update = millis();
    }
};

BLEScan* pBLEScan;
Config cfg;

String jsonString = "";
int devCounter = 0;

Timer scanTimer(SCAN_TIME * 1000 + 1000); // Scan duration + delay
Timer wifiCheckTimer(500);
Timer configModeTimer(2000);

bool configMode = false;
bool wifiInitialized = false;
bool isScanning = false;

// Format a single device's data into JSON using the UUID
String jsonDataMaker(String UUID, float BLEstrength, String BeaconName) {
  devCounter++;
  String jsonData = "\"" + UUID + "\": {";  // Use UUID as device ID
  jsonData += "\"RSSI\": " + String(BLEstrength, 1) + ",";
  jsonData += "\"Beacon name\": \"" + BeaconName + "\"";
  jsonData += "},";
  return jsonData;
}

// Wrap all the data in a valid JSON object
String jsonDataFixer(String rawData) {
  if (rawData.endsWith(",")) {
    rawData.remove(rawData.length() - 1); // Remove trailing comma
  }
  String jsonData = "{";
  jsonData += rawData;
  jsonData += ",\"Scanner name\": \"" + cfg.scannerName + "\"";
  jsonData += "}";
  return jsonData;
}


// BLE device callback
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveName() && advertisedDevice.getName().startsWith(BEACON_NAME_PREFIX)) {
      float rssi = advertisedDevice.getRSSI();
      String BeaconName = advertisedDevice.getName();
      String uuid = advertisedDevice.getAddress().toString();  // Get the device's MAC address as UUID
      
      Serial.print(BeaconName);
      Serial.print(" RSSI: ");
      Serial.println(rssi);
      Serial.print(" UUID (MAC Address): ");
      Serial.println(uuid);
      
      jsonString += jsonDataMaker(uuid, rssi, BeaconName);  // Use UUID as device key
    }
  }
};

void scanCompleteCallback(BLEScanResults results) {
  Serial.print("Scan complete. Devices found: ");
  Serial.println(results.getCount());
  // pBLEScan->clearResults(); // The results are automatically cleared after the callback
  
  // After scan is complete, we can send the data
  wifiDataSender();
  isScanning = false;
}

void initializeBLE(){
  // Init BLE scan
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(BLE_SCAN_INTERVAL);
  pBLEScan->setWindow(BLE_SCAN_WINDOW);
}


void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  configModeTimer.reset();

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  cfg.loadConfig();

  Serial.println("Hold BOOT button (PIN 9) now to enter config mode.");
  
  WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());
  Serial.println("Connecting to WiFi...");
}

void loop() {
  // Handle config mode entry within the first 2 seconds
  if (!configModeTimer.hasElapsed()) {
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
      if (!configMode) {
        configMode = true;
        cfg.enterSerialConfig(); // This function blocks and then restarts, so no need to handle state after it.
      }
    }
  }

  // Handle WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    if(wifiCheckTimer.checkAndReset()){
      Serial.print(".");
    }
    return; // Wait for wifi to connect
  } 
  
  if (!wifiInitialized) {
      Serial.println("\nConnected to WiFi");
      initializeBLE();
      wifiInitialized = true;
  }

  // Non-blocking BLE scan
  if (scanTimer.hasElapsed() && !isScanning) {
    if (BLEscan()) {
        scanTimer.reset();
    }
  }

  // The data sending is now triggered by the BLE scan completion callback.
  wifiReceiveData();
}

bool BLEscan() {
  devCounter = 0;
  jsonString = "";
  Serial.println("Starting BLE scan...");
  isScanning = true;
  
  // Start the scan asynchronously. The scanCompleteCallback function will be called when it's done.
  if(pBLEScan->start(SCAN_TIME, scanCompleteCallback)) {
    return true;
  } else {
    Serial.println("Error starting scan");
    return false;
  }
}

void wifiDataSender() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(cfg.serverUrl);
    http.addHeader("Content-Type", "application/json");

    String finalJson = jsonDataFixer(jsonString);
    Serial.println("Sending JSON: ");
    Serial.println(finalJson);

    int httpResponseCode = http.POST(finalJson);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    if (httpResponseCode != 200) {
      Serial.print("Error: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void wifiReceiveData(){
  return;
}
