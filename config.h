
#define NAME            "receiver001"


#define WIFI_SSID       "XXXXXXXXXXXXX"         // WiFi SSID
#define WIFI_PASSWORD   "XXXXXXXXXXXXX"         // WiFi Secret

#define MQTT_SERVER     "XXX.XXX.XXX.XXX"       // MQTT Broker Server
#define MQTT_PORT       1883                    // MQTT Broker Port

void sendMqtt(String name, BLEUUID uuid, int rssi);
void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length);