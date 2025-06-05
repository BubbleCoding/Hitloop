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
#include <Preferences.h>


BLEScan* pBLEScan;
Preferences preferences;

String Data = "";
int devCounter = 0;

String ssid = "";
String password = "";
String serverUrl = "";
String name = "";

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
  jsonData += ",\"Scanner name\": \"" + String(name) + "\"";
  jsonData += "}";
  return jsonData;
}

void enterSerialConfig() {
  Serial.println("Entering configuration mode...");
  preferences.begin("config", false); // Start preferences in read-write mode

  Serial.println("Enter SSID:");
  while (!Serial.available()) { delay(100); }
  ssid = Serial.readStringUntil('\n');
  ssid.trim();
  preferences.putString("ssid", ssid);
  Serial.print("SSID set to: ");
  Serial.println(ssid);

  Serial.println("Enter Password:");
  while (!Serial.available()) { delay(100); }
  password = Serial.readStringUntil('\n');
  password.trim();
  preferences.putString("password", password);
  Serial.println("Password set.");

  Serial.println("Enter Server URL:");
  while (!Serial.available()) { delay(100); }
  serverUrl = Serial.readStringUntil('\n');
  serverUrl.trim();
  preferences.putString("serverUrl", serverUrl);
  Serial.print("Server URL set to: ");
  Serial.println(serverUrl);

  preferences.end();
  Serial.println("Configuration saved. Restarting in 3 seconds...");
  delay(3000);
  ESP.restart();
}

void loadConfig() {
  preferences.begin("config", true); // Start preferences in read-only mode
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  serverUrl = preferences.getString("serverUrl", "http://192.168.1.165:5000/data"); // Default value
  preferences.end();

  Serial.println("Loaded configuration:");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Server URL: ");
  Serial.println(serverUrl);
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
  WiFi.begin(ssid.c_str(), password.c_str());
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

void setScannerName() {
  String mac = WiFi.macAddress();
  String macSuffix = mac.substring(12);
  macSuffix.replace(":", "");
  name = String(SCANNER_NAME) + "-" + macSuffix;
  Serial.print("Scanner name set to: ");
  Serial.println(name);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(SETUP_DELAY);

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  loadConfig();

  Serial.println("Hold BOOT button (PIN 9) now to enter config mode.");
  delay(2000); // Give user time to press button

  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    enterSerialConfig();
  }

  initializeWireless();
  setScannerName();
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
    http.begin(serverUrl);
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
