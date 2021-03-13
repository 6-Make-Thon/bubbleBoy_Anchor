/*
   Basend on Arduino ESP32 BLE Scan and BLE Beacon scan

   This program scans nearby BLE devices and prints there ProximityUUID if exists
*/

// http://www.neilkolban.com/esp32/docs/cpp_utils/html/class_b_l_e_advertised_device.html#a1c26f896a17c92cbeb59c7f061d510bc
// https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE/src


#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

        BLEBeacon oBeacon = BLEBeacon();
        oBeacon.setData(advertisedDevice.getManufacturerData());
        BLEUUID uuid = oBeacon.getProximityUUID();

        /* 
        * Only devices of type iBeacon
        */
        if((uuid.bitSize() != 0) && !uuid.equals(BLEUUID("00000000-0000-0000-0000-000000000000"))) {

          Serial.println(uuid.toString().c_str());
          Serial.printf("BLEUUID %s \n", uuid.toString().c_str());
          Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
          Serial.printf("getRSSI: %d \n", advertisedDevice.getRSSI());

        }

    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(10);
  pBLEScan->setWindow(9);  // less or equal setInterval value
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  // Serial.print("Devices found: ");
  // Serial.println(foundDevices.getCount());
  // Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(200);
}
