/* 
Deze code moet:
BLE ontvangen van de beacons.                             |Check
De RSSI en de naam van de beacon naar de server sturen.   |Check
Data ontvangen van de server.                             |
Commands van de server uitvoeren.                         |
*/
#include "Arduino.h"
#include <vector>
#include "config.h"
#include "Configuration.h"
#include "Timer.h"

// Process classes
#include "SystemManager.h"
#include "WifiManager.h"
#include "BleManager.h"
#include "IMUManager.h"
#include "LedManager.h"

Config cfg;

// Define the global pointer required for callbacks
BleManager* g_bleManager = nullptr;

// Process managers
std::vector<Process*> processes;
WifiManager* wifiManager;
BleManager* bleManager;
IMUManager* imuManager;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  cfg.loadConfig();

  // Create and add all processes to the manager
  processes.push_back(new SystemManager(cfg));

  wifiManager = new WifiManager(cfg);
  processes.push_back(wifiManager);

  imuManager = new IMUManager();
  processes.push_back(imuManager);

  // The BleManager depends on the WifiManager, Config, and IMUManager
  bleManager = new BleManager(*wifiManager, cfg, imuManager);
  processes.push_back(bleManager);

  processes.push_back(new LedManager());
  
  // Initialize all processes
  for (auto process : processes) {
    process->setup();
  }
}

void loop() {
  // Update all processes
  for (auto process : processes) {
    process->update();
  }
}
