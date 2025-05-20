// This is a beacon

#include <WiFi.h>
#include <HTTPClient.h>

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 10;  //In seconds
BLEScan *pBLEScan;

float BLEStrength = 0;
String BLEDeviceName = "";
String Data = "";
int devCounter = 0;

const char* ssid = "Rover";           // Replace with your Wi-Fi SSID
const char* password = "SMisgoed";   // Replace with your Wi-Fi password
const char* serverUrl = "http://192.168.1.165:5000/data";  // Flask server URL
const char*  name = "Beacon-1";

String jsonDataMaker(float BLEstrength, String BLEname, const char* BeaconName){
    devCounter = devCounter + 1;

    //String jsonData = "{";
    String jsonData = "\"Device_" + String(devCounter) + "\": {";
    jsonData += "\"RSSI\": " + String(BLEstrength, 1) + ",";
    jsonData += "\"BLE device name\": \"" + BLEname + "\"";
    jsonData += "}";
    jsonData += ",";
    return jsonData;
}

String jsonDataFixer(String json){
    String jsonData = "{";
    jsonData += json;
    jsonData += "\"Beacon name\": \"" + String(name) + "\"";
    jsonData += "}";

    return jsonData;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName().startsWith("ESP32")){
      Serial.print(advertisedDevice.getName());
      Serial.print(" strength: ");
      BLEStrength = advertisedDevice.getRSSI();
      BLEDeviceName = advertisedDevice.getName();
      Serial.println(advertisedDevice.getRSSI());
      Serial.print(F(""));
      Data = Data + jsonDataMaker(BLEStrength, BLEDeviceName, name);
    }
  }
};

// {"Device_16": {"RSSI": -75.0,"BLE device name": "ESP32-3"},"Device_17": {"RSSI": -90.0,"BLE device name": "ESP32-2"},"Beacon name": "Beacon-1"}


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

    Data += "\"Beacon name\": \"" + String(name) + "\"";
    Data += "}";
    Serial.print(Data);
    int httpResponseCode = http.POST(Data);
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
  delay(2000);
}

void BLEscan(){
  devCounter = 0;
  Data = "{";
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  //Data = jsonDataFixer(Data);
  // Serial.print("Devices found: ");
  // Serial.println(foundDevices->getCount());
  Serial.println("BLE scan done!");
  Serial.println(Data);
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
  delay(2000);
}

