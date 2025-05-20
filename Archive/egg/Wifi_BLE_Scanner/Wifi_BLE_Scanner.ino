#include "WiFi.h"
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>

int scanTime = 5;  //In seconds
BLEScan *pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveName()) {
      Serial.print("Device name: ");
      Serial.println(advertisedDevice.getName().c_str());
      Serial.println("");
    }

    if (advertisedDevice.haveServiceUUID()) {
      BLEUUID devUUID = advertisedDevice.getServiceUUID();
      Serial.print("Found ServiceUUID: ");
      Serial.println(devUUID.toString().c_str());
      Serial.println("");
    }

    if (advertisedDevice.haveManufacturerData() == true) {
      String strManufacturerData = advertisedDevice.getManufacturerData();

      uint8_t cManufacturerData[100];
      memcpy(cManufacturerData, strManufacturerData.c_str(), strManufacturerData.length());

      if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00) {
        Serial.println("Found an iBeacon!");
        BLEBeacon oBeacon = BLEBeacon();
        oBeacon.setData(strManufacturerData);
        Serial.printf("iBeacon Frame\n");
        Serial.printf(
          "ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()),
          ENDIAN_CHANGE_U16(oBeacon.getMinor()), oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower()
        );
      } else {
        Serial.println("Found another manufacturers beacon!");
        Serial.printf("strManufacturerData: %d ", strManufacturerData.length());
        for (int i = 0; i < strManufacturerData.length(); i++) {
          Serial.printf("[%X]", cManufacturerData[i]);
        }
        Serial.printf("\n");
      }
    }

    if (advertisedDevice.getFrameType() == BLE_EDDYSTONE_URL_FRAME) {
      Serial.println("Found an EddystoneURL beacon!");
      BLEEddystoneURL EddystoneURL = BLEEddystoneURL(&advertisedDevice);
      Serial.printf("URL bytes: 0x");
      String url = EddystoneURL.getURL();
      for (auto byte : url) {
        Serial.printf("%02X", byte);
      }
      Serial.printf("\n");
      Serial.printf("Decoded URL: %s\n", EddystoneURL.getDecodedURL().c_str());
      Serial.printf("EddystoneURL.getDecodedURL(): %s\n", EddystoneURL.getDecodedURL().c_str());
      Serial.printf("TX power %d (Raw 0x%02X)\n", EddystoneURL.getPower(), EddystoneURL.getPower());
      Serial.println("\n");
    }

    if (advertisedDevice.getFrameType() == BLE_EDDYSTONE_TLM_FRAME) {
      Serial.println("Found an EddystoneTLM beacon!");
      BLEEddystoneTLM EddystoneTLM(&advertisedDevice);
      Serial.printf("Reported battery voltage: %dmV\n", EddystoneTLM.getVolt());
      Serial.printf("Reported temperature: %.2f°C (raw data=0x%04X)\n", EddystoneTLM.getTemp(), EddystoneTLM.getRawTemp());
      Serial.printf("Reported advertise count: %lu\n", EddystoneTLM.getCount());
      Serial.printf("Reported time since last reboot: %lus\n", EddystoneTLM.getTime());
      Serial.println("\n");
      Serial.print(EddystoneTLM.toString().c_str());
      Serial.println("\n");
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");

  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}


void loop() {
  Serial.println("Scan start");

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4ld", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(10);
    }
  }
  Serial.println("Wifi Scan done");

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  // Wait a bit before scanning again.
  delay(3000);

  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices->getCount());
  Serial.println("BLE scan done!");
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
  delay(3000);
}
