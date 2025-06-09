#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEScan.h>
#include "Process.h"
#include "Timer.h"
#include "config.h"
#include "EventManager.h"
#include "IMUManager.h"

// Forward declaration for the global pointer
class BleManager;
extern BleManager* g_bleManager;

// The callback function that is executed when the scan is complete.
void scanCompleteCallback(BLEScanResults results);

class BleManager : public Process {
public:
    BleManager(IMUManager* imu)
        : Process(),
          imuManager(imu),
          scanTimer(SCAN_INTERVAL_MS),
          pBLEScan(nullptr)
    {
        g_bleManager = this;
    }

    void setup(EventManager* em) override {
        Process::setup(em);
        BLEDevice::init("");
        pBLEScan = BLEDevice::getScan();
        pBLEScan->setActiveScan(false);
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

    void onScanComplete(BLEScanResults results) {
        Serial.printf("Scan complete! Found %d devices.\n", results.getCount());
        
        float avgAngleXZ = 0.0, avgAngleYZ = 0.0, totalMovement = 0.0;
        if (imuManager) {
            avgAngleXZ = imuManager->getAverageAngleXZ();
            avgAngleYZ = imuManager->getAverageAngleYZ();
            totalMovement = imuManager->getTotalMovement();
            imuManager->prepareForNextInterval();
        }
        
        ScanCompleteEvent event(results, avgAngleXZ, avgAngleYZ, totalMovement);
        eventManager->publish(event);
    }

private:
    void startScan() {
        Serial.println("Starting BLE scan...");
        pBLEScan->start(SCAN_DURATION, scanCompleteCallback);
    }

    IMUManager* imuManager;
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