//
//    FILE: PCF8575_test.ino
//  AUTHOR: Rob Tillaart
//    DATE: 2020-07-20
// PURPOSE: test PCF8575 library
//     URL: https://github.com/RobTillaart/PCF8575


#include "MQTT_DR.h"
#include "lcd.h"
#include "DS3231.h"
#include "DR_SHTC3.h"
#include "Thermo.h"
#include "secret.h"
#include "splash.h"

#define GLCD_PIN 16
uint8_t firstTime=1;


float temp_f=0;
char phraseSp[10]="Setpoint:";
char phraseOut[5]="Out:";
char dow[8][4]={" ","Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
uint8_t wifiConnected=0;
uint32_t heartbeatCount=0;
String str;

void mqttSend(){
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
    //Serial.print(DT.temperature,1);
    //Serial.print("F - ");
    //Serial.println(DT.SHTCTemp);
    Serial.print(Thermostat.setpoint);
    Serial.print("-->");
    Serial.println(Thermostat.beginTime);
}



void setup()
{
  Serial.begin(115200);
  wifiConnected=Setup_wifi();
  MQTT_Init();
  ThermoInit();
  LcdInit(200000);
}


void loop()
{
  static uint8_t toggle=0;
  uint16_t x=0,y=0;
  char charArray[10];
  ThermoUpdate();
  String tempStr= String((float)Thermostat.setpoint/10,1);
  Serial.println(tempStr);
  //Serial.println("Start");
  mqttSend();
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  lcdDrawString(0,0,(int8 *)phraseSp);
  lcdDrawString(10,0,(int8 *)charArray);
  tempStr=dow[DT.dow];
  tempStr+=" ";
  if(DT.hour<10){
    tempStr+=" ";
  }
  tempStr+= String(DT.hour) + ":";
  if(DT.minute<10){
    tempStr+="0";
  } 
  tempStr+=String(DT.minute);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  lcdDrawString(0,7,(int8 *)charArray);
  if(Thermostat.status==HVAC_RUNNING){
    if(Thermostat.mode==MODE_COOL){
      lcdDrawBitmap(120,32,(const ABitmap *) &bigFlake1);
    }else{
      lcdDrawBitmap(120,32,(const ABitmap *) &bigFlame1);
    }
  }else{
    lcdDrawBitmap(120,32,(const ABitmap *) &eraseIcon);
  }
  tempStr=String(DT.SHTCTemp,1);
  tempStr.toCharArray(charArray, tempStr.length() + 1);
  LARGE_DrawString((int8 *)charArray,32,24,0);
  
  if(CurrentTemp>-400){
    CurrentTemp = (float)((uint16_t)(CurrentTemp*100))/100;   //get to 2 decimal places
    tempStr=String(CurrentTemp,0);
    tempStr.toCharArray(charArray, tempStr.length() + 1);
    lcdDrawString(10,7,(int8_t *)phraseOut);
    lcdDrawString(14,7,(int8 *)charArray);
  }
  lcdSetCursor(121,0);
  if(wifiConnected){
    lcdWriteChar(15,0,'W');
  }else{
    lcdWriteChar(15,0,'-');
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

