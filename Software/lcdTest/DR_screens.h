//--------------------------------------------------------
//  $Id: screens.h 441 2010-09-03 03:58:39Z david $
//  $Date: 2010-09-03 11:58:39 +0800 (Fri, 03 Sep 2010) $
//  $Rev: 441 $
//  Brief description: 	Screen Setup & Handling                      
//  Last changed by $Author: david $
//--------------------------------------------------------
#ifndef SCREENS_H
#define SCREENS_H

#include <stdint.h>

typedef enum {
	TYPE_NONE = 0,
	TYPE_LABEL,
	TYPE_STRING,
	TYPE_SINGLESTRING,
	TYPE_INT_8,
	TYPE_INT_16,
	TYPE_INT_32,
	TYPE_UINT_8,
	TYPE_UINT_16,
	TYPE_UINT_32,
	TYPE_PHONE,
	TYPE_HR_MIN,
	TYPE_DECIMAL,
	TYPE_NOSHOW,
  TYPE_OBJ_END
} ScreenObjTypes;

typedef struct {
	uint8_t x;
	uint8_t y;
} ScreenObjCoords;

typedef struct {
	uint8_t type;
	uint8_t * Format_Or_Num;
	uint8_t * Value;
	uint32_t MinValue;
	uint32_t MaxValue;
	ScreenObjCoords Coords;
} ScreenObj;

typedef enum {
	KEY_NONE = 0, KEY, KEY_END
} KeyTypes;

typedef struct {
	uint8_t KeyType;
	int8_t LeftColumn;
	int8_t RightColumn;
	uint8_t * Value;
} Keys;

typedef struct {

	uint16_t DS3231Temp;	//* 10
	uint16_t SHTCTemp;		//* 10
	uint8_t TimeString[12];
	uint8_t DateString[9];

} Screen;

extern Screen ScreenVars;
extern uint8_t MainBanner[17];
extern const uint8_t dowText[8][4];

extern const Keys MainKeys[];
void updateTempsAndTime(void);
void ScreenDraw(uint8_t screen);


#endif

