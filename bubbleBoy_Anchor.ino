/*
   Basend on Arduino ESP32 BLE Scan and BLE Beacon scan

   This program scans nearby BLE devices and prints there ProximityUUID if exists
*/

// http://www.neilkolban.com/esp32/docs/cpp_utils/html/class_b_l_e_advertised_device.html#a1c26f896a17c92cbeb59c7f061d510bc
// https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE/src


// BLE includes
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>

// MQTT
#include <WiFi.h>         // ESP Wifi
#include <PubSubClient.h> //MQTT Client

#include "config.h"

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

WiFiClient espClient;           // WiFi
PubSubClient client(espClient); // MQTT Node

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

          int rssi = advertisedDevice.getRSSI();

          sendMqtt(String(NAME), uuid, rssi);
        }

    }
};

// Sendet Daten über MQTT an den Server
void sendMqtt(String name, BLEUUID uuid, int rssi) {

  do {
    Serial.print(".");
    // Try to connect - if fail repeat
    if (client.connect("bubbleboy")) {
      // 'beaconator/ble/NAME/X-KOORDINATE/Y-KOORDINATE/raw/MAC
      String topic = "beaconator/ble/" + name + "/raw/" + String(uuid.toString().c_str());   // Topic = ID + Channel

      Serial.println("MQTT publish to topic " + topic); // Debug
      client.publish(topic.c_str(), String(rssi).c_str());   // Send Data
    }
    delay(500);
  } while (!client.connected());

}

/* 
 * Gets called when message is send to the scanner
 */
void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT Subscription Callback: ");
  Serial.print(topic);
  Serial.print(" : ");
  for(int i = 0; i < length; i++) {
    Serial.printf("%x ", payload[i]);
  }


  // TODO
  /*
   * Sent the data to the shopping cart
   * Connect to the beacon ofer BLE and send data
   * One time connection - after that disconnect
   */



}

void setup() {

  Serial.begin(115200);

  // Init MQTT Client
  client.setServer(MQTT_SERVER, MQTT_PORT); // MQTT Client

  // Connect to WLAN
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  delay(1500);

  // Does not work if the first connection is made in onResult
  // sendMqtt(String(NAME), BLEUUID("00000000-0000-0000-0000-000000000000"), 22);
  do {
    Serial.print(".");
    client.connect("bubbleboy");
    delay(500);
  } while (!client.connected());

  /*
   * Subscribe to MQTT topic
   */
  String subscribe_topic = "beaconator/ble/downlink/" + String(NAME) + "/#";
  boolean subscribe_success = client.subscribe(subscribe_topic.c_str());
  Serial.println(subscribe_success);
  client.setCallback(mqttSubscriptionCallback);

  Serial.println("Scanning...");

  // Init BLE Scanner

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
  /*
   * Clear result cache
   */
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

  /*
   * Let the PubSubClient client update the subscription
   */
  client.loop();
  delay(200);
}
