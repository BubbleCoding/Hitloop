#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEBeacon.h>
#include "esp_mac.h"

#define DEVICE_NAME_PREFIX "HitloopBeacon"
#define BEACON_UUID_REV "A134D0B2-1DA2-1BA7-C94C-E8E00C9F7A2D"

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
    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    Serial.println("iBeacon advertising restarted");
  }
};

String getUniqueDeviceName() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);

  char macSuffix[5];
  snprintf(macSuffix, sizeof(macSuffix), "%02X%02X", mac[4], mac[5]);
  
  return String(DEVICE_NAME_PREFIX) + "-" + macSuffix;
}

void init_beacon() {
  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->stop();

  pAdvertising->setMinInterval(160);  // 100ms
  pAdvertising->setMaxInterval(240);  // 150ms

  BLEBeacon myBeacon;
  myBeacon.setManufacturerId(0x4c00);
  myBeacon.setMajor(5);
  myBeacon.setMinor(88);
  myBeacon.setSignalPower(0xc5);
  myBeacon.setProximityUUID(BLEUUID(BEACON_UUID_REV));

  BLEAdvertisementData advertisementData;
  advertisementData.setFlags(0x1A);  // General discoverable mode
  advertisementData.setManufacturerData(myBeacon.getData());

  pAdvertising->setAdvertisementData(advertisementData);
  pAdvertising->start();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  String deviceName = getUniqueDeviceName();
  Serial.print("Device name set to: ");
  Serial.println(deviceName);

  BLEDevice::init(deviceName.c_str());
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P3);  // Max TX power

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  init_beacon();

  Serial.println("iBeacon is advertising!");
}

void loop() {
  // iBeacon only — no BLE service or characteristic
  delay(1000);
}
