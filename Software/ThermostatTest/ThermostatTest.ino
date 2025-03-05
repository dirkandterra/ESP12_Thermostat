/*
Example sketch for interfacing with the DS1620 temperature chip.

Copyright (c) 2011, Matt Sparks
All rights reserved.
*/
#include <stdlib.h>
#include "DS1620.h"
#include <DS3231.h>
#include <Wire.h>

#define FALSE 0
#define TRUE 1

enum mode{
  MODE_OFF,
  MODE_HEAT,
  MODE_COOL
};
enum status{
  HVAC_OFF,
  HVAC_RUNNING
};
#define INTERVAL_TIMECHECK 500
#define INTERVAL_TEMPCHECK 2000

uint32_t scanMillis=0;
uint8_t validTemp=0;
DS3231 myRTC;

float tempAvg=70;

bool h12Flag;
bool pmFlag;

DS1620 ds1620(RST_PIN, CLK_PIN, DQ_PIN);

typedef struct _thermostat{
  uint16_t temp;
  uint8_t mode; //0=Off 1=Heat 2=Cool
  uint8_t status; //0=Not Running 1=Running
  uint8_t modeChanged;
  uint8_t hold;
  uint8_t fanOn;
  uint16_t setpoint;
  uint16_t hyst;
  uint8_t beginTime;    //in minutes after midnight        
  uint8_t endTime;      //in minutes after midnight
  uint16_t dayTempCool;
  uint16_t nightTempCool;
  uint16_t dayTempHeat;
  uint16_t nightTempHeat;
  uint8_t tempSamples;
  uint8_t heatRelay;
  uint8_t coolRelay;
  uint8_t fanRelay;
  uint32_t tempCheckTarget;
}thermostat_T;
thermostat_T Thermostat;

typedef struct _datetime{
  uint8_t hour;
  uint8_t minute;
  uint8_t dow;
  uint32_t timeCheckTarget;
}datetime_T;
datetime_T DT;

uint8_t timerRoutine(uint32_t timeNow, uint32_t timeTarget){
  if(timeNow>=timeTarget){
    return TRUE;
  }else{
    return FALSE;
  }
}

void setAutoTemp(uint16_t minutesAfterMidnight){
  uint16_t dayTemp=0;
  uint16_t nightTemp=0;

  if(Thermostat.hold && !Thermostat.modeChanged){
    return;     //only change setpoint if mode changed when on hold
  }
  if(Thermostat.mode==MODE_OFF){
    return;
  }else if(Thermostat.mode==MODE_HEAT){
    dayTemp=Thermostat.dayTempHeat;
    nightTemp=Thermostat.nightTempHeat;
  }else{
    dayTemp=Thermostat.dayTempCool;
    nightTemp=Thermostat.nightTempCool;
  }
  if((minutesAfterMidnight>=Thermostat.beginTime && minutesAfterMidnight<Thermostat.endTime) && !(DT.dow==1 || DT.dow==7)) {
      Thermostat.setpoint=dayTemp;
  }else{
      Thermostat.setpoint=nightTemp;
  }
}

void checkTime(){
  static uint8_t lockout=0; //keeps the temp set from re-triggering the whole minute that the time matches trigger
  uint16_t minutesAfterMidnight=0;
  if(timerRoutine(scanMillis,DT.timeCheckTarget)){
    DT.timeCheckTarget=scanMillis+INTERVAL_TIMECHECK;
    DT.minute=myRTC.getMinute();
    if(DT.minute==0){
      DT.hour=myRTC.getHour(h12Flag,pmFlag);
      DT.dow=myRTC.getDoW();
    }
    minutesAfterMidnight=DT.hour*60+DT.minute;
    if(minutesAfterMidnight==Thermostat.beginTime){
      if(!lockout){
        setAutoTemp(minutesAfterMidnight);
      }
    }else if(minutesAfterMidnight==Thermostat.endTime){
      if(!lockout){
        setAutoTemp(minutesAfterMidnight);
      }
    }else{
      lockout=0;
      if(Thermostat.modeChanged){
        setAutoTemp(minutesAfterMidnight);
      }
    }
    Thermostat.modeChanged=0;
  }
}

void checkTemperature(){
  if(timerRoutine(scanMillis,Thermostat.tempCheckTarget)){
    Thermostat.tempCheckTarget=scanMillis+INTERVAL_TEMPCHECK;
    Thermostat.temp=(myRTC.getTemperature() * 9/5.0 + 32)*10;
    validTemp=1;
    if(Thermostat.mode==MODE_OFF){
      Thermostat.heatRelay=0;
      Thermostat.coolRelay=0;
      return;
    }else if(Thermostat.mode==MODE_HEAT){
      Thermostat.coolRelay=0;
      if(Thermostat.temp>=(Thermostat.setpoint+Thermostat.hyst)){
        Thermostat.heatRelay=0;
      }else if(Thermostat.temp<=(Thermostat.setpoint-Thermostat.hyst)){
        Thermostat.heatRelay=1;
      }
    }else{
      Thermostat.heatRelay=0;
      if(Thermostat.temp<=(Thermostat.setpoint-Thermostat.hyst)){
        Thermostat.coolRelay=0;
      }else if(Thermostat.temp>=(Thermostat.setpoint+Thermostat.hyst)){
        Thermostat.coolRelay=1;
      }    
    }
    if(Thermostat.heatRelay || Thermostat.coolRelay){
      Thermostat.status=HVAC_RUNNING;
    }else{
      Thermostat.status=HVAC_OFF;
    }
    digitalWrite(4,Thermostat.heatRelay);
    tempAvg=tempAvg*.95+(ds1620.temp_c() * 9/5.0 + 32)*.05;
    Serial.print(tempAvg);
    Serial.print(">>");
    Serial.print(Thermostat.temp);
    Serial.print(" - ");
    Serial.println(Thermostat.status);
     Serial.print(DT.hour);
     Serial.print(":");
    Serial.print(DT.minute);
    Serial.print(" ");
    Serial.println(DT.dow);
  }
}

void setup()
{
	// Start the I2C interface
	Wire.begin();
 
	// Start the serial interface
	Serial.begin(115200);
  delay(100);

  ds1620.config();
  myRTC.setClockMode(FALSE);  //Military time
  DT.hour=myRTC.getHour(h12Flag,pmFlag);
  DT.minute=myRTC.getMinute();
  DT.dow=myRTC.getDoW();
  Thermostat.hyst=20;
  Thermostat.mode=MODE_HEAT;
  Thermostat.dayTempHeat=730;
  Thermostat.nightTempHeat=580;
  Thermostat.beginTime=6.5*60;
  Thermostat.endTime=17.5*60;
  Thermostat.modeChanged=1;                //Force temps to be set after first time check
  setAutoTemp(DT.hour*60+DT.minute);
  //myRTC.setHour(7);
  //myRTC.setMinute(37);
  //myRTC.setDoW(6);
  pinMode(4,OUTPUT);
}

void loop()
{
  scanMillis=millis();
  const float temp_f = ds1620.temp_c() * 9/5.0 + 32;
  const float rtcTemp_f = myRTC.getTemperature() * 9/5.0 + 32;

  checkTemperature();
  checkTime();

}
