#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "Process.h"
#include "Timer.h"
#include "Configuration.h"
#include "SharedState.h"
#include <WiFi.h>

class WifiManager : public Process {
private:
    Timer wifiCheckTimer;
    Config& cfg;
    SharedState& sharedState;

public:
    WifiManager(Config& config, SharedState& state) 
        : wifiCheckTimer(500), cfg(config), sharedState(state) {}

    void setup(EventManager* em) override {
        Process::setup(em);
        WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());
        Serial.println("Connecting to WiFi...");
    }

    void update() override {
        bool isConnected = (WiFi.status() == WL_CONNECTED);

        if (isConnected != sharedState.wifiConnected) {
            sharedState.wifiConnected = isConnected;
            if (isConnected) {
                Serial.println("\nConnected to WiFi");
            }
        }

        if (!isConnected) {
            if (wifiCheckTimer.checkAndReset()) {
                Serial.print(".");
            }
        }
    }

    bool isConnected() const {
        return sharedState.wifiConnected;
    }

    String getMacAddress() const {
        return WiFi.macAddress();
    }
};

#endif // WIFI_MANAGER_H 