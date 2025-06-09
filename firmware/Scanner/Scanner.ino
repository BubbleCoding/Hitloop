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
#include "EventManager.h"

Config cfg;

// Global pointer for BLE callback
BleManager* g_bleManager = nullptr;
// Central Event Manager
EventManager eventManager;

// Process managers
std::vector<Process*> processes;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  cfg.loadConfig();

  // --- Instantiate all process managers ---
  SystemManager* systemManager = new SystemManager(cfg);
  processes.push_back(systemManager);
  
  WifiManager* wifiManager = new WifiManager(cfg);
  processes.push_back(wifiManager);
  
  IMUManager* imuManager = new IMUManager();
  processes.push_back(imuManager);

  LedManager* ledManager = new LedManager();
  processes.push_back(ledManager);
  
  VibrationManager* vibrationManager = new VibrationManager();
  processes.push_back(vibrationManager);
  
  BehaviorManager* behaviorManager = new BehaviorManager(ledManager, vibrationManager);
  processes.push_back(behaviorManager);
  
  HTTPManager* httpManager = new HTTPManager(cfg);
  processes.push_back(httpManager);
  
  DataManager* dataManager = new DataManager(imuManager, wifiManager);
  processes.push_back(dataManager);
  
  BleManager* bleManager = new BleManager(*wifiManager);
  processes.push_back(bleManager);
  
  // Initialize all processes and pass them the event manager
  for (auto process : processes) {
    process->setup(&eventManager);
  }
}

void loop() {
  // Update all processes
  for (auto process : processes) {
    process->update();
  }
}
