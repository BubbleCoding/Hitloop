// This is a beacon

#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Rover";           // Replace with your Wi-Fi SSID
const char* password = "SMisgoed";   // Replace with your Wi-Fi password
const char* serverUrl = "http://192.168.1.165:5000/data";  // Flask server URL
const char* name = "Beacon-1";

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
}

void loop() {
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

  delay(10000); // Wait 10 seconds before sending again
}

void jsonDataMaker(int BLEstrength, char BLEname, char BeaconName){
    String jsonData = "{";
    jsonData += "\"RSSI\": " + String(BLEstrength, 1) + ",";
    jsonData += "\"Beacon name\": " + String(BeaconName, 1);
    jsonData += "\"BLE device name\": " + String(BLEname, 1);
    jsonData += "}";
}
