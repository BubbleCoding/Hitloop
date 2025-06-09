#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "Process.h"
#include "Timer.h"
#include "Configuration.h"
#include <WiFi.h>

class WifiManager : public Process {
private:
    Timer wifiCheckTimer;
    bool connected = false;
    Config& cfg;

public:
    WifiManager(Config& config) : wifiCheckTimer(500), cfg(config) {}

    void setup() override {
        WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());
        Serial.println("Connecting to WiFi...");
    }

    void update() override {
        if (WiFi.status() != WL_CONNECTED) {
            if (wifiCheckTimer.checkAndReset()) {
                Serial.print(".");
            }
            return;
        }

        if (!connected) {
            connected = true;
            Serial.println("\nConnected to WiFi");
        }
    }

    bool isConnected() const {
        return connected;
    }

    String getMacAddress() const {
        return WiFi.macAddress();
    }
};

#endif // WIFI_MANAGER_H 