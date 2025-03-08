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
#include "core.h"

#define BANNERCYCLESP	3	//Seconds between changing notifications on main banner
typedef enum {
	LABEL = 0, VARIABLE, NOSHOW, GADGET_END
} GadgetTypes;

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
#ifdef ChemCalNumber_Bigger
	TYPE_UINT_32,
#endif
	TYPE_PHONE,
	TYPE_HR_MIN,
	TYPE_DECIMAL
} GadgetSubTypes;

typedef struct {
	uint8_t x;
	uint8_t y;
} GadgetCoords;

typedef struct {
	uint8_t type;
	uint8_t subtype;
	uint8_t * Format_Or_Num;
	uint8_t * Value;
	uint32_t MinValue;
	uint32_t MaxValue;
	GadgetCoords Coords;
} Gadgets;

typedef enum {
	KEY_NONE = 0, KEY, KEY_END
} KeyTypes;

typedef struct {
	uint8_t KeyType;
	int8_t LeftColumn;
	int8_t RightColumn;
	uint8_t * Value;
} Keys;

typedef enum {
	ALARM_STOP_1,           //0
	ALARM_STOP_2,           //1
	ALARM_STOP_3,           //2
	ALARM_WTR_PRES_HI,      //3
	ALARM_WTR_PRES_LO,      //4
	ALARM_CHM_PRES_HI,      //5
	ALARM_CHM_PRES_LO,      //6
	ALARM_USBLOG_ERROR,     //7
	ALARM_NOFLOW_ERROR,     //8
	ALARM_NOTATRATE_ERROR,  //9
	ALARM_NOWATERFLOW_ERROR,  //10
	ALARM_NOPOWER_ERROR,	//11
	ALARM_AUXDI_ERROR,		//12
	ALARM_BATCHDONE,		//13
	ALARM_EEPROM_ERROR,		//14
	ALARM_END
} AlarmTypes;

//#define ALARM_CRITICAL_MASK 0x7f // Stop signals ***NOT USED

// ml to 1/10 gal
#define ML_TO_GAL(ml)       ((uint16_t)(ml * 1000 / 378541))
// 1/10 gal to ml
#define GAL_TO_ML(gal)      ((uint16_t)((uint32_t)gal * 378541 / 1000))
// mL to oz
#define ML_TO_OZ(ml)        ((uint16_t)(ml * 10000 / 295735))
// oz to mL
#define OZ_TO_ML(oz)        ((uint16_t)((uint32_t)oz * 295735 / 10000))

typedef struct {
	uint8_t *Message;
} Alarms;

typedef struct {

	uint8_t COPReset;

	uint16_t ChemRate;
#ifndef CodeChange_1_4_2022
	uint8_t ChemPulsesIn;
#endif
	uint16_t ChemPress;

	uint16_t WaterRate;
	uint8_t WtrPulsesIn;
	uint16_t WaterPress;

	uint16_t RateAc;

	int32_t UserTotal;

	// Diagnostic Menu

	uint8_t TimeString[12];
	uint8_t DateString[9];

	uint8_t Hour; // This is what gets read from the RTC module
	uint8_t Min; // This is what gets read from the RTC module
	uint8_t Sec; // This is what gets read from the RTC module

	uint8_t setHour; // This is what gets written to the RTC module
	uint8_t setMin; // This is what gets written to the RTC module
	uint8_t setSec; // This is what gets written to the RTC module

	uint8_t setDay; // This is what gets written to the RTC module
	uint8_t setMonth; // This is what gets written to the RTC module
	uint8_t setYear; // This is what gets written to the RTC module

	uint8_t Day; // This is what gets read from the RTC module
	uint8_t Month; // This is what gets read from the RTC module
	uint8_t Year; // This is what gets read from the RTC module

	uint8_t FileCount;
	//uint8_t USBMessage[14];													
	uint8_t USBString[9];
	uint8_t USBInserted;

	uint8_t AuxDigIn;

	uint16_t MotorSpeed;
	uint16_t OzPerMin;
	uint16_t ProcessVariable;
	uint16_t ChemFreq;
	uint16_t WaterFreq;
	uint16_t GPAFlow;
	uint16_t GPMFlow;
	uint16_t GPHFlow;
	uint16_t OZPAFlow;
	uint16_t OZPMFLow;
	uint16_t TargetRate;
	uint16_t CurrentRate;

	uint16_t ButtonPressed;

	uint8_t Direction;
	uint8_t ChemTotReset;
	uint8_t UserTotReset;
	uint8_t WaterTotReset;

#ifndef CodeChange_1_4_2022
	uint16_t PulseCount;
#endif
	uint16_t RatePTime;
	uint16_t AdjRate;

	uint8_t SigStrength;
	uint8_t SigStrBar;
	uint8_t connStatus;
	uint8_t DataService;

	uint8_t FlowTimeLabel[17];
	uint8_t AdjRateLabel[17];
	uint8_t BatchSPMainMenuLabel[17];

	uint16_t AlarmMask;
	uint8_t StopNum;

	uint16_t AuxPress;
	uint8_t TankLevelAlarm;

	uint8_t BatchedPivotKillEnable;

	uint8_t LoginFileSaved;

	uint8_t Calib_420Cmd;
	uint8_t Calib_420Done;

	uint8_t AssemblyTestState;
	uint8_t AssemblyTestCmd;
	uint8_t AssemblyTestSystem;
	uint16_t AssemblyTestTimer;
	uint8_t AssemblyTimeToRate;
	uint8_t AssemblyCyclesInDeadband;
	uint8_t AssemblySecAtRate;
	uint8_t AssemblyAtRateDB;
	uint8_t AssemblyMaxPressure;
	uint8_t AssemblyFirstFailure;

	unsigned char APN[17];
	unsigned char ICCID[2][11];
	uint8_t ProvRequest;

#ifdef EnableGPSRate
	int32_t CenterLatDegree;
	int32_t CenterLonDegree;
#endif
	
#ifdef DRFLASHTEST
int8_t DRTEST;					//TODO: DR TEST
#endif  	

} Screen;

typedef struct {
	uint8_t running :1;			//Show "Running"
	uint8_t delayingStart :1;	//Show "Delaying Start"
	uint8_t delayingBatchPK :1;		//Show "Delaying PK"
} bannerNotificationFlags;

extern bannerNotificationFlags mainBannerFlags;

extern Screen ScreenVariables;

extern uint8_t AlarmMessages[][17];
extern uint8_t NonAlarmStopMessages[][17];
extern const uint8_t SchedModeString[3][5];
extern const uint8_t WaterTotModeString[3][5];

extern uint8_t displayDelay[6];

extern const Keys MainMenuKeys[];
extern const Keys ChemicalMenuKeys[];
extern const Keys RateMenuKeys[];
extern const Keys BatchMenuKeys[];
extern const Keys ScheduleMenuKeys[];
extern const Keys Setting1MenuKeys[];
extern const Keys AlarmMenuKeys[];
extern const Keys WaterMenuKeys[];
extern const Keys SettingMenuKeys[];
extern const Keys ClockMenuKeys[];
extern const Keys Diagnostic1MenuKeys[];
extern const Keys Diagnostic2MenuKeys[];
extern const Keys Diagnostic3MenuKeys[];
extern const Keys PIDMenuKeys[];
extern const Keys CellMenuKeys[];
extern const Keys SMSMenuKeys[];
extern const Keys ADV_InfoKeys[];
extern const Keys BattMenuKeys[];
extern const Keys AuxInfoMenuKeys[];
#ifdef EnableGPSRate
extern const Keys GPSRateMenuKeys[];
extern uint8_t label_Center[15];
extern uint8_t GPSRateMessages[][17];
#endif
extern const Keys FeaturesMenuKeys[];
extern const Keys TotalMenuKeys[];
extern const Keys TimedStrtStpKeys[];
extern const Keys BatchingKeys[];
extern const Keys EEPROMErrorKeys[];
extern const Keys Testing420Keys[];
extern const Keys AssemblyTestKeys[];
extern const Keys PropCtrlMenuKeys[];
extern const Keys RemoteMenuKeys[];

extern uint8_t MainBannerTimer;

extern uint8_t ChemRateFormat[6];
extern uint8_t SetRateFormat[6];

extern uint8_t killPivotStatus;

extern uint8_t AlarmStringNum;

void InitVolatileScreenVariables(void);
void ScreenDraw(uint8_t screen);
void updateChemFormat();
void UpdateChemRate(uint32_t num);
void updateCellDiag();
void updateMainBanner(uint8_t condition);

#endif

