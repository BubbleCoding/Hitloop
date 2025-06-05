#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Preferences.h>
#include "config.h" // For SCANNER_NAME

#define BOOT_BUTTON_PIN 9

class Config {
public:
  String ssid;
  String password;
  String serverUrl;
  String scannerName;

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

    WiFi.mode(WIFI_STA); // Ensure WiFi is on to get MAC address
    String mac = WiFi.macAddress();
    String macSuffix = mac.substring(12);
    macSuffix.replace(":", "");
    scannerName = String(SCANNER_NAME) + "-" + macSuffix;
    Serial.print("Scanner name set to: ");
    Serial.println(scannerName);
  }

  void clearConfig() {
    Serial.println("Clearing configuration...");
    preferences.begin("config", false); // Start in read-write mode
    preferences.clear();
    preferences.end();
    Serial.println("Configuration cleared. Restarting in 3 seconds...");
    delay(3000);
    ESP.restart();
  }

  void enterSerialConfig() {
    Serial.println("Entering configuration mode...");
    Serial.println("Send 'c' to configure, or 'e' to erase configuration.");

    while (!Serial.available()) { delay(100); }
    char cmd = Serial.read();
    while(Serial.available() && Serial.read() != '\n'); 

    if (cmd == 'e') {
        clearConfig();
        return;
    }

    if (cmd != 'c') {
        Serial.println("Invalid command. Aborting configuration.");
        return;
    }

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

private:
  Preferences preferences;
};

#endif // CONFIGURATION_H 