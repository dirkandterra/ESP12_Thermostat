#ifndef MQTT_DR_H
#define MQTT_DR_H
#include <stdint.h>
#include <WString.h>

extern float CurrentTemp;
extern float IncomingSetTemp;
extern String CurrentDateTime;
uint8_t Setup_wifi(void);
void MQTT_Init(void);
void MQTT_CheckConnection(uint32_t hb);
void MQTT_SendHeartbeat(uint32_t hb);
void MQTT_SendTemperature(float temp);
void MQTT_SendSHTCTemperature(float temp);
#endif