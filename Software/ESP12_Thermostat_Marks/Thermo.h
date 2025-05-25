#ifndef THERMO_H
#define THERMO_H
#include <stdlib.h>
#include <stdint.h>
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

typedef struct _thermostat{
  uint16_t temp;
  uint8_t mode; //0=Off 1=Heat 2=Cool
  uint8_t status; //0=Not Running 1=Running
  uint8_t modeChanged;
  uint8_t hold;
  uint8_t fanOn;
  uint16_t setpoint;
  uint16_t hyst;
  uint16_t beginTime;    //in minutes after midnight        
  uint16_t endTime;      //in minutes after midnight
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
extern thermostat_T Thermostat;

typedef struct _datetime{
  uint8_t hour;
  uint8_t minute;
  uint8_t dow;
  float temperature;
  float SHTCTemp;
  bool h12Flag;
  bool pmFlag;
  uint32_t timeCheckTarget;
}datetime_T;
extern datetime_T DT;

void ThermoInit(void);
void ThermoUpdate(void);

#endif