// This is a beacon

#include <WiFi.h>
#include <HTTPClient.h>

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5;  //In seconds
BLEScan *pBLEScan;

float BLEStrength = 0;

const char* ssid = "XXXX";           // Replace with your Wi-Fi SSID
const char* password = "XXXX";   // Replace with your Wi-Fi password
const char* serverUrl = "XXXX;  // Flask server URL
const char*  name = "Beacon-1";

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName().startsWith("ESP32")){
      Serial.print(advertisedDevice.getName());
      Serial.print(" strength: ");
      BLEStrength = advertisedDevice.getRSSI();
      Serial.println(advertisedDevice.getRSSI());
      Serial.print(F(""));
    }
  }
};


void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connecting to: ");
  Serial.println(serverUrl);

  Serial.println("\nConnected to WiFi");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEscan();
  wifiDataSender();
}

void wifiDataSender(){
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    // Generate fake data
    float RSSI = random(70, 90) / 10.0;  // e.g., 20.0 to 35.0°C
    float temp = random(70, 90) / 10.0;  // e.g., 20.0 to 35.0°C

    String jsonData = "{";
    jsonData += "\"RSSI\": " + String(RSSI, 1) + ",";
    jsonData += "\"RSSI_2\": " + String(RSSI, 1) + ",";
    jsonData += "\"Temp\": " + String(temp, 1);
    jsonData += "}";
    //String jsonData = jsonDataMaker(RSSI, String("ESP-32"), name);

    Serial.println("Sending data: " + jsonData);

    int httpResponseCode = http.POST(jsonData);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Requesting: ");
    Serial.println(serverUrl);
    Serial.print("Error: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());


    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
  delay(5000);
}

void BLEscan(){
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  // Serial.print("Devices found: ");
  // Serial.println(foundDevices->getCount());
  Serial.println("BLE scan done!");
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
  delay(5000);
}

String jsonDataMaker(float BLEstrength, String BLEname, const char* BeaconName){
    String jsonData = "{";
    jsonData += "\"RSSI\": " + String(BLEstrength, 1) + ",";
    jsonData += "\"Beacon name\": " + String(BeaconName, 1)+ "," ;
    jsonData += "\"BLE device name\": " + BLEname;
    jsonData += "}";

    return jsonData;
}
