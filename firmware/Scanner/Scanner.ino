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
#include "IMUManager.h"
#include "LedManager.h"
#include "VibrationManager.h"
#include "BehaviorManager.h"
#include "HTTPManager.h"
#include "DataManager.h"
#include "BleManager.h"

Config cfg;

// Global pointer for BLE callback
BleManager* g_bleManager = nullptr;

// Process managers
std::vector<Process*> processes;
WifiManager* wifiManager;
IMUManager* imuManager;
LedManager* ledManager;
VibrationManager* vibrationManager;
BehaviorManager* behaviorManager;
HTTPManager* httpManager;
DataManager* dataManager;
BleManager* bleManager;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  cfg.loadConfig();

  // --- Instantiate all process managers ---
  // Core hardware and system
  processes.push_back(new SystemManager(cfg));
  wifiManager = new WifiManager(cfg);
  processes.push_back(wifiManager);
  imuManager = new IMUManager();
  processes.push_back(imuManager);

  // Peripheral managers
  ledManager = new LedManager();
  processes.push_back(ledManager);
  vibrationManager = new VibrationManager();
  processes.push_back(vibrationManager);

  // Logic and communication managers
  behaviorManager = new BehaviorManager(ledManager, vibrationManager);
  processes.push_back(behaviorManager);
  httpManager = new HTTPManager(cfg, behaviorManager);
  processes.push_back(httpManager);
  dataManager = new DataManager(imuManager, wifiManager, httpManager);
  processes.push_back(dataManager);
  
  // The BleManager now only depends on Wifi and Data managers
  bleManager = new BleManager(*wifiManager, dataManager);
  processes.push_back(bleManager);
  
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
