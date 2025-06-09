#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <ArduinoJson.h>
#include <BLEAdvertisedDevice.h>
#include "Process.h"
#include "IMUManager.h"
#include "EventManager.h"
#include "config.h"
#include "SharedState.h"

class DataManager : public Process {
public:
    DataManager(SharedState& state) : sharedState(state) {}
    
    void setup(EventManager* em) override {
        Process::setup(em);
        eventManager->subscribe(EVT_SCAN_COMPLETE, this);
    }
    
    void onEvent(Event& event) override {
        if (event.type == EVT_SCAN_COMPLETE) {
            ScanCompleteEvent& e = static_cast<ScanCompleteEvent&>(event);
            processScanResults(e);
        }
    }
    
    void update() override {
        // Reactive
    }

private:
    void processScanResults(ScanCompleteEvent& scanEvent) {
        if (!sharedState.wifiConnected) {
            Serial.println("WiFi not connected, skipping data processing.");
            return;
        }

        JsonDocument doc;
        doc["scanner_id"] = sharedState.macAddress;

        JsonArray beacons = doc.createNestedArray("beacons");
        BLEUUID serviceUUID(BEACON_SERVICE_UUID);

        for (int i = 0; i < scanEvent.results.getCount(); i++) {
            BLEAdvertisedDevice device = scanEvent.results.getDevice(i);
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

        JsonObject movement = doc.createNestedObject("movement");
        movement["avgAngleXZ"] = scanEvent.avgAngleXZ;
        movement["avgAngleYZ"] = scanEvent.avgAngleYZ;
        movement["totalMovement"] = scanEvent.totalMovement;
        
        String jsonBuffer;
        serializeJson(doc, jsonBuffer);
        
        DataReadyForHttpEvent httpEvent(jsonBuffer);
        eventManager->publish(httpEvent);
    }

    SharedState& sharedState;
};

#endif // DATA_MANAGER_H 