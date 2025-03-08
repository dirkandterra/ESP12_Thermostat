//
//    FILE: PCF8575_test.ino
//  AUTHOR: Rob Tillaart
//    DATE: 2020-07-20
// PURPOSE: test PCF8575 library
//     URL: https://github.com/RobTillaart/PCF8575


#include "MQTT_DR.h"
#include "GLCD12864.h"
#include "DS3231.h"
#include "DR_SHTC3.h"
#include "Thermo.h"
#include "secret.h"

#define GLCD_PIN 16
uint8_t firstTime=1;


float temp_f=0;
char phrase[2][26]={{"3231: "},{"SHTC: "}};
uint8_t wifiConnected=0;
uint32_t heartbeatCount=0;
String str;

void SHTC3_Init(){
  //Wire.begin();
  
}

void getTime(){
  static int hbtime =0;
  hbtime++;
    if(hbtime>=30){
      heartbeatCount++;
      MQTT_SendHeartbeat(heartbeatCount);
      hbtime=0;
    }

    if(hbtime%10==0){
      MQTT_SendTemperature(DT.temperature);
    }
    else if(hbtime%5==0){
      MQTT_SendSHTCTemperature(DT.SHTCTemp);
    }
    Serial.print(DT.temperature,1);
    Serial.print("F - ");
    Serial.println(DT.SHTCTemp);
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
  ThermoInit();
  glcdInit(400000,GLCD_PIN);
}


void loop()
{
  static uint8_t toggle=0;
  uint16_t page=0x0CB8;
  uint16_t x=0,y=0;
  char charArray[10];
  ThermoUpdate();
  String tempStr= String(DT.temperature,2);
  Serial.println(tempStr);
  setCoord(0,7);
  lcdPrintChar(00+0x20);
  setCoord(0,0);
  //Serial.println("Start");
  getTime();
  printPhrase(phrase[0]);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  printPhrase(charArray);
  lcdPrintChar('F');
  setCoord(0,1);
  tempStr="";
  if(DT.hour<10){
    tempStr=" ";
  }
  tempStr+= String(DT.hour) + ":";
  if(DT.minute<10){
    tempStr+="0";
  } 
  tempStr+=String(DT.minute);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  printPhrase(charArray);
  setCoord(0,2);
  tempStr=String(DT.SHTCTemp);
  printPhrase(phrase[1]);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  printPhrase(charArray);
  lcdPrintChar('F');
  setCoord(0,3);
  if(CurrentTemp!=""){
    CurrentTemp.toCharArray(charArray, CurrentTemp.length() + 1);
    printPhrase(charArray);
    lcdPrintChar('F');
  }
  setCoord(121,0);
  if(wifiConnected){
    lcdCustomPrintChar(WIFINETWORK);
  }else{
    lcdCustomPrintChar(NONETWORK);
  }

  /*setCoord(0,0);
  for(y=32;y<132;y++){
    lcdPrintChar(y);
  }*/
  MQTT_CheckConnection(heartbeatCount);

  delay(2000);
  //InvertFont=!InvertFont;
  //C2, C1, RS, RW, 7-0

}


//  -- END OF FILE --

