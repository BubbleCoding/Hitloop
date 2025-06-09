#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEScan.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Process.h"
#include "Timer.h"
#include "config.h"
#include "WifiManager.h"
#include "IMUManager.h"
#include "LedManager.h"
#include "VibrationManager.h"

// Forward declaration for the global pointer
class BleManager;
extern BleManager* g_bleManager;

// The callback function that is executed when the scan is complete.
// It must be a global or static function.
void scanCompleteCallback(BLEScanResults results);

class BleManager : public Process {
public:
    BleManager(WifiManager& wifi, Config& config, IMUManager* imu, LedManager* led, VibrationManager* vib)
        : Process(),
          wifiManager(wifi),
          cfg(config),
          imuManager(imu),
          ledManager(led),
          vibrationManager(vib),
          scanTimer(SCAN_INTERVAL_MS),
          pBLEScan(nullptr)
    {
        g_bleManager = this; // Assign this instance to the global pointer
    }

    void setup() override {
        BLEDevice::init("");
        pBLEScan = BLEDevice::getScan();
        pBLEScan->setActiveScan(true);
        pBLEScan->setInterval(BLE_SCAN_INTERVAL);
        pBLEScan->setWindow(BLE_SCAN_WINDOW);
        scanTimer.reset();
        Serial.println("BLE Initialized");
    }

    void update() override {
        if (scanTimer.checkAndReset()) {
            startScan();
        }
    }

    // This method is called by the global scanCompleteCallback
    void onScanComplete(BLEScanResults results) {
        Serial.printf("Scan complete! Found %d devices.\n", results.getCount());
        if (wifiManager.isConnected()) {
            sendData(results);
        } else {
            Serial.println("WiFi not connected, skipping data send.");
        }
    }

private:
    void startScan() {
        Serial.println("Starting BLE scan...");
        
        // Prepare the IMU for a new accumulation interval
        if (imuManager) {
            imuManager->prepareForNextInterval();
        }

        // Start the scan. The scanCompleteCallback will be executed when it's done.
        pBLEScan->start(SCAN_DURATION, scanCompleteCallback);
    }

    void sendData(BLEScanResults& results) {
        HTTPClient http;
        http.begin(cfg.serverUrl.c_str());
        http.addHeader("Content-Type", "application/json");

        JsonDocument doc;
        doc["scanner_id"] = wifiManager.getMacAddress();

        JsonArray beacons = doc.createNestedArray("beacons");
        for (int i = 0; i < results.getCount(); i++) {
            BLEAdvertisedDevice device = results.getDevice(i);
            if(device.haveName() && String(device.getName().c_str()).startsWith(BEACON_NAME_PREFIX)) {
                JsonObject beacon = beacons.add<JsonObject>();
                beacon["name"] = device.getName().c_str();
                beacon["rssi"] = device.getRSSI();
            }
        }

        if (imuManager) {
            JsonObject movement = doc.createNestedObject("movement");
            movement["avgAngleXZ"] = imuManager->getAverageAngleXZ();
            movement["avgAngleYZ"] = imuManager->getAverageAngleYZ();
            movement["totalMovement"] = imuManager->getTotalMovement();
        }

        String jsonBuffer;
        serializeJson(doc, jsonBuffer);
        Serial.println("Sending JSON: " + jsonBuffer);

        int httpResponseCode = http.POST(jsonBuffer);

        if (httpResponseCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Received response: " + payload);
            parseBehaviorResponse(payload);
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    }
    
    uint32_t hexToColor(String hex) {
        if (hex.startsWith("#")) {
            hex = hex.substring(1);
        }
        // Add "0x" prefix for strtol to correctly parse it as a hex number
        return (uint32_t)strtol(("0x" + hex).c_str(), NULL, 16);
    }

    void parseBehaviorResponse(String& payload) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.printf("deserializeJson() failed: %s\n", error.c_str());
            return;
        }

        // --- Parse and Set LED Behavior ---
        if (doc.containsKey("led_behavior")) {
            JsonObject led_config = doc["led_behavior"];
            const char* type = led_config["type"];
            bool createNew = true; // Assume we need to create a new behavior

            if (ledManager->currentBehavior && strcmp(ledManager->currentBehavior->type, type) == 0) {
                // Types match, now check parameters to see if an update is needed
                createNew = false; // Presume it's the same unless a parameter differs
                if (strcmp(type, "Breathing") == 0) {
                    uint32_t newColor = hexToColor(led_config["params"]["color"].as<String>());
                    if (static_cast<BreathingBehavior*>(ledManager->currentBehavior)->color != newColor) createNew = true;
                } else if (strcmp(type, "HeartBeat") == 0) {
                    uint32_t newColor = hexToColor(led_config["params"]["color"].as<String>());
                    unsigned long newPeriod = led_config["params"]["period"].as<unsigned long>();
                    auto* current = static_cast<HeartBeatBehavior*>(ledManager->currentBehavior);
                    if (current->color != newColor || current->period != newPeriod) createNew = true;
                } else if (strcmp(type, "Cycle") == 0) {
                    uint32_t newColor = hexToColor(led_config["params"]["color"].as<String>());
                    int newDelay = led_config["params"]["delay"].as<int>();
                    auto* current = static_cast<CycleBehavior*>(ledManager->currentBehavior);
                    if (current->color != newColor || current->delay != newDelay) createNew = true;
                }
            }

            if (createNew) {
                Serial.printf("Setting new LED behavior: %s\n", type);
                if (strcmp(type, "Off") == 0) ledManager->setBehavior(new LedsOffBehavior());
                else if (strcmp(type, "Breathing") == 0) ledManager->setBehavior(new BreathingBehavior(hexToColor(led_config["params"]["color"].as<String>())));
                else if (strcmp(type, "HeartBeat") == 0) ledManager->setBehavior(new HeartBeatBehavior(hexToColor(led_config["params"]["color"].as<String>()), led_config["params"]["period"].as<unsigned long>()));
                else if (strcmp(type, "Cycle") == 0) ledManager->setBehavior(new CycleBehavior(hexToColor(led_config["params"]["color"].as<String>()), led_config["params"]["delay"].as<int>()));
            }
        }

        // --- Parse and Set Vibration Behavior ---
        if (doc.containsKey("vibration_behavior")) {
            JsonObject vib_config = doc["vibration_behavior"];
            const char* type = vib_config["type"];
            bool createNew = true;

            if (vibrationManager->currentBehavior && strcmp(vibrationManager->currentBehavior->type, type) == 0) {
                createNew = false;
                if (strcmp(type, "Constant") == 0) {
                    uint8_t newIntensity = vib_config["params"]["intensity"].as<uint8_t>();
                    if (static_cast<ConstantVibrationBehavior*>(vibrationManager->currentBehavior)->intensity != newIntensity) createNew = true;
                } else if (strcmp(type, "Burst") == 0) {
                    uint8_t newIntensity = vib_config["params"]["intensity"].as<uint8_t>();
                    unsigned long newFreq = vib_config["params"]["frequency"].as<unsigned long>();
                    auto* current = static_cast<BurstVibrationBehavior*>(vibrationManager->currentBehavior);
                    if (current->intensity != newIntensity || current->frequency != newFreq) createNew = true;
                }
            }

            if (createNew) {
                Serial.printf("Setting new vibration behavior: %s\n", type);
                if (strcmp(type, "Off") == 0) vibrationManager->setBehavior(new MotorOffBehavior());
                else if (strcmp(type, "Constant") == 0) vibrationManager->setBehavior(new ConstantVibrationBehavior(vib_config["params"]["intensity"].as<uint8_t>()));
                else if (strcmp(type, "Burst") == 0) vibrationManager->setBehavior(new BurstVibrationBehavior(vib_config["params"]["intensity"].as<uint8_t>(), vib_config["params"]["frequency"].as<unsigned long>()));
            }
        }
    }

    // Member variables
    WifiManager& wifiManager;
    Config& cfg;
    IMUManager* imuManager;
    LedManager* ledManager;
    VibrationManager* vibrationManager;
    Timer scanTimer;
    BLEScan* pBLEScan;
};

// Define the callback function to pass to the BLE scanner
inline void scanCompleteCallback(BLEScanResults results) {
    if (g_bleManager) {
        g_bleManager->onScanComplete(results);
    }
}

#endif // BLE_MANAGER_H 