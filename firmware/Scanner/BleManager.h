#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEScan.h>
#include "Process.h"
#include "Timer.h"
#include "config.h"
#include "WifiManager.h"
#include "EventManager.h"

// Forward declaration for the global pointer
class BleManager;
extern BleManager* g_bleManager;

// The callback function that is executed when the scan is complete.
void scanCompleteCallback(BLEScanResults results);

class BleManager : public Process {
public:
    BleManager(WifiManager& wifi)
        : Process(),
          wifiManager(wifi),
          scanTimer(SCAN_INTERVAL_MS),
          pBLEScan(nullptr)
    {
        g_bleManager = this; // Assign this instance to the global pointer
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

    // This method is called by the global scanCompleteCallback
    void onScanComplete(BLEScanResults results) {
        Serial.printf("Scan complete! Found %d devices.\n", results.getCount());
        if (wifiManager.isConnected()) {
            ScanCompleteEvent event(results);
            eventManager->publish(event);
        } else {
            Serial.println("WiFi not connected, skipping event publish.");
        }
    }

private:
    void startScan() {
        // The isScanning() check is removed as it's not available.
        // The scanTimer logic prevents overlapping scans.
        Serial.println("Starting BLE scan...");
        
        // The DataManager will handle getting data from the IMU at the right time.
        pBLEScan->start(SCAN_DURATION, scanCompleteCallback);
    }

    // Member variables
    WifiManager& wifiManager;
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