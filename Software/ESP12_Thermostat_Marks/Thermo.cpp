/*
Example sketch for interfacing with the DS1620 temperature chip.

Copyright (c) 2011, Matt Sparks
All rights reserved.
*/
#include <stdlib.h>
#include "Thermo.h"
#include "DR_SHTC3.h"
#include <DS3231.h>
#include <Wire.h>

#define FALSE 0
#define TRUE 1
#define HEATRELAY 12
#define COOLRELAY 13
#define FANRELAY  14

#define INTERVAL_TIMECHECK 500
#define INTERVAL_TEMPCHECK 2000

uint32_t scanMillis=0;
uint8_t validTemp=0;

float tempAvg=70;

bool h12Flag;
bool pmFlag;

bool firstBootSetAutoTemp = true;

datetime_T DT;
thermostat_T Thermostat;

DS3231 myRTC;
SHTC3 mySHTC3;

void UpdateTime();
void UpdateTemp();

void UpdateTime(){
    DT.minute=myRTC.getMinute();
    if(DT.minute==0){
      DT.hour=myRTC.getHour(DT.h12Flag,DT.pmFlag);
      DT.dow=myRTC.getDoW();
    }
}

void UpdateTemp(){
  DT.temperature=(myRTC.getTemperature() * 9/5.0 + 32);
  mySHTC3.update(); 
  DT.SHTCTemp=mySHTC3.toDegF();
}

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
  Serial.print("**");
  Serial.print(minutesAfterMidnight>=Thermostat.beginTime);
  Serial.print(" Off at ");
  Serial.print(Thermostat.endTime);
  Serial.print(" Not weekend: ");
  Serial.println(!(DT.dow==1 || DT.dow==7));
  if((minutesAfterMidnight>=Thermostat.beginTime) && (minutesAfterMidnight<Thermostat.endTime) && !(DT.dow==1 || DT.dow==7)) {
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
    UpdateTime();
    minutesAfterMidnight=DT.hour*60+DT.minute;
    if(minutesAfterMidnight==Thermostat.beginTime){
      if(!lockout){
        setAutoTemp(minutesAfterMidnight);
      }
    }else if(minutesAfterMidnight==Thermostat.endTime){
      if(!lockout){
        setAutoTemp(minutesAfterMidnight);
      }
    }else if(firstBootSetAutoTemp){
      firstBootSetAutoTemp=false;
      Serial.println("FirstBoot");
      setAutoTemp(minutesAfterMidnight);
    }
    else{
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
    UpdateTemp();
    Thermostat.temp=(uint16_t)(DT.SHTCTemp*10);
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
    digitalWrite(HEATRELAY,Thermostat.heatRelay);
    digitalWrite(COOLRELAY,Thermostat.coolRelay);
    tempAvg=tempAvg*.95+DT.SHTCTemp*.05;
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

void ThermoInit()
{
	// Start the I2C interface
	Wire.begin();
 
  mySHTC3.begin();
  myRTC.setClockMode(FALSE);  //Military time
  //myRTC.setHour(7);
  //myRTC.setMinute(37);
  //myRTC.setDoW(6);
  DT.hour=myRTC.getHour(h12Flag,pmFlag);
  DT.minute=myRTC.getMinute();
  DT.dow=myRTC.getDoW();
  Thermostat.hyst=15;
  Thermostat.dayTempHeat=730;
  Thermostat.nightTempHeat=580;
  Thermostat.dayTempCool=720;
  Thermostat.nightTempCool=850;
  Thermostat.beginTime=6.5*60;
  Thermostat.endTime=17.5*60;
  Thermostat.modeChanged=1;                //Force temps to be set after first time check
  setAutoTemp(DT.hour*60+DT.minute);
  DT.SHTCTemp=57;
  Thermostat.mode=MODE_COOL;
  Thermostat.setpoint=580;
  pinMode(HEATRELAY,OUTPUT);
  pinMode(COOLRELAY,OUTPUT);
  //pinMode(FANRELAY,OUTPUT);
}

void ThermoUpdate()
{
  scanMillis=millis();

  checkTemperature();
  checkTime();

}
