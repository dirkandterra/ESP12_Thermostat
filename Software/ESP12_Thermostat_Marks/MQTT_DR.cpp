#include "MQTT_DR.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "secret.h"

float CurrentTemp = -459.68;
float IncomingSetTemp = 0;

String CurrentDateTime;


// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = MQTT_SERVER;

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Don't change the function below. This functions connects your ESP8266 to your router
uint8_t Setup_wifi() {
  uint8_t retry=30;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while ((WiFi.status() != WL_CONNECTED) && retry>0) {
    delay(500);
    Serial.print(".");
    retry--;
  }
  if(retry==0){
    Serial.println("No Wifi!");
    return 0;
  }else{
    Serial.println("");
    Serial.print("WiFi connected - ESP IP address: ");
    Serial.println(WiFi.localIP());
    return 1;
  }
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="esp32/nowTemp"){
    CurrentTemp=messageTemp.toFloat();
    Serial.println(messageTemp);
  }
  else if(topic=="esp32/nowDateTime"){
    CurrentDateTime=messageTemp;
  }
  else if(topic=="ThermESP12/dayCoolTemp"){
    IncomingSetTemp=messageTemp.toFloat();
    Serial.println(IncomingSetTemp);
  }
}

void MQTT_Init(void){
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void MQTT_SendHeartbeat(uint32_t hb){
  char tempString[12];
  Serial.print("Send Heartbeat ");
  Serial.println(hb);
  dtostrf(hb,1,2,tempString);
  client.publish("ThermESP12/hb",tempString);
}

void MQTT_SendTemperature(float temp){
  char tempString[12];
  dtostrf(temp,1,2,tempString);
  client.publish("ThermESP12/ds3231",tempString);
}

void MQTT_SendSHTCTemperature(float temp){
  char tempString[12];
  dtostrf(temp,1,2,tempString);
  client.publish("ThermESP12/SHTC",tempString);
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void MQTT_Reconnect(uint32_t hb) {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ThermESP12",MOSQUITTO_USR,MOSQUITTO_PWD)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("esp32/nowTemp");
      client.subscribe("ThermESP12/dayCoolTemp");
      client.subscribe("esp32/nowDateTime");
      MQTT_SendHeartbeat(hb);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 20 seconds");
      // Wait 20 seconds before retrying
      delay(20000);
    }
  }
}

void MQTT_CheckConnection(uint32_t hb){
  if (!client.connected()) {
    MQTT_Reconnect(hb);
  }
  if(!client.loop()){
    client.connect("ThermESP12",MOSQUITTO_USR,MOSQUITTO_PWD);
  }
}