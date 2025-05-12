#include <BLEDevice.h>
#include <BLEBeacon.h>
#include <BLEUtils.h>
#include <BLEServer.h>

BLEAdvertising* pAdvertising;

void setup() {
  Serial.begin(115200);
  BLEDevice::init("Beacon_1"); // Optional name

  BLEServer* pServer = BLEDevice::createServer();

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x004C); // Apple ID
  oBeacon.setProximityUUID(BLEUUID("870469bf-ecd2-0cb1-2544-50293cd8bf54"));
  oBeacon.setMajor(1);
  oBeacon.setMinor(1);
  oBeacon.setSignalPower(-59);

  BLEAdvertisementData oAdvertisementData;
  BLEAdvertisementData oScanResponseData;
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED

  String strServiceData;

  strServiceData += (char)26; // Length of payload
  strServiceData += (char)0xFF;
  strServiceData += oBeacon.getData(); 
  oAdvertisementData.addData(strServiceData);


  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);


  BLEDevice::startAdvertising();
  Serial.println("iBeacon started");
}

void loop() {
  delay(1000);
}
