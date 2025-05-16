#include <WiFi.h>
WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin("XXXX", "XXXX");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");

  if (client.connect("XXXX", 5000)) {
    Serial.println("Connection to Flask server succeeded!");
    client.stop();
  } else {
    Serial.println("Connection to Flask server FAILED!");
  }
}

void loop() {}
