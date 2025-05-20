#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEBeacon.h>

#define DEVICE_NAME     "ESP32-3"
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

  BLEDevice::init(DEVICE_NAME);
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
