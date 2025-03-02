//
//    FILE: PCF8575_test.ino
//  AUTHOR: Rob Tillaart
//    DATE: 2020-07-20
// PURPOSE: test PCF8575 library
//     URL: https://github.com/RobTillaart/PCF8575


#include "MQTT_DR.h"
#include "GLCD12864.h"
#include "DS3231.h"
#include "secret.h"

#define GLCD_PIN 16

typedef struct _datetime{
  uint8_t hour;
  uint8_t minute;
  uint8_t dow;
  float temperature;
  bool h12Flag;
  bool pmFlag;
}datetime_T;

void UpdateTime(datetime_T *DT);
void UpdateTemp(datetime_T *DT);

DS3231 myRTC;
float temp_f=0;
datetime_T DT;
char phrase[2][26]={{"Temp1: "},{"Temp2: "}};
uint8_t wifiConnected=0;
uint32_t heartbeatCount=0;
String str;



void UpdateTime(){
    DT.minute=myRTC.getMinute();
    if(DT.minute==0){
      DT.hour=myRTC.getHour(DT.h12Flag,DT.pmFlag);
      DT.dow=myRTC.getDoW();
    }
}

void UpdateTemp(){
  DT.temperature=(myRTC.getTemperature() * 9/5.0 + 32);
}

void getTime(){
  static int hbtime =0;
  
  hbtime++;
    if(hbtime>=30){
      heartbeatCount++;
      MQTT_SendHeartbeat(heartbeatCount);
      hbtime=0;
    }
    UpdateTime();
    UpdateTemp();
    
    MQTT_SendTemperature(DT.temperature);

    Serial.print(DT.temperature,1);
    Serial.print("F - ");
    Serial.print(DT.hour);
    Serial.print(":");
    Serial.print(DT.minute);
    Serial.print(" ");
    Serial.println(DT.dow);
}



void setup()
{
  Serial.begin(115200);
  wifiConnected=Setup_wifi();
  MQTT_Init();
  glcdInit(400000,GLCD_PIN);
}


void loop()
{
  static uint8_t toggle=0;
  uint16_t page=0x0CB8;
  uint16_t x=0,y=0;
  char charArray[10];
  String tempStr= String(DT.temperature);
  Serial.println(tempStr);
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
  tempStr= String(DT.hour) + ":" +String(DT.minute);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  printPhrase(charArray);
  setCoord(0,2);
  if(CurrentTemp!=""){
    CurrentTemp.toCharArray(charArray, CurrentTemp.length() + 1);
    printPhrase(charArray);
    lcdCustomPrintChar('F');
  }
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
  MQTT_CheckConnection(heartbeatCount);

  delay(2000);
  //InvertFont=!InvertFont;
  //C2, C1, RS, RW, 7-0

}


//  -- END OF FILE --

