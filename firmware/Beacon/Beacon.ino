#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "esp_mac.h"

#define DEVICE_NAME_PREFIX "HitloopBeacon"
// This UUID must match the one in the Scanner's config.h
#define BEACON_SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"

BLEServer* pServer;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("deviceConnected = true");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("deviceConnected = false");
    // Restart advertising after disconnection
    pServer->getAdvertising()->start();
    Serial.println("Advertising restarted");
  }
};

String getUniqueDeviceName() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);

  char macSuffix[5];
  snprintf(macSuffix, sizeof(macSuffix), "%02X%02X", mac[4], mac[5]);
  
  return String(DEVICE_NAME_PREFIX) + "-" + macSuffix;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing...");

  String deviceName = getUniqueDeviceName();
  Serial.print("Device name set to: ");
  Serial.println(deviceName);

  // --- Initialize BLE Device ---
  BLEDevice::init(deviceName.c_str());
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P3);

  // --- Create Server and Service ---
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(BEACON_SERVICE_UUID);
  pService->start(); // Start the service

  // --- Configure and Start Advertising ---
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(BEACON_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();

  Serial.println("Beacon is advertising with service UUID!");
}

void loop() {
  // Nothing to do here for a simple beacon
  delay(2000);
}
