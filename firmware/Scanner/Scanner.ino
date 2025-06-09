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
#include "SharedState.h"

Config config;

// Global pointer for BLE callback
BleManager* g_bleManager = nullptr;
// Central Event Manager
EventManager eventManager;
SharedState sharedState;

// Create instances of all the processes
SystemManager systemManager(config, sharedState);
WifiManager wifiManager(config, sharedState);
LedManager ledManager;
VibrationManager vibrationManager;
IMUManager imuManager;
BleManager bleManager(&imuManager);
DataManager dataManager(sharedState);
HTTPManager httpManager(config);
BehaviorManager behaviorManager(&ledManager, &vibrationManager);

Process* processes[] = {
    &systemManager,
    &wifiManager,
    &ledManager,
    &vibrationManager,
    &imuManager,
    &bleManager,
    &dataManager,
    &httpManager,
    &behaviorManager
};

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  config.loadConfig();

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
