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

int scanTime = 10;  // Scan duration in seconds
BLEScan* pBLEScan;

String Data = "";
int devCounter = 0;

const char* ssid = "XXXXXX";
const char* password = "XXXXXX";
const char* serverUrl = "http://192.168.1.165:5000/data";
const char* name = "Scanner-1";

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

// BLE device callback
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveName() && advertisedDevice.getName().startsWith("ESP32")) {
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

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Init BLE scan
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
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
  
  BLEScanResults* results = pBLEScan->start(scanTime, false);
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
