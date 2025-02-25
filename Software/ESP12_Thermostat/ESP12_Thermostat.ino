//
//    FILE: PCF8575_test.ino
//  AUTHOR: Rob Tillaart
//    DATE: 2020-07-20
// PURPOSE: test PCF8575 library
//     URL: https://github.com/RobTillaart/PCF8575


#include "Wire.h"
#include "c64Font.c"
#include "GameBoyFont.c"
#define pcfAddress 0x20
#include "DS1620.h"
#include <DS3231.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "secret.h"

static const uint8_t RST_PIN = 13;  // pin 3 on DS1620
static const uint8_t CLK_PIN = 12;  // pin 2 on DS1620
static const uint8_t DQ_PIN  = 14;  // pin 1 on DS1620

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = MQTT_SERVER;

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

DS1620 ds1620(RST_PIN, CLK_PIN, DQ_PIN);
float temp_f=0;

#define FONT_USED c64font
#define FONT_PRINT_WIDTH 8
#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define LCD_WIDTH 128
#define LCD_HEIGHT 64
#define LCD_CHIP_WIDTH 64

#define NONETWORK 128
#define WIFINETWORK 127

#define CUSTOM_FONT_INDEX GameBoyFont__index
#define CUSTOM_FONT_BITMAP GameBoyFont__bitmap

uint8_t customX=0;
uint8_t InvertFont=0;
char phrase[2][26]={{"Temp1: "},{"Temp2: "}};
String temperatureDisplay="";
uint8_t wifiConnected=0;


DS3231 myRTC;
bool h12Flag;
bool pmFlag;
typedef struct _datetime{
  uint8_t hour;
  uint8_t minute;
  uint8_t dow;
  uint32_t timeCheckTarget;
}datetime_T;
datetime_T DT;

typedef struct _xy{
  uint8_t x;
  uint8_t y;
}Coordinates;
float temperature=0;

Coordinates textCoord;

uint16_t lastRead=0x00;

uint32_t heartbeatCount=0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
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
    wifiConnected=0;
  }else{
    Serial.println("");
    Serial.print("WiFi connected - ESP IP address: ");
    Serial.println(WiFi.localIP());
    wifiConnected=1;
  }

  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
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
    temperatureDisplay=messageTemp;
      Serial.println(messageTemp);
  }
}

void sendHeartbeat(){
  char tempString[12];
  heartbeatCount++;
  Serial.print("Send Heartbeat ");
  Serial.println(heartbeatCount);
  dtostrf(heartbeatCount,1,2,tempString);
  client.publish("ThermESP12/hb",tempString);
  dtostrf(temperature,1,2,tempString);
  client.publish("ThermESP12/ds3231",tempString);
  dtostrf(temp_f,1,2,tempString);
  client.publish("ThermESP12/ds1620",tempString);
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ThermESP12",MOSQUITTO_USR,MOSQUITTO_PWD)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("esp32/nowTemp");
      sendHeartbeat();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 20 seconds");
      // Wait 20 seconds before retrying
      delay(20000);
    }
  }
}

void getTime(){
  static int hbtime =0;
  temp_f = temp_f * 0.05 + .95 * (ds1620.temp_c() * 9/5.0 + 32);
  temperature=(myRTC.getTemperature() * 9/5.0 + 32);
  hbtime++;
    if(hbtime>=30){
      sendHeartbeat();
      hbtime=0;
    }
    DT.minute=myRTC.getMinute();
    if(DT.minute==0){
      DT.hour=myRTC.getHour(h12Flag,pmFlag);
      DT.dow=myRTC.getDoW();
    }
    
    Serial.print(temperature,1);
    Serial.print("F - ");
    Serial.print(DT.hour);
    Serial.print(":");
    Serial.print(DT.minute);
    Serial.print(" ");
    Serial.println(DT.dow);
    Serial.println(temp_f);
}

uint16_t read16()
{
  uint16_t _dataIn = 0;
  if (Wire.requestFrom((uint8_t)pcfAddress, (uint8_t)2) != 2)
  {
    return lastRead;                   //  last value
  }
  _dataIn = Wire.read();            //  low 8 bits
  _dataIn |= (Wire.read() << 8);    //  high 8 bits
  lastRead=_dataIn;
  return _dataIn;
}

bool write16(const uint16_t value)
{
  uint16_t dataOut = value;
  Wire.beginTransmission(pcfAddress);
  Wire.write(dataOut & 0xFF);      //  low 8 bits
  Wire.write(dataOut >> 8);        //  high 8 bits
  return Wire.endTransmission();
}


void printHex(uint16_t x)
{
  if (x < 0x1000) Serial.print('0');
  if (x < 0x100)  Serial.print('0');
  if (x < 0x10)   Serial.print('0');
  Serial.println(x, HEX);
}

void lcdWrite(uint16_t data){
  write16(data);
  digitalWrite(16,1);
  digitalWrite(16,1);
  digitalWrite(16,0);
  digitalWrite(16,0);
}
void setCoord(uint8_t x, uint8_t y){
  textCoord.x = x;
  customX=x;
  textCoord.y = y;
}

void setBitCoordX(uint8_t x, uint8_t chip){
  uint16_t data=0;
  data = (chip<<2)*256+0x40+x;
  lcdWrite(data);
}

void setBitCoordY(uint8_t y, uint8_t chip){
  uint16_t data=0;
  data = (chip<<2)*256+0xB8+y;
  lcdWrite(data);
}

void lcdClearScreen(){
  uint8_t chipSel=3;
  uint16_t temp;
  uint8_t data=0x00;
  int ii=0,jj=0;
  if(InvertFont){
    data=0xFF;
  }
  for(ii=0;ii<(LCD_HEIGHT/8);ii++){
    setBitCoordY(ii,chipSel); //Set page or line
    setBitCoordX(0,chipSel); //Set horizontal pixel
    for(jj=0;jj<LCD_CHIP_WIDTH;jj++){
      lcdWrite((chipSel<<2)*256 + 0x200 + data);
    }
  }
}

void lcdCustomPrintChar(unsigned char c){
  c=c-0x20;
  //Serial.println(c,DEC);
  uint16_t temp=0;
  uint16_t fontBase=CUSTOM_FONT_INDEX[c];
  uint8_t fontPrintWidth=CUSTOM_FONT_BITMAP[fontBase];
  uint8_t fontWidth=fontPrintWidth+0;
  uint8_t chipSel=0x00;
  uint8_t pixelX=customX;
  uint8_t lastLine=fontWidth;
  int ii=0;
  fontBase++;
  //If the char won't fit on this line, jump to the next
  if((pixelX+fontPrintWidth)>=LCD_WIDTH){
    chipSel=1;
    pixelX=0;    
    textCoord.x=0;
    textCoord.y++;
    //If next line won't fit on the screen, start at the top
    if((textCoord.y*FONT_HEIGHT)>=LCD_HEIGHT){
      textCoord.y=0;
    }
  }
  // If the character begines on chip two, get the chip select correct
  else if(pixelX>=LCD_CHIP_WIDTH){
    //Do we have room for the full spacer after the char before LCD width end?
    if((pixelX+fontWidth)>=LCD_WIDTH){
      lastLine=LCD_WIDTH-pixelX-1;      //Print only char and blank lines that fit
    }
    chipSel=2;
    pixelX=pixelX-LCD_CHIP_WIDTH;
  }
  else{
    chipSel=1;
  }
  setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
  setBitCoordX(pixelX,chipSel); //Set horizontal pixel
  for(ii=0;ii<fontPrintWidth;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    temp=CUSTOM_FONT_BITMAP[fontBase+ii];
    if(InvertFont){
      temp = ~temp&0xFF;
    }
    lcdWrite((chipSel<<2)*256+0x0200+temp);
    pixelX++;
  }
  for(ii=fontPrintWidth;ii<=lastLine;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    if(!InvertFont){
      lcdWrite((chipSel<<2)*256+0x0200+0x00);
    }else{
      lcdWrite((chipSel<<2)*256+0x0200+0xFF);
    }
    pixelX++;
  }
  customX=pixelX+(chipSel-1)*64;
}

void lcdPrintChar(char c){
  uint16_t temp=0;
  uint8_t chipSel=0x00;
  uint8_t pixelX=textCoord.x*FONT_WIDTH;
  uint8_t lastLine=FONT_WIDTH;
  int ii=0;
  //If the char won't fit on this line, jump to the next
  if((pixelX+FONT_PRINT_WIDTH)>=LCD_WIDTH){
    chipSel=1;
    pixelX=0;    
    textCoord.x=0;
    textCoord.y++;
    //If next line won't fit on the screen, start at the top
    if((textCoord.y*FONT_HEIGHT)>=LCD_HEIGHT){
      textCoord.y=0;
    }
  }
  // If the character begines on chip two, get the chip select correct
  else if(pixelX>=LCD_CHIP_WIDTH){
    //Do we have room for the full spacer after the char before LCD width end?
    if((pixelX+FONT_WIDTH)>=LCD_WIDTH){
      lastLine=LCD_WIDTH-pixelX-1;      //Print only char and blank lines that fit
    }
    chipSel=2;
    pixelX=pixelX-LCD_CHIP_WIDTH;
  }
  else{
    chipSel=1;
  }
  setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
  setBitCoordX(pixelX,chipSel); //Set horizontal pixel
  for(ii=0;ii<FONT_PRINT_WIDTH;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    temp=FONT_USED[(c-0x20)*FONT_PRINT_WIDTH+ii];
    if(InvertFont){
      temp = ~temp&0xFF;
    }
    lcdWrite((chipSel<<2)*256+0x0200+temp);
    pixelX++;
  }
  for(ii=FONT_PRINT_WIDTH;ii<=lastLine;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    if(!InvertFont){
      lcdWrite((chipSel<<2)*256+0x0200+0x00);
    }else{
      lcdWrite((chipSel<<2)*256+0x0200+0xFF);
    }
    pixelX++;
  }
  textCoord.x++;
  if((textCoord.x*FONT_WIDTH)>=LCD_WIDTH){
    textCoord.x=0;
    textCoord.y++;
    if((textCoord.y*FONT_HEIGHT)>=LCD_HEIGHT){
      textCoord.y=0;
    }
  }
}

void printPhrase(char *ph){
  int x=0;
  while(ph[x]!=0x00){
    lcdCustomPrintChar(ph[x]);
    x++;
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(16,OUTPUT);
  Wire.setClock(400000);
  Wire.begin();
  ds1620.config();
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  delay(500);

  lcdWrite(0x0C3F);
  lcdWrite(0x0C40);
  lcdWrite(0x0CC0);
  delay(500);
  lcdClearScreen();
}


void loop()
{
  static uint8_t toggle=0;
  uint16_t page=0x0CB8;
  uint16_t x=0,y=0;
  char charArray[10];
  String tempStr= String(temperature);
  setCoord(0,7);
  lcdPrintChar(00+0x20);
  setCoord(0,0);
  //Serial.println("Start");
  getTime();
  printPhrase(phrase[0]);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  printPhrase(charArray);
  lcdCustomPrintChar('F');
  setCoord(0,1);
  printPhrase(phrase[1]);
  tempStr= String(temp_f);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  printPhrase(charArray);
  lcdCustomPrintChar('F');
  setCoord(121,0);
  if(wifiConnected){
    lcdCustomPrintChar(WIFINETWORK);
  }else{
    lcdCustomPrintChar(NONETWORK);
  }

  /*setCoord(0,0);
  for(y=32;y<132;y++){
    lcdCustomPrintChar(y);
  }*/

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()){
    client.connect("ThermESP12",MOSQUITTO_USR,MOSQUITTO_PWD);
  }
  delay(2000);
  //InvertFont=!InvertFont;
  //C2, C1, RS, RW, 7-0

}


//  -- END OF FILE --

