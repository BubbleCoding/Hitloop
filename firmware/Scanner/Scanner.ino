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

BLEScan* pBLEScan;
Config cfg;

String Data = "";
int devCounter = 0;

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
      
      Data += jsonDataMaker(uuid, rssi, BeaconName);  // Use UUID as device key
    }
  }
};

void initializeWireless(){
// Connect to Wi-Fi
  WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_CONNECT_DELAY);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

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
  delay(SETUP_DELAY);

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  cfg.loadConfig();

  Serial.println("Hold BOOT button (PIN 9) now to enter config mode.");
  delay(2000); // Give user time to press button

  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    cfg.enterSerialConfig();
  }

  initializeWireless();
}

void loop() {
  BLEscan();
  wifiDataSender();
  wifiReceiveData();
}

void BLEscan() {
  devCounter = 0;
  Data = "";
  Serial.println("Starting BLE scan...");
  
  BLEScanResults* results = pBLEScan->start(SCAN_TIME, false);
  Serial.print("Scan complete. Devices found: ");
  Serial.println(results->getCount());
  
  pBLEScan->clearResults();  // Always free up memory!
  delay(1000);
}

void wifiDataSender() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(cfg.serverUrl);
    http.addHeader("Content-Type", "application/json");

    String finalJson = jsonDataFixer(Data);
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
  delay(3000);
}

void wifiReceiveData(){
  return;
}
