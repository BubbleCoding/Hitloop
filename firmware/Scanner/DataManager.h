#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <ArduinoJson.h>
#include <BLEAdvertisedDevice.h>
#include "Process.h"
#include "IMUManager.h"
#include "WifiManager.h"
#include "HTTPManager.h"
#include "config.h"

class DataManager : public Process {
public:
    DataManager(IMUManager* imu, WifiManager* wifi, HTTPManager* http) 
        : imuManager(imu), wifiManager(wifi), httpManager(http) {}
    
    // Called by BleManager when a scan is complete
    void processScanResults(BLEScanResults& results) {
        JsonDocument doc;
        doc["scanner_id"] = wifiManager->getMacAddress();

        JsonArray beacons = doc.createNestedArray("beacons");
        BLEUUID serviceUUID(BEACON_SERVICE_UUID);

        for (int i = 0; i < results.getCount(); i++) {
            BLEAdvertisedDevice device = results.getDevice(i);
            if (device.isAdvertisingService(serviceUUID)) {
                JsonObject beacon = beacons.add<JsonObject>();
                if (device.haveName()) {
                    beacon["name"] = device.getName().c_str();
                } else {
                    beacon["name"] = device.getAddress().toString().c_str();
                }
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
        
        if (httpManager) {
            httpManager->sendData(jsonBuffer);
        }
    }
    
    void update() override {
        // This manager is reactive, does nothing in update
    }

private:
    IMUManager* imuManager;
    WifiManager* wifiManager;
    HTTPManager* httpManager;
};

#endif // DATA_MANAGER_H 