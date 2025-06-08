#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "Process.h"
#include "Timer.h"
#include "config.h"
#include "WifiManager.h"
#include "Configuration.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <HTTPClient.h>

class BleManager;

extern BleManager* g_bleManager;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice);
};

void scanCompleteCallback(BLEScanResults results);

class BleManager : public Process {
private:
    Timer scanTimer;
    bool isScanning;
    BLEScan* pBLEScan;
    String jsonString;
    int devCounter;
    const WifiManager& wifiManager;
    Config& cfg;

    bool startScan() {
        if (isScanning) return false;
        
        devCounter = 0;
        jsonString = "";
        Serial.println("Starting BLE scan...");
        isScanning = true;
      
        if(pBLEScan->start(SCAN_TIME, scanCompleteCallback, false)) { // Explicitly non-continuous
            return true;
        } else {
            Serial.println("Error starting scan");
            isScanning = false;
            return false;
        }
    }
    
    String jsonDataMaker(const String& UUID, float BLEstrength, const String& BeaconName) {
        devCounter++;
        String jsonData = "\"" + UUID + "\": {";
        jsonData += "\"RSSI\": " + String(BLEstrength, 1) + ",";
        jsonData += "\"Beacon name\": \"" + BeaconName + "\"";
        jsonData += "},";
        return jsonData;
    }

    String jsonDataFixer() {
        String rawData = jsonString;
        if (rawData.endsWith(",")) {
            rawData.remove(rawData.length() - 1);
        }
        String jsonData = "{";
        jsonData += rawData;
        jsonData += ",\"Scanner name\": \"" + cfg.scannerName + "\"";
        jsonData += "}";
        return jsonData;
    }

    void wifiDataSender() {
        if (wifiManager.isConnected()) {
            HTTPClient http;
            http.begin(cfg.serverUrl);
            http.addHeader("Content-Type", "application/json");

            String finalJson = jsonDataFixer();
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
    }

public:
    BleManager(const WifiManager& wifi, Config& config) : 
        scanTimer(SCAN_TIME * 1000 + 1000), 
        isScanning(false), 
        pBLEScan(nullptr), 
        wifiManager(wifi),
        cfg(config) 
    {
        g_bleManager = this;
    }

    void setup() override {
        BLEDevice::init("");
        pBLEScan = BLEDevice::getScan();
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        pBLEScan->setActiveScan(true);
        pBLEScan->setInterval(BLE_SCAN_INTERVAL);
        pBLEScan->setWindow(BLE_SCAN_WINDOW);
        Serial.println("BLE Initialized");
    }

    void update() override {
        if (wifiManager.isConnected() && scanTimer.hasElapsed() && !isScanning) {
            if (startScan()) {
                scanTimer.reset();
            }
        }
    }

    void onScanComplete(BLEScanResults& results) {
        Serial.print("Scan complete. Devices found: ");
        Serial.println(results.getCount());
        isScanning = false;
        wifiDataSender();
    }

    void onDeviceFound(BLEAdvertisedDevice& advertisedDevice) {
        if (advertisedDevice.haveName() && advertisedDevice.getName().startsWith(BEACON_NAME_PREFIX)) {
            float rssi = advertisedDevice.getRSSI();
            String BeaconName = advertisedDevice.getName();
            String uuid = advertisedDevice.getAddress().toString();
            
            jsonString += jsonDataMaker(uuid, rssi, BeaconName);
        }
    }
};

inline void MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    if (g_bleManager) {
        g_bleManager->onDeviceFound(advertisedDevice);
    }
}

inline void scanCompleteCallback(BLEScanResults results) {
    if (g_bleManager) {
        g_bleManager->onScanComplete(results);
    }
}

#endif // BLE_MANAGER_H 