/********************************************************************/
/*   Name: Screens                                                  */
/*------------------------------------------------------------------*/
/* Brief description: 	Screen Setup & Handling				        */
/*------------------------------------------------------------------*/
#include "screens.h"
#include "keys.h"
#include "core.h"
#include "global.h"
#include "lcd.h"
#include "utils.h"
#include "nvdata.h"
#include "Modem.h"
#include "BattBackup.h"
#include <string.h>

#ifdef EnableGPSRate
#include "RateByPosition.h"
#endif

const uint8_t BannerTxt[][17]={"-LOW TANK LEVEL-","-SCHD STRT&STP -","-  SCHD START  -","-  SCHD STOP   -","-REPLACE BATTERY"};

uint8_t AlarmMessages[][17] =
{
    "  - STOPPED  -  ",
    "   - STOP 1 -   ",
    "   - STOP 2 -   ",
    "   -        -   ",
    "- WTR PRES HI - ",
    "- WTR PRES LO - ",
    "- CHM PRES HI - ",
    "- CHM PRES LO - ",
    " -  USB ERR   - ",
    " -NO CHEM FLOW- ",
    " -OFF RATE STP- ",
    " -NO WATR FLOW- ",
    " -  NO POWER  - ",
    " - AUX INPUT  - ",
    " - BATCH DONE - ",    //can be to kill pivot or not
	" -MEMORY ERROR- "
};

uint8_t NonAlarmStopMessages[][17] =
{
	"- REMOTE STOP - ",
	"-CANCEL PUSHED- ",
	"-  BATCH DONE  -",
	"-PROPMODE STOP- ",
	"- SCHED STOP  - "

};

uint8_t displayDelay[6]="     ";

uint8_t pumpStatusText[][8] =
{
	"STOPPED",
    "RUNNING",
};
uint8_t connectMessages[][10] =
{
	"   NO PDP",		// Packet Data Protocol Connection (Could mean no cell connection or modem not activated)
	"SCKT CLSD",		// Have cell signal, socket connection is closed and not listening
	"  ERROR! ",		// Shouldn't ever be in this state, have socket connection but no PDP
	"SCKT OPEN",		// Socket is open (data should be transmitting)
	"  ERROR! ",		// Shouldn't ever be in this state, have socket listen but no PDP
	"SCKT LSTN",			// Socket listen (modem is listening for server request)
	" NO COMMS",			// Modem not responding to commands
#ifdef CheckSIM_T
	"RESETTING",			// Modem is initializing
	"   NO SIM"  // SIM card is not present or soemthing.
#else
	"RESETTING"			// Modem is initializing	
#endif
};


uint8_t sigStrBar[5][7] =
{
    "[    ]",
    "[*   ]",
    "[**  ]",
    "[*** ]",
    "[****]"
};

uint8_t UnitsString[][6] = {"ML/M ","OZ/AC","G/AC ","GPM  ","OZ/M ","GPH  "};
uint8_t BatchUnitsString[][3] = {"L ", "G ", "G ","G ","G ","G "};
uint8_t PulseUnitsString[][2] = {"L", "G", "G","G","G","G"};
uint8_t ChemRateFormatString[][6] = {"%5i  ", "%5i  ", "%5.1d", "%5.2c","%5.1d", "%5.1d"};
uint8_t SetChemRateFormatString[][6] = {"%5i  ", "%5i  ", "%5.1d","%5.1d","%5.1d","%5.1d"};
#ifdef RemoveValveOption
uint8_t LabelCtrl[][7] = {"   PWM","4-20mA"};
#else
uint8_t LabelCtrl[][7] = {"   PWM","4-20mA"," VALVE"};
#endif
uint8_t LabelEnable[][5] = {"DSBL","ENBL"};
uint8_t LabelBatch[][7] = {"CANCEL","RESET "};
uint8_t LabelDirection[][2] = {"F","R","S"," "};
uint8_t LabelMode[][5] = {"MAN ","AUTO"};
uint8_t ProportionalType[][6] = {"WATER","0-5V "};
const uint8_t WaterTotModeString[3][5] = {"DSBL","MARK","ALWY"};
uint8_t ModemModelStrings[][5]={"    "," SVG"," NAG","DUAL", " EUG","M1- ", " NF ", "UKNW"};
uint8_t Calib420Strings[][5]={"  0%","100%"," 50%"};
uint8_t SystemTypeString[][5]={"H-3G","H-1G","E-2G"};
uint8_t AssemblyTestStateString[][6]={"Start","Prime","  50%"," 100%","PausL","LowSp","PausM"," MSpd",
		"PausH","HiSpd","PausC","Catch","PausP","Purge"," Done"};
uint8_t AssemblyTestCmdString[][5]={"FAIL"," RES","  - "," ADV"};
uint8_t DataServiceString[][5]={"NONE","OKAY"};
uint8_t ModemErrLogString[][5]={"DSBL","LVL1","LVL2","  RX","  TX", "TXRX"};
uint8_t AuthStatusString[][11]={"UPLOAD ATT"," UPLOAD OK"," TKN RECVD","TOKEN FAIL","   REF TRY",
		"  REF FAIL"," LOGIN TRY","LOGIN FAIL","          ","LOGIN SAVD", " PROV WAIT", "PROVISIOND"};
uint8_t LabelPropMode[][5]={"DSBL","ENBL","4-20"};

Screen	ScreenVariables;
uint8_t AlarmStringNum;
uint8_t killPivotStatus;

uint8_t MainBanner[17]="  - STOPPED  -  ";// Message for Main Banner on front screen
bannerNotificationFlags mainBannerFlags;	//Keeps track of notifications that need displayed
uint8_t MainBannerTimer;

uint8_t ChemRateFormat[6];
uint8_t SetRateFormat[6];
 
void updateCellDiag(void);
void updateRateLabels(void);
void updateBatchLabel(void);
void updateMainBanner(uint8_t condition);
 
// Home Screen // 
uint8_t label_Chemical[15]="-- CHEMICAL --";
uint8_t label_Total[6]="TOTAL";
uint8_t label_PSI[4]="PSI";
const uint8_t label_WaterTitle[15]="--  WATER   --";
 
const Gadgets	MainMenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)&MainBanner, NULL, NULL, 0, 0}, 
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)&displayDelay, NULL, NULL, 4, 1},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Chemical, NULL, NULL, 1, 2},
     
    {VARIABLE, TYPE_UINT_16, (uint8_t *)ChemRateFormat, (uint8_t *)&ScreenVariables.ChemRate, NULL, NULL, 0, 3},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 5, 3},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.ChemPress, NULL, NULL, 10, 3},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 3},
     
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Total, NULL, NULL, 1, 4},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.1d", (uint8_t *)&NVdata.BatchTotalizer, NULL, NULL, 7, 4},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)BatchUnitsString, 0, 5, 15, 4},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)&ScreenVariables.BatchSPMainMenuLabel, NULL, NULL, 0, 5},
     
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_WaterTitle, NULL, NULL, 1, 6},
         
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%4i", (uint8_t *)&ScreenVariables.WaterRate, NULL, NULL, 0, 7},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterUnits, (uint8_t *)PulseUnitsString, 0, 1, 4, 7},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)"/MIN", NULL, NULL, 5, 7},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.WaterPress, NULL, NULL, 10, 7},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 7},
    
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};
 
const Keys MainMenuKeys[]=
{
	{KEY_END,    0,   0, NULL}
};

// Home Screen (again???) // 
const uint8_t label_RemoteRateTitle[17]="- REMOTE  RATE -";

const Gadgets	MainMenuBatchProportionalGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)&MainBanner, NULL, NULL, 0, 0}, 
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)&displayDelay, NULL, NULL, 4, 1},  
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Chemical, NULL, NULL, 1, 2},
    
    {VARIABLE, TYPE_UINT_16, (uint8_t *)ChemRateFormat, (uint8_t *)&ScreenVariables.ChemRate, NULL, NULL, 0, 3},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 5, 3},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.ChemPress, NULL, NULL, 10, 3},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 3},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Total, NULL, NULL, 1, 4},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.1d", (uint8_t *)&NVdata.BatchTotalizer, NULL, NULL, 7, 4},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)BatchUnitsString, 0, 5, 15, 4},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)&ScreenVariables.BatchSPMainMenuLabel, NULL, NULL, 0, 5},  
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_RemoteRateTitle, NULL, NULL, 0, 6},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)SetRateFormat, (uint8_t *)&NVdata.RateAc, 0, 65535, 6, 7},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 11, 7},
    
 
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

// Features /////////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_FeaturesMenuTitle[16]="   *FEATURES*  ";
const uint8_t label_PropCtrl[10]="PROP CTRL";
const uint8_t label_Batching[9]="BATCHING";
const uint8_t label_RemoteSpt[11]="REMOTE SPT";
const uint8_t label_Schedule[9]="SCHEDULE";
const uint8_t label_AuxInfo[9]="AUX INFO";
#ifdef EnableGPSRate
const uint8_t label_GPSRate[9]="GPS RATE";
#endif

const Gadgets	FeaturesMenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_FeaturesMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PropCtrl, NULL, NULL, 1, 1},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showPropCtrlScreen, (uint8_t *)LabelEnable, 0, 1, 12, 1},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Batching, NULL, NULL, 1, 2},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showBatchScreen, (uint8_t *)LabelEnable, 0, 1, 12, 2},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_RemoteSpt, NULL, NULL, 1, 3},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showRemoteScreen, (uint8_t *)LabelEnable, 0, 1, 12, 3},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Schedule, NULL, NULL, 1, 4},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showScheduleScreen, (uint8_t *)LabelEnable, 0, 1, 12, 4},

    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AuxInfo, NULL, NULL, 1, 5},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showAuxInfoScreen, (uint8_t *)LabelEnable, 0, 1, 12, 5},
    
#ifdef EnableGPSRate
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_GPSRate, NULL, NULL, 1, 6},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showGPSRateScreen, (uint8_t *)LabelEnable, 0, 1, 12, 6},
#endif

    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	FeaturesMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15, 15,(uint8_t *)&FeaturesMenuGadgets[2]}, // Proportional Control
    {KEY,        15, 15, (uint8_t *)&FeaturesMenuGadgets[4]}, // Batching
    {KEY,        15, 15, (uint8_t *)&FeaturesMenuGadgets[6]}, // Remote Setpoint
    {KEY,        15, 15, (uint8_t *)&FeaturesMenuGadgets[8]}, // Schedule
    {KEY,        15, 15, (uint8_t *)&FeaturesMenuGadgets[10]}, // Aux Info
#ifdef EnableGPSRate
    {KEY,        15, 15, (uint8_t *)&FeaturesMenuGadgets[12]}, // GPS Rate
#endif
    {KEY_END,    0,   0, NULL}
};

// Rate Set /////////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_RateMenuTitle[16]="   *RATE SET*  ";
const uint8_t label_Rate[5]="RATE";
const uint8_t label_Units[6]="UNITS";
const uint8_t label_TnkLvl[8]		="TNK LVL";
const uint8_t label_PwrUpRun[12]="PWR UP RUN ";

const Gadgets	RateMenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_RateMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Rate, NULL, NULL, 1, 1},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)SetRateFormat, (uint8_t *)&NVdata.RateAc, 0, 65535, 10, 1},
    
    // if UnitsStringNum line moves,
    // Search Keys.c for "if(mMotor.Enabled && CurrentRow == 2)",
    // it must be changed there too to prevent a units change while running
    // On "moveScreen" function, it also checks to see if MENURATE is being exited so it can perform L<->G conversions
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Units, NULL, NULL, 1, 2},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 11, 2},  
    																							  
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)&ScreenVariables.AdjRateLabel, NULL, NULL, 0, 4},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)&ScreenVariables.FlowTimeLabel, NULL, NULL, 0, 5},

    //!!!!!!!  If TankLvl moves position, search "keys.c" for g_tankPulseEdit
    //and put in the proper column and make sure on correct page
    //It allows the user to edit the var and store it to pulse count
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_TnkLvl, NULL, NULL, 1, 6},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.1d", (uint8_t *)&NVdata.ChemTankCalcLevel, 0, 65535, 9, 6},				 
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)BatchUnitsString, 0, 5, 15, 6},			
       
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PwrUpRun, NULL, NULL, 1, 7},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.BatchONStartup, (uint8_t *)LabelEnable, 0, 1, 12, 7},  
    
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	RateMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,         10, 14,(uint8_t *)&RateMenuGadgets[2]}, // RateAc
    {KEY,        15, 15, (uint8_t *)&RateMenuGadgets[4]}, // UnitsString
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY,        9, 14, (uint8_t *)&RateMenuGadgets[8]}, // Tank Level
    {KEY,        15, 15,(uint8_t *)&RateMenuGadgets[11]}, //Power Up
    {KEY_END,    0,   0, NULL}
};

// Totals screen //////////////////////////////////////////////////////////////////////////////////
const uint8_t label_TotalMenuTitle[16]="  *TOTALIZERS* ";
const uint8_t label_Product[15]="-- PRODUCT  --";
const uint8_t label_Reset[6]="RESET";
const uint8_t label_TotalMode[12]="TOTAL MODE ";

const Gadgets	TotalMenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_TotalMenuTitle, NULL, NULL, 0, 0},	
    
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Product, NULL, NULL, 1, 1},        
    
	{VARIABLE, TYPE_INT_32, (uint8_t *)"%b.1d", (uint8_t *)&NVdata.UserTotalizer, NULL, NULL, 3, 2},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)BatchUnitsString, 0, 5, 15, 2},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Reset, NULL, NULL, 1, 3},
	{VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.UserTotReset, (uint8_t *)LabelBatch, 0, 1, 10, 3},   
    
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_WaterTitle, NULL, NULL, 1, 4},     
    
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_TotalMode, NULL, NULL, 1, 5},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterTotalizerMode, (uint8_t *)WaterTotModeString, 0, 2, 12, 5},
    
    {VARIABLE, TYPE_INT_32, (uint8_t *)"%b.1d", (uint8_t *)&NVdata.WaterTotalizer, NULL, NULL, 3, 6},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterUnits, (uint8_t *)BatchUnitsString, 0, 1, 15, 6},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Reset, NULL, NULL, 1, 7},
	{VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.WaterTotReset, (uint8_t *)LabelBatch, 0, 1, 10, 7}, 
    
	{GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	TotalMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15, 15,(uint8_t *)&TotalMenuGadgets[5]}, //Prod total reset
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15, 15,(uint8_t *)&TotalMenuGadgets[8]}, //Prod total reset
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15, 15,(uint8_t *)&TotalMenuGadgets[12]}, //Water total reset
    {KEY_END,    0,   0, NULL}
};

// Prop Ctrl  ///////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_PropCtrlMenuTitle[17]="  *PROP CTRL*   ";
const uint8_t label_Water[6]="WATER";
const uint8_t label_PM[3]="PM";
const uint8_t label_SPT[4]="SPT";

const Gadgets	 PropCtrlMenuGadgets[]=
{
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PropCtrlMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Water, NULL, NULL, 1, 1},
	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterFlowEnabled, (uint8_t *)LabelEnable, 0, 1, 12, 1},
	
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_SPT, NULL, NULL, 2, 2},        
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.1d", (uint8_t *)&NVdata.NominalWater, 0, 65535, 7, 2},
	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterUnits, (uint8_t *)PulseUnitsString, 0, 1, 13, 2},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PM, NULL, NULL, 14, 2},
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PropCtrl, NULL, NULL, 1, 7},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showPropCtrlScreen, (uint8_t *)LabelEnable, 0, 1, 12, 7},
	
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys PropCtrlMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,        15,15, (uint8_t*)& PropCtrlMenuGadgets[2]}, // Water En
    {KEY,        7, 12, (uint8_t*)& PropCtrlMenuGadgets[4]}, // Water Stpt
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15, 15,(uint8_t*)&PropCtrlMenuGadgets[8]}, // Proportional Control
    {KEY_END,    0,   0, NULL}
};
// End of:  Prop Ctrl

// Batching /////////////////////////////////////////////////////////////////
const uint8_t label_BatchingMenuTitle[15]="   *BATCHING* ";
const uint8_t label_Batch[6]="BATCH";

const Gadgets	 BatchMenuGadgets[]=
{
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_BatchingMenuTitle, NULL, NULL, 0, 0}, 
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Batch, NULL, NULL, 1, 1},
	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.BatchEnabled, (uint8_t *)LabelEnable, 0, 1, 12, 1},
        
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_SPT, NULL, NULL, 1, 2},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.1d", (uint8_t *)&NVdata.SetBatch, 0, 65535, 8, 2},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)BatchUnitsString, 0, 5, 15, 2},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Total, NULL, NULL, 0, 3},    
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.1d", (uint8_t *)&NVdata.BatchTotalizer, NULL, NULL, 8, 3},
	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)BatchUnitsString, 0, 5, 15, 3},
	
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Reset, NULL, NULL, 1, 4},
	{VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.ChemTotReset, (uint8_t *)LabelBatch, 0, 1, 10, 4},
	
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Batching, NULL, NULL, 1, 7},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showBatchScreen, (uint8_t *)LabelEnable, 0, 1, 12, 7},
    
	{GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	BatchMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,        15,15, (uint8_t *)& BatchMenuGadgets[2]}, // Batch En
    {KEY,        8, 13, (uint8_t *)& BatchMenuGadgets[4]}, // Batch Stpt
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15, 15,(uint8_t *)&BatchMenuGadgets[10]}, //Batch total reset
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15, 15,(uint8_t *)&BatchMenuGadgets[12]}, // Batching Disable
    {KEY_END,    0,   0, NULL}
};

// Remote  ///////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_RemoteMenuTitle[17]="    *REMOTE*    ";
const uint8_t label_Max[4]="MAX";
const uint8_t label_Off[4]="OFF";
const uint8_t label_Min[4]="MIN";

const Gadgets	 RemoteMenuGadgets[]=
{
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_RemoteMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_RemoteSpt, NULL, NULL, 1, 1},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.PropModeEnabled, (uint8_t *)LabelPropMode, 0, 2, 12, 1},
    	
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Max, NULL, NULL, 1, 2},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)SetRateFormat, (uint8_t *)&NVdata.PropMaxRate, 0, 65535, 5, 2},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 11, 2},
    	
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Min, NULL, NULL, 1, 3},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)SetRateFormat, (uint8_t *)&NVdata.PropMinRate, 0, 65535, 5, 3},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 11, 3},
        
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Off, NULL, NULL, 1, 4},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)SetRateFormat, (uint8_t *)&NVdata.propModeStopRate, 0, 65535, 5, 4},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 11, 4},
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_RemoteSpt, NULL, NULL, 1, 7},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showRemoteScreen, (uint8_t *)LabelEnable, 0, 1, 12, 7},
	
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys RemoteMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,        15,15, (uint8_t *)&RemoteMenuGadgets[2]}, // Prop En
    {KEY,        5,  9, (uint8_t *)&RemoteMenuGadgets[4]}, // Prop Max Stpt
    {KEY,        5,  9, (uint8_t *)&RemoteMenuGadgets[7]}, // Prop Min Stpt
    {KEY,        5,  9, (uint8_t *)&RemoteMenuGadgets[10]},  //Remote Off	    
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY,        15,  15, (uint8_t *)&RemoteMenuGadgets[13]},  // Remote Disable	
    {KEY_END,    0,   0, NULL}
};
// End of:  Remote

// Schedule ///////////////////////////////////////////////////////////////////////////
const uint8_t label_ScheduleMenuTitle[17]="   *SCHEDULE*   ";
const uint8_t label_StartMode[11]="START MODE";
const uint8_t label_StopMode[10]="STOP MODE";
const uint8_t label_StartTime[11]="START TIME";
const uint8_t label_StopTime[10]="STOP TIME";
const uint8_t SchedModeString[3][5] = {"DSBL","ONCE"," RPT"};

const Gadgets ScheduleMenuGadgets[]=
{
	//!!!!!!!!!!!!!!!!!! If moving the position of start hour, min or stop hour,min  , 
	//then check keys.c for column# for "OK" button push, this resets Sch mode 
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_ScheduleMenuTitle, NULL, NULL, 0, 0},
   
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_StartMode, NULL, NULL, 1, 1},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.SchStartTime.Mode, (uint8_t *)SchedModeString, 0, 2, 12, 1},
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_StartTime, NULL, NULL, 1, 2},
    
#ifdef CodeChange_1_10_2022
    {VARIABLE, TYPE_HR_MIN, (uint8_t *)"%2", (uint8_t *)&NVdata.SchStartTime.Hour, 0, 23, 11, 2},
#else
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&NVdata.SchStartTime.Hour, 0, 23, 11, 2},
#endif
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)":", NULL, NULL, 13, 2},
#ifdef CodeChange_1_10_2022
    {VARIABLE, TYPE_HR_MIN, (uint8_t *)"%2", (uint8_t *)&NVdata.SchStartTime.Min, 0, 59, 14, 2},
#else
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&NVdata.SchStartTime.Min, 0, 59, 14, 2},
#endif
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_StopMode, NULL, NULL, 1, 3},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.SchStopTime.Mode, (uint8_t *)SchedModeString, 0, 2, 12, 3},
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_StopTime, NULL, NULL, 1, 4},
#ifdef CodeChange_1_10_2022
    {VARIABLE, TYPE_HR_MIN, (uint8_t *)"%2", (uint8_t *)&NVdata.SchStopTime.Hour, 0, 23, 11, 4},
#else
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&NVdata.SchStopTime.Hour, 0, 23, 11, 4},
#endif
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)":", NULL, NULL, 13, 4},
#ifdef CodeChange_1_10_2022
    {VARIABLE, TYPE_HR_MIN, (uint8_t *)"%2", (uint8_t *)&NVdata.SchStopTime.Min, 0, 59, 14, 4},
#else
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&NVdata.SchStopTime.Min, 0, 59, 14, 4},
#endif
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Schedule, NULL, NULL, 1, 7},        
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showScheduleScreen, (uint8_t *)LabelEnable, 0, 1, 12, 7},
   
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};
const Keys ScheduleMenuKeys[]=
{
	{KEY_NONE,    0,  0, NULL},
	{KEY,        15, 15, (uint8_t *)&ScheduleMenuGadgets[2]}, // Start Mode
	{KEY,        11, 15, (uint8_t *)&ScheduleMenuGadgets[6]}, // start minute
	{KEY,        15, 15, (uint8_t *)&ScheduleMenuGadgets[8]}, // Stop Mode
	{KEY,        11, 15, (uint8_t *)&ScheduleMenuGadgets[12]}, // Stop minute
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY,        15, 15, (uint8_t *)&ScheduleMenuGadgets[14]}, // Schedule Disable
    {KEY_END,    0,   0, NULL},
    {KEY,        11, 15, (uint8_t *)&ScheduleMenuGadgets[4]}, // start hour
    {KEY,        11, 15, (uint8_t *)&ScheduleMenuGadgets[10]} // stop hour
};
// End of: Schedule

// Aux Info /////////////////////////////////////////////////////////
const uint8_t label_AuxMenuTitle[17]="   *AUX INFO*   ";
const uint8_t label_AuxDigInput[15]	="AUX DIG INPUT:";
const uint8_t label_AuxDIInv[12]	="AUX DI INV:";
const uint8_t label_AuxPress[11]	="AUX PRESS:";
const uint8_t label_AuxPCal[11]		="AUX P CAL:";

const Gadgets AuxInfoMenuGadgets[]=
{
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AuxMenuTitle, NULL, NULL, 0, 0},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AuxDigInput, NULL, NULL, 0, 1},
   {VARIABLE, TYPE_UINT_8, (uint8_t *)"%1i", (uint8_t *)&ScreenVariables.AuxDigIn, 0, 1, 15, 1},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AuxDIInv, NULL, NULL, 1, 2},
   {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.InvAuxDI, (uint8_t *)LabelEnable, 0, 1, 12, 2},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AuxPress, NULL, NULL, 0, 3},
   {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&ScreenVariables.AuxPress, 0, 65535, 11, 3},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AuxPCal, NULL, NULL, 1, 4},
   {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&NVdata.AuxPressCal, 1, 65535, 11, 4},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AuxInfo, NULL, NULL, 1, 7},        
   {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showAuxInfoScreen, (uint8_t *)LabelEnable, 0, 1, 12, 7},
   
   // Termination
   {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	AuxInfoMenuKeys[]=
{
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY,        15, 15, (uint8_t *)&AuxInfoMenuGadgets[4]}, // Inv Aux DI
	{KEY_NONE,    0,  0, NULL},
	{KEY,        11, 15, (uint8_t *)&AuxInfoMenuGadgets[8]}, // Aux Press Cal	
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY,        15, 15, (uint8_t *)&AuxInfoMenuGadgets[10]}, // Aux disable	
	{KEY_END,    0,   0, NULL}
};
// End of: Aux Info

#ifdef EnableGPSRate
// GPS Rate /////////////////////////////////////////////////////////
const uint8_t label_GPSRateMenuTitle[17]="   *GPS RATE*   ";
uint8_t label_Center[15]="FIELD ??? BASE";
const uint8_t label_CurrentSlice[6]="SLICE";
//const uint8_t label_SliceRate[7]="SIGNAL";
const uint8_t label_Remote[8] = "REMOTE ";
uint8_t GPSRateMessages[][17] =
{
	"STATUS IS GOOD  ", // 0: No Error
	"GPS TIMED OUT   ", // 1: GPS GetData Timed Out
	"REMOTE ERROR 2  ", // 2: TBD
	"REMOTE LINK MIA ", // 3: Did not receive communication from RemoteUnit
	"RADIO LINK ERROR", // 4: LoRa CRC Fail
	"RADIO MSG ERROR ", // 5: MyCRC Fail
	"INVALID GPS CALC", // 6: Slice computed by CalculateBearing was outside expectations
	"WAITING FOR LINK", // 7: Waiting for first message from RemoteUnit   
	"REMOTE BATT LOW ", // 8: Remote unit has low battery.
	//"                ", // 9:
	//"                ", // 10:
	//"                ", // 11:
	//"                ", // 12:
	//"                ", // 13:
	"FAILED BY ERROR ", // 14:
	"NOT CONFIGURED  "  // 15: Need to get new information from rateData.csv

};

const Gadgets GPSRateMenuGadgets[]=
{
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_GPSRateMenuTitle, NULL, NULL, 0, 0},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Center, NULL, NULL, 0, 1},
   
   //{VARIABLE, TYPE_SINGLESTRING, NULL, ScreenVariables.CenterLatDegreeString, NULL, NULL, 3, 2},
   {VARIABLE, TYPE_INT_32, (uint8_t *)"%b.6M", (uint8_t *)&ScreenVariables.CenterLatDegree, NULL, NULL, 3, 2},
   
   //{VARIABLE, TYPE_SINGLESTRING, NULL, ScreenVariables.CenterLonDegreeString, NULL, NULL, 3, 3},
   {VARIABLE, TYPE_INT_32, (uint8_t *)"%b.6M", (uint8_t *)&ScreenVariables.CenterLonDegree, NULL, NULL, 3, 3},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Remote, NULL, NULL, 0, 4},
   //{VARIABLE, TYPE_UINT_8, (uint8_t *)"%2", (uint8_t *)&RemoteFieldData.RadioID, 0, 15, 7, 4},
   {VARIABLE, TYPE_SINGLESTRING, NULL, (uint8_t *)&RemoteFieldData.RadioID, 0, 15, 7, 4},
   //{VARIABLE, TYPE_UINT_8, (uint8_t *)"%2", (uint8_t *)&RemoteFieldData.errorCode, 0, 15, 3, 4},   
   
   {VARIABLE, TYPE_HR_MIN, (uint8_t *)"%2", (uint8_t *)&RemoteFieldData.hour, 0, 15, 0, 5},
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)":", NULL, NULL, 2, 5},
   {VARIABLE, TYPE_HR_MIN, (uint8_t *)"%2", (uint8_t *)&RemoteFieldData.minute, 0, 15, 3, 5},
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_CurrentSlice, NULL, NULL, 7, 5},
   {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3", (uint8_t *)&FieldSlices.CurrentSlice, 1, 360, 13, 5},
   
   //{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_SliceRate, NULL, NULL, 0, 5},
   

   
   {VARIABLE, TYPE_STRING, (uint8_t*)&RateByPositionControl.GPSRateMessageSelect, (uint8_t *)GPSRateMessages, 0, 15, 0, 6},
   
   {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_GPSRate, NULL, NULL, 1, 7},        
   {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.showGPSRateScreen, (uint8_t *)LabelEnable, 0, 1, 12, 7},
   
   // Termination
   {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	GPSRateMenuKeys[]=
{
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY,        15, 15, (uint8_t *)&GPSRateMenuGadgets[13]}, // GPS Rate Enable/Disable
	{KEY_END,    0,   0, NULL}
};
// End of: GPS Rate
#endif

// Advanced: Alarm ///////////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_AlarmMenuTitle[12]	="ADV - ALARM"; 
const uint8_t label_NotAtRate[16]		="NOT AT RATE ALM";
const uint8_t label_Deadbnd[8]			="DEADBND";
const uint8_t label_Percent[2]			="%";
const uint8_t label_Time[5]				="TIME";
const uint8_t label_NoFlowTime[14]		="NO FLOW ALARM";
const uint8_t label_Sec[2]				="s";
const uint8_t label_TnkAlm[8]		="TNK ALM";


const Gadgets	AlarmMenuGadgets[] = {
	// Line 0
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AlarmMenuTitle, NULL, NULL, 0, 0},
	// Line 1
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_TnkAlm, NULL, NULL, 1, 1},
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.1d", (uint8_t *)&NVdata.ChemTankCalcLevelAlarm , 0, 65535, 9, 1},  
	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)BatchUnitsString, 0, 5, 15, 1},
	// Line 2
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_NotAtRate, NULL, NULL, 0, 2},
    // Line 3    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Deadbnd, NULL, NULL, 1, 3},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5.1d", (uint8_t *)&NVdata.RateAlmDeadband, 150, 1000, 10, 3},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Percent, NULL, NULL, 15, 3},
    // Line 4
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Time, NULL, NULL, 1, 4},        
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&NVdata.NotAtRateTimerSetting, 10, 65535, 10, 4},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 15, 4},
	// Line 5
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_NoFlowTime, NULL, NULL, 0, 5},
	// Line 6
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Time, NULL, NULL, 1, 6},
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&NVdata.NoFlowTimerSetting, 10, 65535, 10, 6},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 15, 6},
	// Line 7
	// Termination
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	AlarmMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,        9, 14, (uint8_t *)&AlarmMenuGadgets[2]}, // Tank Alarm 
    {KEY_NONE,    0,  0, NULL},
    {KEY,        10, 14, (uint8_t *)&AlarmMenuGadgets[6]}, // Deadband
    {KEY,        10, 14, (uint8_t *)&AlarmMenuGadgets[9]}, // Alarm Time 
    {KEY_NONE,    0,  0, NULL}, //
    {KEY,        10, 14, (uint8_t *)&AlarmMenuGadgets[13]},// No Flow Time 
    {KEY_NONE,    0,  0, NULL}, 
    {KEY_END,    0,   0, NULL}
};
// End of: Advanced: Alarm

// Advanced: Setup Chem ///////////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_AdvChmMenuTitle[16]	="ADV - SETUP CHM";
const uint8_t label_PresHi[8]			="PRES HI";
const uint8_t label_HiDly[7]			="HI DLY";
const uint8_t label_PresLo[8]			="PRES LO";
const uint8_t label_LoDly[7]			="LO DLY";
const uint8_t label_PulsesPer[8]		="PULSES/";
const uint8_t label_PresCal[9]			="PRES CAL";
const uint8_t label_Control[8]			="CONTROL";

const Gadgets	ChemicalMenuGadgets[]=
{
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvChmMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PresHi, NULL, NULL, 1, 1},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&NVdata.Chem_PresHi, 1, 250, 10, 1},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 1},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_HiDly, NULL, NULL, 1, 2},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.ChemHiPressDly, 0, 255, 10, 2},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 13, 2},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PresLo, NULL, NULL, 1, 3},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&NVdata.Chem_PresLo, 0, 250, 10, 3},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 3},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_LoDly, NULL, NULL, 1, 4},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.ChemLoPressDly, 0, 255, 10, 4},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 13, 4},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PulsesPer, NULL, NULL, 1, 5},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)PulseUnitsString, 0, 5, 8, 5},
#ifdef ChemCalNumber_Bigger
    {VARIABLE, TYPE_UINT_32, (uint8_t *)"%6i", (uint8_t *)&NVdata.Chem_Pulses, 1, 999999, 10, 5},	
#else
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6i", (uint8_t *)&NVdata.Chem_Pulses, 1, 65535, 10, 5},
#endif
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PresCal, NULL, NULL, 1, 6},    
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&NVdata.Chem_PsiCal, 1, 999, 10, 6},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 6},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Control, NULL, NULL, 1, 7},
#ifdef RemoveValveOption
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.ControlStringNum, (uint8_t *)LabelCtrl, 0, 1, 10, 7},
#else
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.ControlStringNum, (uint8_t *)LabelCtrl, 0, 2, 10, 7},
#endif

    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	ChemicalMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,        10, 12, (uint8_t *)&ChemicalMenuGadgets[2]},  // Chem_PresHi
    {KEY,        10, 12, (uint8_t *)&ChemicalMenuGadgets[5]},	//Chem Hi Delay
    {KEY,        10, 12, (uint8_t *)&ChemicalMenuGadgets[8]}, // Chem_PresLo  //was 11,12
    {KEY,        10, 12, (uint8_t *)&ChemicalMenuGadgets[11]},	//Chem Lo Delay
    {KEY,        11, 15, (uint8_t *)&ChemicalMenuGadgets[15]}, // Chem_Pulses
    {KEY,        10, 12, (uint8_t *)&ChemicalMenuGadgets[17]}, // Chem PSI Pulses
    {KEY,        15, 15, (uint8_t *)&ChemicalMenuGadgets[20]}, // Chem_Control

    {KEY_END,    0,   0, NULL}
};

// Advanced: Setup Water ////////////////////////////////////////////////////////////////////////////
const uint8_t label_AdvWtrMenuTitle[16]	="ADV - SETUP WTR";

const Gadgets	WaterMenuGadgets[]=
{
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvWtrMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PresHi, NULL, NULL, 1, 1},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&NVdata.Wtr_PresHi, 1, 999, 10, 1},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 1},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_HiDly, NULL, NULL, 1, 2},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.WaterHiPressDly, 0, 255, 10, 2},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 13, 2},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PresLo, NULL, NULL, 1, 3},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&NVdata.Wtr_PresLo, 0, 999, 10, 3},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 3},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_LoDly, NULL, NULL, 1, 4},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.WaterLoPressDly, 0, 255, 10, 4},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 13, 4},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Units, NULL, NULL, 1, 5},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterUnits, (uint8_t *)PulseUnitsString, 0, 1, 15, 5},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PulsesPer, NULL, NULL, 1, 6},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterUnits, (uint8_t *)PulseUnitsString, 0, 1, 8, 6},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.2c", (uint8_t *)&NVdata.Wtr_Pulses, 1, 65535, 10, 6},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PresCal, NULL, NULL, 1, 7},    
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&NVdata.Wtr_PsiCal, 1, 999, 10, 7},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 7},

    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	WaterMenuKeys[]=
{
	{KEY_NONE,	0,  0,NULL},
  	{KEY,		10,12,(uint8_t *)&WaterMenuGadgets[2]},	// Wtr_PresHi
  	{KEY,       10, 12, (uint8_t *)&WaterMenuGadgets[5]},	//Water Hi Delay
  	{KEY,		10,12,(uint8_t *)&WaterMenuGadgets[8]},	// Wtr_PresLo
  	{KEY,       10, 12, (uint8_t *)&WaterMenuGadgets[11]},	//Water Lo Delay
  	{KEY,       15, 15, (uint8_t *)&WaterMenuGadgets[14]},	//Water Units
  	{KEY,		10,15,(uint8_t *)&WaterMenuGadgets[17]},	// Wtr_Pulses 
  	{KEY,		10,12,(uint8_t *)&WaterMenuGadgets[19]},// Wtr_PsiCal
    {KEY_END,    0,   0, NULL}
};

// Advanced: Settings ///////////////////////////////////////////////////////////////////////
const uint8_t label_S2MenuTitle[15]="ADV - SETTINGS";
const uint8_t label_FieldNum[8]="FIELD #";
const uint8_t label_LogInt[8]="LOG INT";
const uint8_t label_WtrInt[8]="WTR INT";
const uint8_t label_StartDly[10]="START DLY";
const uint8_t label_IrrigKill[11]="IRRIG STOP";
const uint8_t label_IKDly[8]="STP DLY";

const Gadgets Setting2MenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_S2MenuTitle, NULL, NULL, 0, 0},
   
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_IrrigKill, NULL, NULL, 1, 1},
	{VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.BatchedPivotKillEnable, (uint8_t *)LabelEnable, 0, 1, 12, 1},
		
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_IKDly, NULL, NULL, 1, 2},    
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&NVdata.BatchedPivotKillDelay, 0, 65535, 8, 2},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Min, NULL, NULL, 13, 2}, 
	
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_FieldNum, NULL, NULL, 1, 3},
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&NVdata.fieldID, 1, 65535, 11, 3},
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_LogInt, NULL, NULL, 1, 4},         
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.Interval, 0, 250, 10, 4},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Min, NULL, NULL, 13, 4},
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_WtrInt, NULL, NULL, 1, 5},         
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.WaterLogInt, 0, 250, 10, 5},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Min, NULL, NULL, 13, 5},
   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_StartDly, NULL, NULL, 1, 6},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&NVdata.startDelayTime, 0, 99, 11, 6},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Min, NULL, NULL, 13, 6},   
   
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys SettingMenuKeys[]=
{
	{KEY_NONE,    0,  0, NULL},
    {KEY,        15, 15, (uint8_t *)&Setting2MenuGadgets[2]},  // Irrigation Kill
    {KEY,        8, 12, (uint8_t *)&Setting2MenuGadgets[4]},  // Irrigation Kill Delay
    {KEY,        11, 15, (uint8_t *)&Setting2MenuGadgets[7]},  // Field Number
    {KEY,        11, 12, (uint8_t *)&Setting2MenuGadgets[9]}, // Log Int  
    {KEY,        11, 12, (uint8_t *)&Setting2MenuGadgets[12]}, // Water Log Int 
    {KEY,        11, 12, (uint8_t *)&Setting2MenuGadgets[15]}, // Start Delay
    {KEY_NONE,    0,  0, NULL},
    {KEY_END,     0,  0, NULL}
};
// End of: Advanced: Settings 

// Advanced: Date Time ////////////////////////////////////////////////////////////////////////////// 
const uint8_t label_AdvClkMenuTitle[16]	="ADV - DATE TIME";
const uint8_t label_Month[6]			="MONTH";
const uint8_t label_Day[4]				="DAY";
const uint8_t label_Year[5]				="YEAR";
const uint8_t label_timeUpdate[11]      ="TIME UPDTE";
const uint8_t label_Hour[5]="HOUR";
  
const Gadgets	ClockMenuGadgets[]=
{                                                      
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvClkMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Hour, NULL, NULL, 1, 1},
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&ScreenVariables.setHour, 0, 23, 14, 1},
      
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Min, NULL, NULL, 1, 2},
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&ScreenVariables.setMin, 0, 59, 14, 2},
      
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Month, NULL, NULL, 1, 4},
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&ScreenVariables.setMonth, 1, 12, 14, 4},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Day, NULL, NULL, 1, 5},
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&ScreenVariables.setDay, 1, 31, 14, 5},
      
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Year, NULL, NULL, 1, 6},
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&ScreenVariables.setYear, 0, 99, 14, 6},  
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_timeUpdate, NULL, NULL, 1, 7},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.timeDateAdjust, (uint8_t *)LabelMode, 0, 1, 12, 7},
    
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys	ClockMenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,        14, 15, (uint8_t *)&ClockMenuGadgets[2]},  // Hr
    {KEY,        14, 15, (uint8_t *)&ClockMenuGadgets[4]},  // Min
    {KEY_NONE,    0,  0, NULL},
    {KEY,        14, 15, (uint8_t *)&ClockMenuGadgets[6]},  // Month
    {KEY,        14, 15, (uint8_t *)&ClockMenuGadgets[8]},  // Day
    {KEY,        14, 15, (uint8_t *)&ClockMenuGadgets[10]}, // Year
    {KEY,        14, 15, (uint8_t *)&ClockMenuGadgets[12]}, // Manual / Auto
    {KEY_END,     0,  0, NULL}
};
 
// Advanced: Diagnostic 1 /////////////////////////////////////////////////////////////////////////// 
const uint8_t label_AdvD1MenuTitle[17]	="ADV - DIAGNOST 1";
const uint8_t label_ChmPr[7]			="CHM PR";
const uint8_t label_Chm[4]				="CHM";
const uint8_t label_Hz[8]				="CHEM Hz";
const uint8_t label_Motor[6] = "MOTOR";
const uint8_t label_SP[4]				="SP=";
const uint8_t label_PV[4]				="PV=";

const Gadgets	Diagnostic1MenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvD1MenuTitle, NULL, NULL, 0, 0},
   
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_ChmPr, NULL, NULL, 0, 1},
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.ChemPress, NULL, NULL, 9, 1},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 1},
   	 
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Chm, NULL, NULL, 0, 2},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)ChemRateFormat, (uint8_t *)&ScreenVariables.ChemRate, NULL, NULL, 5, 2},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 11, 2},
	
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Hz, NULL, NULL, 0, 3},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&ScreenVariables.ChemFreq, NULL, NULL, 11, 3},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Motor, NULL, NULL, 0, 4},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5.2c", (uint8_t *)&ScreenVariables.MotorSpeed, NULL, NULL, 8, 4},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Percent, NULL, NULL, 13, 4},
    {VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.Direction, (uint8_t *)LabelDirection, 0, 3, 15, 4},
	   
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_SP, NULL, NULL, 0, 7},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&ScreenVariables.TargetRate, 0, 65535, 3, 7},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PV, NULL, NULL, 9, 7},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&ScreenVariables.ProcessVariable, NULL, NULL, 11, 7},
   	
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys    Diagnostic1MenuKeys[]=
{
   {KEY_NONE,    0,  0, NULL},
   {KEY_END,     0,  0, NULL}
};
// End of: Advanced: Diagnostic 1

// Advanced: Diagnostic 2 /////////////////////////////////////////////////////////////////////
uint8_t USBStringNum;
uint8_t USBMessage;
const uint8_t label_AdvD2MenuTitle[17]="ADV - DIAGNOST 2";
const uint8_t label_Mode[5]			="MODE";
const uint8_t label_ManualPerc[8]		="MANUAL%";
const uint8_t label_LastStop[12]		="LAST STOP #";
const uint8_t label_Ver[5]				="VER:";
const uint8_t label_ID[4]				="ID:";
const uint8_t label_Period[2]			=".";
  
const Gadgets Diagnostic2MenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvD2MenuTitle, NULL, NULL, 0, 0},
     
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Mode, NULL, NULL, 1, 1},
 	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.Auto, (uint8_t *)LabelMode, 0, 1, 11, 1},
 	
 	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_ManualPerc, NULL, NULL, 1, 2},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.2c", (uint8_t *)&NVdata.ManualPercent, 0, 10000, 9, 2},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Percent, NULL, NULL, 15, 2},
     
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_LastStop, NULL, NULL, 0, 3},
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.StopNum, NULL, NULL, 13, 3},
     
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Ver, NULL, NULL, 0, 4},
    {VARIABLE, TYPE_STRING, NULL, (uint8_t *)VERSION, NULL, NULL, 5, 4},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_ID, NULL, NULL, 0, 5},
    {VARIABLE, TYPE_STRING, NULL, (uint8_t *)NVdata.ModemPN, NULL, NULL, 6, 5},
    
	{VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&ip[0], 0, 255, 0, 6},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Period, NULL, NULL, 3, 6},
	{VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&ip[1], 0, 255, 4, 6},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Period, NULL, NULL, 7, 6},
	{VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&ip[2], 0, 255, 8, 6},
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Period, NULL, NULL, 11, 6},
	{VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&ip[3], 0, 255, 12, 6},
     
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Rate, NULL, NULL, 0, 7},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)ChemRateFormat, (uint8_t *)&ScreenVariables.ChemRate, NULL, NULL, 5, 7},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)UnitsString, 0, 5, 11, 7},
    
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys Diagnostic2MenuKeys[]=
{
	{KEY_NONE,    0,  0, NULL},
    {KEY,        15, 15,(uint8_t *)&Diagnostic2MenuGadgets[2]}, // Manual Auto
    {KEY,         9, 14,(uint8_t *)&Diagnostic2MenuGadgets[4]}, // Manual Percent
    {KEY_END,     0,  0, NULL}
};
// End of: Advanced: Diagnostic 2
 
// Advanced: Diagnostic 3 //////////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_AdvD3MenuTitle[17]	="ADV - DIAGNOST 3";
const uint8_t label_WaterPressure[9] = "WATER PR";
const uint8_t label_WaterFrequency[9] = "WATER Hz";
const uint8_t label_WtrPls[8]			="WTR PLS";
 
const Gadgets	Diagnostic3MenuGadgets[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvD3MenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_WaterPressure, NULL, NULL, 0, 1},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.WaterPress, NULL, NULL, 9, 1},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 1},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_WaterFrequency, NULL, NULL, 0, 2},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&ScreenVariables.WaterFreq, NULL, NULL, 11, 2},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Water, NULL, NULL, 0, 3},
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%4i", (uint8_t *)&ScreenVariables.WaterRate, NULL, NULL, 6, 3},
    {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.WaterUnits, (uint8_t *)PulseUnitsString, 0, 1, 11, 3},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)"/MIN", NULL, NULL, 12, 3},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_WtrPls, NULL, NULL, 0, 4},
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.WtrPulsesIn, NULL, NULL, 13, 4},
            
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys Diagnostic3MenuKeys[]=
{
    {KEY_NONE,    0,  0, NULL},
    {KEY_END,     0,  0, NULL}
};
// End of: Advanced: Diagnostic 3

// Advanced: PID tuning menu
const uint8_t label_AdvPIDMenuTitle[16]	="ADV - PID SETUP";
const uint8_t label_Samples[8]			="SAMPLES";
const uint8_t label_Interval[9]			="INTERVAL";
const uint8_t label_OutGain[9]			="OUT GAIN";
const uint8_t label_MinPWM[8]			="MIN PWM";
const uint8_t label_MaxPWM[8]			="MAX PWM";
const uint8_t label_Softstart[10]		="SOFTSTART";

const Gadgets PIDMenuGadgets[] =
{
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvPIDMenuTitle, NULL, NULL, 0, 0},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Samples, NULL, NULL, 1, 1},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.FiltTaps, 1, 100, 13, 1},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Interval, NULL, NULL, 1, 2},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%4.1d", (uint8_t *)&NVdata.CtrlInterval, 1, 100, 12, 2},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_OutGain, NULL, NULL, 1, 3},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&NVdata.CtrlGain, 1, 250, 13, 3},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Deadbnd, NULL, NULL, 1, 4},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5.1d", (uint8_t *)&NVdata.CtrlDeadBand, 1, 2000, 10, 4},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Percent, NULL, NULL, 15, 4},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_MinPWM, NULL, NULL, 1, 5},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.2c", (uint8_t *)&NVdata.MinPWMSpeed, 0, 10000, 9, 5},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Percent, NULL, NULL, 15, 5},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_MaxPWM, NULL, NULL, 1, 6},        
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%6.2c", (uint8_t *)&NVdata.MaxPWMSpeed, 0, 10000, 9, 6},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Percent, NULL, NULL, 15, 6},
    
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Softstart, NULL, NULL, 1, 7},        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%4.1d", (uint8_t *)&NVdata.softStartTime, 0, 250, 11, 7},
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 15, 7},
   
    {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys PIDMenuKeys[] =
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,         13,15, (uint8_t *)&PIDMenuGadgets[2]},
    {KEY,         13,15, (uint8_t *)&PIDMenuGadgets[4]},
    {KEY,         13,15, (uint8_t *)&PIDMenuGadgets[6]},
    {KEY,         10,14, (uint8_t *)&PIDMenuGadgets[8]},
    {KEY,          9,14, (uint8_t *)&PIDMenuGadgets[11]},
    {KEY,          9,14, (uint8_t *)&PIDMenuGadgets[14]},
    {KEY,          12,14, (uint8_t *)&PIDMenuGadgets[17]},

    {KEY_END,      0, 0, NULL}
};
// End of: Advanced: PID

// Advanced: Cell Diagnostics 
const uint8_t label_AdvCellMenuTitle[16]="ADV - CELL DIAG";
const uint8_t label_CellSig[17]			="CELLSIG: -   dBm";
const uint8_t label_Dataconn[10]		="DATACONN:";
const uint8_t label_Conn[6]				="CONN:";
const uint8_t label_Modem[7]			="MODEM:";
const uint8_t label_ErrLog[9]			="ERR LOG:";
const uint8_t label_Auth[5]				="AUTH";

const Gadgets	Cell_Diagnostics[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvCellMenuTitle, NULL, NULL, 0, 0},
	
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Modem, NULL, NULL, 1, 1},
	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.modemEnable, (uint8_t *)LabelEnable, 0, 1, 12, 1},
	
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_CellSig, NULL, NULL, 0, 2},        
	{VARIABLE, TYPE_UINT_8, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.SigStrength , 0, 113, 10, 2},
	 
	{VARIABLE, TYPE_STRING, (uint8_t*)&ScreenVariables.SigStrBar, (uint8_t *)sigStrBar, 0, 99, 10, 3},
	 
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Dataconn, NULL, NULL, 0, 4}, 
	{VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.DataService, (uint8_t *)DataServiceString, 0, 1, 12, 4},
	 
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Conn, NULL, NULL, 0, 5}, 
	{VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.connStatus, (uint8_t *)connectMessages, 0, 3, 7, 5},
	 
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Auth, NULL, NULL, 0, 6},
	{VARIABLE, TYPE_STRING, (uint8_t *)&m1.authStatus, (uint8_t *)&AuthStatusString, 0, 8, 6, 6},
	
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_ErrLog, NULL, NULL, 1, 7},
	{VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.modemErrLog, (uint8_t *)ModemErrLogString, 0, 5, 12, 7},
	 
	{GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys CellMenuKeys[] =
{
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15,15, (uint8_t *)&Cell_Diagnostics[2]},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY_NONE,    0,  0, NULL},
    {KEY,         15,15, (uint8_t *)&Cell_Diagnostics[13]},
    {KEY_END,    0,   0, NULL}
};
// End of: Advanced: Cell Diagnostics 

// Advanced: Cell Diagnostics 2
const uint8_t label_AdvCell2MenuTitle[17]="ADV - CELL DIAG2";
const uint8_t label_CCID[6]				="CCID:";
const uint8_t label_PROV[7]				="PROV!!";
const uint8_t label_CellChip[11]		="CELL CHIP:";
const uint8_t label_AccessType[9]		="CARRIER:";
const uint8_t label_Stat[6]				="STAT:";
const uint8_t carrierLabel[][4]={"UNK","VZW","ATT","ROG","TLS"};
uint8_t LabelProv[][3] = {"  ","GO"};

const Gadgets  Cell_Diagnostics2[]=
{
		{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvCell2MenuTitle, NULL, NULL, 0, 0},
		
		{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_CCID, NULL, NULL, 0, 1},
		{VARIABLE, TYPE_STRING, NULL, (uint8_t *)ScreenVariables.ICCID[0], NULL, NULL, 6, 1},
		
		{VARIABLE, TYPE_STRING, NULL, (uint8_t *)ScreenVariables.ICCID[1], NULL, NULL, 6, 2},
		
		{VARIABLE, TYPE_STRING, NULL, (uint8_t *)&ScreenVariables.APN, NULL, NULL, 0, 3},
		
		{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PROV, NULL, NULL, 1, 4},
		{VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.ProvRequest, (uint8_t *)&LabelProv, 0, 1, 14, 4},	//Find ProvRequest in keys.c if moved, currently looks 
																												//for row 4	
#ifdef DRFLASHTEST
		 {LABEL, TYPE_LABEL, NULL, (uint8_t *)"ORDER 66:", NULL, NULL, 1, 4},        
		 {VARIABLE, TYPE_INT_8, (uint8_t *)"%3", (uint8_t *)&ScreenVariables.DRTEST , -1, 127, 13, 4},
#endif
		
		{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_CellChip, NULL, NULL, 0, 5},  
		{VARIABLE, TYPE_UINT_8, (uint8_t *)"%1i", (uint8_t *)&m1.fwSwitchState, 0, 1, 15, 5},			//print fwswitch state first, can be overwritten if not used.		
		{VARIABLE, TYPE_STRING, (uint8_t *)&m1.modemModel, (uint8_t *)&ModemModelStrings, 0, 4, 12, 5},
		
		{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AccessType, NULL, NULL, 0, 6},
		{VARIABLE, TYPE_STRING, (uint8_t *)&m1.carrierType, (uint8_t *)&carrierLabel, 0, 4, 10, 6},
		{VARIABLE, TYPE_UINT_8, (uint8_t *)"%2i", (uint8_t *)&m1.accessTechnology, 0, 99, 14, 6},
		
		{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Stat, NULL, NULL, 0, 7},
		{VARIABLE, TYPE_STRING, (uint8_t *)&m1.authStatus, (uint8_t *)&AuthStatusString, 0, 8, 6, 7},
			
		{GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
		
};

const Keys SMSMenuKeys[] =
{
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY,		  15, 15, (uint8_t *)&Cell_Diagnostics2[6]}, 
#ifdef DRFLASHTEST
	{KEY,		  13, 15, (uint8_t *)&Cell_Diagnostics2[6]},  //TODO: DR TEST
#endif
	{KEY_END,    0,   0, NULL}
};
// End of: Advanced: Cell Diagnostics 2

// Advanced: Info  //////////////////////////////////////////////////////////////////////////
const uint8_t label_AdvInfoMenuTitle[11]="ADV - INFO";
const uint8_t label_USB[4]				="USB";
const uint8_t label_CPURst[8]			="CPU RST";
const uint8_t label_Restarts[9]		="RESTARTS";
const uint8_t label_AlmNum[8]			="ALM NUM";
const uint8_t label_AlmEnbl[9]			="ALM ENBL";

const Gadgets  ADV_Info[]=
{
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvInfoMenuTitle, NULL, NULL, 0, 0},
		
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_CPURst, NULL, NULL, 0, 1},                        
    {VARIABLE, TYPE_UINT_8, (uint8_t *)"%5i", (uint8_t*)&ScreenVariables.COPReset, 1, 65535, 11, 1}, 
     
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Restarts, NULL, NULL, 0, 2},                       
    {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t*)&NVdata.Restarts, 1, 65535, 11, 2},	
		
    {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AlmEnbl, NULL, NULL, 1, 3},        
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t*)&NVdata.EnabledAlarmMask, 0, 65535, 11, 3},

	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AlmNum, NULL, NULL, 1, 4},        
	{VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t*)&ScreenVariables.AlarmMask, 0, 65535, 11, 4},
		
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)label_USB, NULL, NULL, 0, 5},
	{VARIABLE, TYPE_STRING, (uint8_t *)&USBStringNum, (uint8_t *)ScreenVariables.USBString, 0, 0, 5, 5},
	
#ifdef DRFLASHTEST
	{LABEL, TYPE_LABEL, NULL, (uint8_t *)"ORDER 66:", NULL, NULL, 1, 4},        
	{VARIABLE, TYPE_INT_8, (uint8_t *)"%3", (uint8_t *)&ScreenVariables.DRTEST , -1, 127, 13, 4},
#endif
		    
	{GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys ADV_InfoKeys[] =
{
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY,		  6, 15, (uint8_t *)&ADV_Info[6]}, // Alarm Number
	{KEY,		  6, 15, (uint8_t *)&ADV_Info[8]}, // Alarm Enable
	{KEY_NONE,    0,  0, NULL},
#ifdef DRFLASHTEST
	{KEY,		  13, 15, (uint8_t *)&ADV_Info[8]},  //TODO: DR TEST
#endif
	{KEY_NONE,    0,  0, NULL},
	{KEY_NONE,    0,  0, NULL},
	{KEY_END,    0,   0, NULL}
};
// End of: Advanced: Info  



// Advanced: Battery ////////////////////////////////////////////////////////////////////////////
const uint8_t label_AdvBattMenuTitle[14]="ADV - BATTERY";
const uint8_t label_BattVolts[9]		="VOLTAGE:";
const uint8_t label_BattMode[6]		="MODE:";
const uint8_t label_LastChg[10]			="LAST CHG:";
const uint8_t label_PDVolts[10]			="PD VOLTS:";

const Gadgets	Batt_Diagnostics[]=
{
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvBattMenuTitle, NULL, NULL, 0, 0},
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_BattVolts, NULL, NULL, 0, 1},        
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%4.1d", (uint8_t *)&batt.BattVolts , 0, 250, 12, 1},
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_BattMode, NULL, NULL, 0, 2},        
	 {VARIABLE, TYPE_UINT_8, (uint8_t *)"%4i", (uint8_t *)&batt.ChargeMode , 0, 25, 12, 2},
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_LastChg, NULL, NULL, 0, 3},        
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%4i", (uint8_t *)&NVdata.LastChargeTime , 0, 65535, 9, 3},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Min, NULL, NULL, 13, 3},       
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PDVolts, NULL, NULL, 0, 4},        
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%4.1d", (uint8_t *)&batt.BattVoltsPD , 0, 250, 12, 4},
	 
	 {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys BattMenuKeys[] =
{
    {KEY_END,    0,   0, NULL}
};
// End of: Advanced: Battery

////////////////////////////////////////////////////////////////////////////
const uint8_t label_EE1[15]	="EEPROM ERROR!!";
const uint8_t label_EE2[15]	="A memory error";
const uint8_t label_EE3[16]	="occurred, check";
const uint8_t label_EE4[16]	="settings before";
const uint8_t label_EE5[12]	="restarting.";
const uint8_t label_EE6[9]	="Press OK";

const Gadgets	EEPROM_Error[]=
{
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_EE1, NULL, NULL, 1, 0},
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_EE2, NULL, NULL, 0, 2},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_EE3, NULL, NULL, 0, 3},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_EE4, NULL, NULL, 0, 4},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_EE5, NULL, NULL, 3, 5},
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_EE6, NULL, NULL, 4, 7},
	 
	 {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys EEPROMErrorKeys[] =
{
    {KEY_END,    0,   0, NULL}
};

////////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_AdvCal1MenuTitle[17]="ADV Calibration1";
const uint8_t label_PWMCmd[9]			="PWM Cmd:";
const uint8_t label_PWMAct[9]			="PWM Act:";
const uint8_t label_CalDone[10]			="Cal Done:";
const uint8_t label_MenuToExit[13]		="Menu to Exit";

const Gadgets	Testing_420[]=
{
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvCal1MenuTitle, NULL, NULL, 0, 0},
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PWMCmd, NULL, NULL, 1, 2},
	 {VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.Calib_420Cmd, (uint8_t *)Calib420Strings, 0, 2, 12, 2},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PWMAct, NULL, NULL, 1, 3},
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5.2c", (uint8_t *)&ScreenVariables.MotorSpeed, NULL, NULL, 10, 3},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_CalDone, NULL, NULL, 1, 4},
	 {VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.Calib_420Done, (uint8_t *)LabelEnable, 0, 1, 12, 4},
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PulsesPer, NULL, NULL, 1, 6},
	 {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.UnitsStringNum, (uint8_t *)PulseUnitsString, 0, 5, 8, 6},
#ifdef ChemCalNumber_Bigger
	 {VARIABLE, TYPE_UINT_32, (uint8_t *)"%6i", (uint8_t *)&NVdata.Chem_Pulses, 1, 999999, 10, 6},
#else
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5i", (uint8_t *)&NVdata.Chem_Pulses, 1, 65535, 11, 6},
#endif
	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_MenuToExit, NULL, NULL, 2, 7},
	 
	 {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};


const Keys Testing420Keys[] =
{
		{KEY_NONE,    0,  0, NULL},
		{KEY_NONE,    0,  0, NULL},
		{KEY,		  12, 12, (uint8_t *)&Testing_420[2]},
		{KEY_NONE,    0,  0, NULL},
		{KEY,		  12, 12, (uint8_t *)&Testing_420[6]},
		{KEY_NONE,    0,  0, NULL},
		{KEY,		  11, 15, (uint8_t *)&Testing_420[9]},
		{KEY_END,    0,   0, NULL}
};
////////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t label_AdvCal2MenuTitle[17]="ADV Calibration2";
const uint8_t label_SysType[10]			="Sys Type:";
const uint8_t label_Test[6]				="Test:";
const uint8_t label_TimeLeft[10]		="TimeLeft:";
const uint8_t label_Cmd[5]				="Cmd:";

const Gadgets	Assembly_Test[]=
{
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_AdvCal2MenuTitle, NULL, NULL, 0, 0},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_ID, NULL, NULL, 0, 1},
	 {VARIABLE, TYPE_STRING, NULL, (uint8_t *)NVdata.ModemPN, NULL, NULL, 3, 1},
	 {VARIABLE, TYPE_UINT_8, (uint8_t *)"%1i", (uint8_t *)&ScreenVariables.USBInserted, NULL, NULL, 15, 1},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_SysType, NULL, NULL, 1, 2},
	 {VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.AssemblyTestSystem , (uint8_t *)SystemTypeString, 0, 2, 11, 2},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Test, NULL, NULL, 1, 3},
	 {VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.AssemblyTestState, (uint8_t *)AssemblyTestStateString, 0, 8, 10, 3},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_TimeLeft, NULL, NULL, 1, 4},
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.AssemblyTestTimer, NULL, NULL, 10, 4},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Sec, NULL, NULL, 13, 4},	 
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Cmd, NULL, NULL, 1, 5}, 
	 {VARIABLE, TYPE_STRING, (uint8_t *)&ScreenVariables.AssemblyTestCmd, (uint8_t *)AssemblyTestCmdString, 0, 4, 6, 5},
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%3i", (uint8_t *)&ScreenVariables.ChemPress, NULL, NULL, 10, 5},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_PSI, NULL, NULL, 13, 5},
	 {VARIABLE, TYPE_STRING, (uint8_t *)&NVdata.Auto, (uint8_t *)LabelMode, 0, 1, 1, 6},
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5.2c", (uint8_t *)&ScreenVariables.MotorSpeed, NULL, NULL, 10, 6},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)label_Percent, NULL, NULL, 15, 6},
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)SetRateFormat, (uint8_t *)&NVdata.RateAc, 0, 65535, 1, 7},
	 {VARIABLE, TYPE_UINT_16, (uint8_t *)"%5.1d", (uint8_t *)&ScreenVariables.ProcessVariable, NULL, NULL, 8, 7},
	 {LABEL, TYPE_LABEL, NULL, (uint8_t *)UnitsString[5], NULL, NULL, 13, 7},
	 
	 {GADGET_END,TYPE_NONE, NULL, NULL, NULL, NULL, 0, 0}
};

const Keys AssemblyTestKeys[] =
{
		{KEY_NONE,    0,  0, NULL},
		{KEY_NONE,    0,  0, NULL},
		{KEY,		  11, 11, (uint8_t *)&Assembly_Test[5]},
		{KEY_NONE,    0,  0, NULL},
		{KEY_NONE,    0,  0, NULL},
		{KEY,		  5, 5, (uint8_t *)&Assembly_Test[12]},
		{KEY_END,    0,   0, NULL}
};





void InitVolatileScreenVariables(void) {
	//uint8_t i;

	ScreenVariables.ChemRate = 0;
	ScreenVariables.WaterRate = 0;

#ifndef CodeChange_1_4_2022
	ScreenVariables.ChemPulsesIn = 0;
#endif
	ScreenVariables.ChemPress = 0;

	ScreenVariables.WtrPulsesIn = 0;
	ScreenVariables.WaterPress = 0;

	//ScreenVariables.Batch = 0;

	ScreenVariables.Hour = 0;
	ScreenVariables.Min = 0;

	ScreenVariables.Day = 0;
	ScreenVariables.Month = 0;
	ScreenVariables.Year = 0;

	// Main Menu
	AlarmStringNum = 0;

	// Diagnostic Menu	
	USBStringNum = 0;

	USBMessage = 0;

	ScreenVariables.LoginFileSaved = 0;

	strcpyn(ScreenVariables.USBString, (uint8_t *) ">>INSERT", 8);
	ScreenVariables.USBInserted = 0;

	ScreenVariables.ChemTotReset = False;
	ScreenVariables.UserTotReset = False;
	ScreenVariables.WaterTotReset = False;
#ifndef CodeChange_1_4_2022	
	ScreenVariables.PulseCount = 0;
#endif

	ScreenVariables.Calib_420Cmd = 0;
	ScreenVariables.Calib_420Done = 0;

	ScreenVariables.AssemblyTestState = 0;
	ScreenVariables.AssemblyTestCmd = 2;

	updateChemFormat();

} // End of: void InitVolatileScreenVariables(void) {

void ScreenDraw(uint8_t screen) {
	uint8_t i;
	int8_t TempString[17];

	int8_t DecimalString[5];

	const Gadgets * CurrentMenu;
	int8_t * ptr;
	uint8_t num;
	uint8_t DecimalCharacter;
	
	g_CurrentMenu = screen;

	switch (screen) {

	case MENUMAIN:
		updateBatchLabel();
		if (NVdata.PropModeEnabled) {
			CurrentMenu = MainMenuBatchProportionalGadgets;
		} else {
			CurrentMenu = MainMenuGadgets;
		}
		break;

	case MENURATE:
		CurrentMenu = RateMenuGadgets;
		updateRateLabels();
		break;

	case MENUTOTALS:
		CurrentMenu = TotalMenuGadgets;
		break;

	case MENUBATCH:
		CurrentMenu = BatchMenuGadgets;
		break;

	case MENUSETTINGS2:
		CurrentMenu = Setting2MenuGadgets;
		break;

	case MENUSCHEDULE:
		CurrentMenu = ScheduleMenuGadgets;
		break;

	case MENUAUXINFO:
		CurrentMenu = AuxInfoMenuGadgets;
		break;
		
#ifdef EnableGPSRate
	case MENUGPSRATE:
		CurrentMenu = GPSRateMenuGadgets;
		break;
#endif

	case FEATURES:
		CurrentMenu = FeaturesMenuGadgets;
		break;
	case PROP_CTRL:
		CurrentMenu = PropCtrlMenuGadgets;
		break;
	case REMOTE:
		CurrentMenu = RemoteMenuGadgets;
		break;
	case MENUALARM:
		CurrentMenu = AlarmMenuGadgets;
		break;

	case MENUCHEMICAL:
		CurrentMenu = ChemicalMenuGadgets;
		break;

	case MENUWATER:
		CurrentMenu = WaterMenuGadgets;
		break;

	case MENUCLOCK:
		CurrentMenu = ClockMenuGadgets;
		break;

	case MENUDIAGNOSTIC1:
		CurrentMenu = Diagnostic1MenuGadgets;
		break;

	case MENUDIAGNOSTIC2:
		CurrentMenu = Diagnostic2MenuGadgets;
		break;

	case MENUDIAGNOSTIC3:
		CurrentMenu = Diagnostic3MenuGadgets;
		break;

	case MENUPID:
		CurrentMenu = PIDMenuGadgets;
		break;

	case MENUCELL:
		updateCellDiag();
		CurrentMenu = Cell_Diagnostics;
		break;
		
	case MENUCELL2:
		CurrentMenu = Cell_Diagnostics2;
		break;

	case MENUINFO:
		CurrentMenu = ADV_Info;
		break;

	case MENUBATT:
		CurrentMenu = Batt_Diagnostics;
		break;

	case EEPROMERR:
		CurrentMenu = EEPROM_Error;
		break;
	case CAL420:
		CurrentMenu = Testing_420;
		break;
	case ASSEMBLYTEST:
		CurrentMenu = Assembly_Test;
		break;

	default:
		CurrentMenu = MainMenuGadgets;
		break;
	} // End of: switch (screen) {

	updateChemFormat();

	i = 0;
	while (CurrentMenu[i].type != GADGET_END) {
		if (CurrentMenu[i].type == LABEL){
			lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
					(int8_t *) CurrentMenu[i].Value);
		}

		if (CurrentMenu[i].type == VARIABLE) {
#ifdef Revised
			switch (CurrentMenu[i].subtype) {
			case TYPE_STRING:
				ptr = (int8_t *) CurrentMenu[i].Value;

				// print the correct string
				num = 0;
				while (num != (uint8_t) *CurrentMenu[i].Format_Or_Num) {
					ptr++;
					if (*ptr == 0x0) {
						num++;
						ptr++;
					}
				}

				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) ptr);
				break;

			case TYPE_SINGLESTRING:
				ptr = (int8_t *) CurrentMenu[i].Value;

				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) ptr);
				break;

			case TYPE_INT_8:
			case TYPE_INT_16:
			case TYPE_INT_32:
			case TYPE_UINT_8:
			case TYPE_UINT_16:
			case TYPE_PHONE:
			case TYPE_DECIMAL:

				if (CurrentMenu[i].subtype == TYPE_INT_8)
					convItoA(TempString,
							(int8_t) *((int8_t *) CurrentMenu[i].Value),
							(int8_t *) CurrentMenu[i].Format_Or_Num);

				if (CurrentMenu[i].subtype == TYPE_INT_16)
					convItoA(TempString,
							(int16_t) *((int16_t *) CurrentMenu[i].Value),
							(int8_t *) CurrentMenu[i].Format_Or_Num);

				if (CurrentMenu[i].subtype == TYPE_INT_32)
					convItoA(TempString,
							(int32_t) *((int32_t *) CurrentMenu[i].Value),
							(int8_t *) CurrentMenu[i].Format_Or_Num);

				if (CurrentMenu[i].subtype == TYPE_UINT_8)
					convItoA(TempString,
							(uint8_t) *((uint8_t *) CurrentMenu[i].Value),
							(int8_t *) CurrentMenu[i].Format_Or_Num);

				if (CurrentMenu[i].subtype == TYPE_UINT_16)
					convItoA(TempString,
							(uint16_t) *((uint16_t *) CurrentMenu[i].Value),
							(int8_t *) CurrentMenu[i].Format_Or_Num);

				if (CurrentMenu[i].subtype == TYPE_PHONE) {
					//convItoA (TempString,(uint32_t)*((uint32_t *)CurrentMenu[i].Value), (int8_t *)CurrentMenu[i].Format_Or_Num);
					strcpyn(TempString, CurrentMenu[i].Value, 10);
					TempString[10] = 0x00;
				}

				if (CurrentMenu[i].subtype == TYPE_DECIMAL) {
					convItoA(DecimalString,
							(uint32_t) *((uint32_t *) CurrentMenu[i].Value),
							//(int32_t) CurrentMenu[i].Value,
							(int8_t *) CurrentMenu[i].Format_Or_Num);

					DecimalCharacter = 0;
					while (DecimalCharacter < 5) {
						if (DecimalString[DecimalCharacter] == ' ')
							DecimalString[DecimalCharacter] = '0';
						DecimalCharacter++;
					}
					DecimalString[DecimalCharacter] = 0x0;//Null Terminate				
				}
				if((g_CurrentMenu == MENUSCHEDULE) && (TempString[0] == ' '))
					TempString[0] = '0';
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			} // End of: switch (CurrentMenu[i].subtype) {
#else
			switch (CurrentMenu[i].subtype) {
			case TYPE_STRING:
				ptr = (int8_t *) CurrentMenu[i].Value;

				// print the correct string
				num = 0;
				while (num != (uint8_t) *CurrentMenu[i].Format_Or_Num) {
					ptr++;
					if (*ptr == 0x0) {
						num++;
						ptr++;
					}
				}

				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) ptr);
				break;

			case TYPE_SINGLESTRING:
				ptr = (int8_t *) CurrentMenu[i].Value;

				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) ptr);
				break;
			case TYPE_INT_8:
				convItoA(TempString,
						(int8_t) *((int8_t *) CurrentMenu[i].Value),
						(int8_t *) CurrentMenu[i].Format_Or_Num);
				if((g_CurrentMenu == MENUSCHEDULE) && (TempString[0] == ' ')) {
					// ensures time prints as: 11:03 rather than 11: 3
					TempString[0] = '0';
				}
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			case TYPE_INT_16:
				convItoA(TempString,
						(int16_t) *((int16_t *) CurrentMenu[i].Value),
						(int8_t *) CurrentMenu[i].Format_Or_Num);
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			case TYPE_INT_32:
				convItoA(TempString,
						(int32_t) *((int32_t *) CurrentMenu[i].Value),
						(int8_t *) CurrentMenu[i].Format_Or_Num);
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			case TYPE_UINT_8:
				convItoA(TempString,
						(uint8_t) *((uint8_t *) CurrentMenu[i].Value),
						(int8_t *) CurrentMenu[i].Format_Or_Num);
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			case TYPE_UINT_16:
				convItoA(TempString,
						(uint16_t) *((uint16_t *) CurrentMenu[i].Value),
						(int8_t *) CurrentMenu[i].Format_Or_Num);
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
#ifdef ChemCalNumber_Bigger
			case TYPE_UINT_32:
				convItoA(TempString,
						(int32_t) *((uint32_t *) CurrentMenu[i].Value),
						(int8_t *) CurrentMenu[i].Format_Or_Num);
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
#endif
			case TYPE_PHONE:
				strcpyn(TempString, CurrentMenu[i].Value, 10);
				TempString[10] = 0x00;
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			case TYPE_HR_MIN:
				convItoA(TempString,
						(uint8_t) *((uint8_t *) CurrentMenu[i].Value),
						(int8_t *) CurrentMenu[i].Format_Or_Num);
				if(TempString[0] == ' '){
					// we want preceding 0
					TempString[0] = '0';
				}
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			case TYPE_DECIMAL:
				convItoA(DecimalString,
						(int32_t) *((uint32_t *) CurrentMenu[i].Value),
						//(uint32_t) *((uint32_t *) CurrentMenu[i].Value),
						//(int32_t) CurrentMenu[i].Value,
						(int8_t *) CurrentMenu[i].Format_Or_Num);

				DecimalCharacter = 0;
				while (DecimalCharacter < 5) {
					if (DecimalString[DecimalCharacter] == ' ')
						DecimalString[DecimalCharacter] = '0';
					DecimalCharacter++;
				}
				DecimalString[DecimalCharacter] = 0x0;//Null Terminate	
				lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
						(int8_t *) TempString);
				break;
			} // End of: switch (CurrentMenu[i].subtype) {
#endif
		} // End of: if (CurrentMenu[i].type == VARIABLE) {
		i++;
	} // End of: while (CurrentMenu[i].type != GADGET_END) {
	cursor();
} // End of: void ScreenDraw(uint8_t screen) {

void updateChemFormat() {
	strcpyn(ChemRateFormat,
			(uint8_t *) ChemRateFormatString[NVdata.UnitsStringNum], 5);
	strcpyn(SetRateFormat,
			(uint8_t *) SetChemRateFormatString[NVdata.UnitsStringNum], 5);
}

void UpdateChemRate(uint32_t num) {
	switch (NVdata.UnitsStringNum) {
	case 2: //GPA
	case 3: //GPM
		if (num >= 65535)
			ScreenVariables.ChemRate = 65535;
		else
			ScreenVariables.ChemRate = (uint16_t) num;

		strcpyn(ChemRateFormat, (uint8_t *) ChemRateFormatString[2], 5);
		break;

	case 1: //Oz/Ac
	case 5: //GPH
		if ((num / 10) >= 65535)
			ScreenVariables.ChemRate = 65535;
		else
			ScreenVariables.ChemRate = (uint16_t) (num / 10);

		strcpyn(ChemRateFormat,
				(uint8_t *) ChemRateFormatString[NVdata.UnitsStringNum], 5);
		break;

	case 0: //mL
	case 4: //OZ/min
			// No decimals here, convert directly
		if (num >= 65535)
			ScreenVariables.ChemRate = 65535;
		else
			ScreenVariables.ChemRate = (uint16_t) num;

		strcpyn(ChemRateFormat,
				(uint8_t *) ChemRateFormatString[NVdata.UnitsStringNum], 5);
		break;

	default:
		break;
	} // End of: switch (NVdata.UnitsStringNum) {
} // End of: void UpdateChemRate(uint32_t num) {

void updateCellDiag(){
	ScreenVariables.SigStrength=m1.sigStrength;
	ScreenVariables.SigStrBar=m1.sigStrBarVal;
	if(m1.creg==1 || m1.creg==5)
		ScreenVariables.DataService=1;
	else
		ScreenVariables.DataService=0;
}

void updateRateLabels() {
	unsigned char tempSNum[6];
	uint8_t *p;
	p = &ScreenVariables.FlowTimeLabel[0];

	if (NVdata.UnitsStringNum == 2) {
		strcpyn(ScreenVariables.FlowTimeLabel, " GPM:     ", 10);
		p = p + 10;
		convItoA(tempSNum, ScreenVariables.RatePTime, "%5.1d");
		strcpyn(p, tempSNum, sizeof(tempSNum));
	} else if (NVdata.UnitsStringNum == 1) {
		strcpyn(ScreenVariables.FlowTimeLabel, " OZ/M:    ", 10);
		p = p + 10;
		convItoA(tempSNum, ScreenVariables.RatePTime, "%5.1d");
		strcpyn(p, tempSNum, sizeof(tempSNum));
	} else {
		strcpyn(ScreenVariables.FlowTimeLabel, "                ", 16);
	}

	p = &ScreenVariables.AdjRateLabel[0];
	if (NVdata.WaterFlowEnabled) {
		strcpyn(ScreenVariables.AdjRateLabel, " PROP:    ", 10);
		p = p + 10;
		convItoA(tempSNum, ScreenVariables.AdjRate, ChemRateFormat);
		strcpyn(p, tempSNum, sizeof(tempSNum));
	} else {
		strcpyn(ScreenVariables.AdjRateLabel, "                ", 16);
	}
}

void updateBatchLabel() {
	unsigned char tempSNum[6];
	if (NVdata.BatchEnabled) {
		strcpyn(ScreenVariables.BatchSPMainMenuLabel, "   SPT          ", 16);
		convItoA(tempSNum, NVdata.SetBatch, "%6.1d");
		strcpyn(ScreenVariables.BatchSPMainMenuLabel + 7, tempSNum,
				sizeof(tempSNum));
		strcpyn(ScreenVariables.BatchSPMainMenuLabel + 13, "  ",
				sizeof(tempSNum));
		ScreenVariables.BatchSPMainMenuLabel[15] =
				BatchUnitsString[NVdata.UnitsStringNum][0];
	} else {
		ScreenVariables.BatchSPMainMenuLabel[0] = 0x00;
	}
}

//Controls what is displayed on Top bar of main screen
//Alarms trump everything, 0=No Change, 1=Stopped, 2=Running, 3=Delayed start
void updateMainBanner(uint8_t condition) {
	int8_t temp = 0;
	if (condition == 1) {
		mainBannerFlags.running = 0;
		mainBannerFlags.delayingStart = 0;
		mainBannerFlags.delayingBatchPK = 0;
	} else if (condition == 2) {
		mainBannerFlags.running = 1;
		mainBannerFlags.delayingStart = 0;
		mainBannerFlags.delayingBatchPK = 0;
	} else if (condition == 3) {
		mainBannerFlags.running = 0;
		mainBannerFlags.delayingStart = 1;
		mainBannerFlags.delayingBatchPK = 0;
	}

	//Check for alarms first
	if (AlarmStringNum) {
		memcpy(MainBanner, AlarmMessages[AlarmStringNum], 17);
		MainBannerTimer = 0;//Reset Banner Timer, Alarm will be the only viewed in this state, reset Banner Timer
		return;
	}

	switch (MainBannerTimer / BANNERCYCLESP) {
	case 2:									//Check tank level alarm
		if (ScreenVariables.TankLevelAlarm) {
			strcpy(MainBanner, BannerTxt[0]);
			break;
		}
		MainBannerTimer = BANNERCYCLESP * 3;//If tank alarm not active, skip this cycle and go to case 3
											//no break here on purpose
	case 3:
		if (batt.BattVoltsPD == 0) {
			strcpy(MainBanner, BannerTxt[4]);
			break;
		}
		MainBannerTimer = BANNERCYCLESP * 4;//If battery alarm, skip this cycle and go to case 4
											//no break here on purpose					
	case 4:									//Check timed starts
		if (NVdata.SchStartTime.Mode) {
			if (NVdata.SchStopTime.Mode)
				strcpy(MainBanner, BannerTxt[1]);
			else
				strcpy(MainBanner, BannerTxt[2]);
			break;
		} else if (NVdata.SchStopTime.Mode) {
			strcpy(MainBanner, BannerTxt[3]);
			break;
		} else {
			MainBannerTimer = 0;//If timed starts not active, skip this cycle and go to back to case 0
		}								//no break here on purpose			
	case 0:								//Default banners
	case 1:								//Linger on the main banner for 2 cycles
	default:
		if (mainBannerFlags.running)
			strcpy(MainBanner, "  - RUNNING  -  ");
		else if (mainBannerFlags.delayingStart)
			strcpy(MainBanner, " DELAYING START ");
		else if (mainBannerFlags.delayingBatchPK)
			strcpy(MainBanner, "DELAY IRRIG KILL");
		else
			strcpy(MainBanner, "  - STOPPED  -  ");
		break;
	} // End of: switch (MainBannerTimer / BANNERCYCLESP) {

	MainBannerTimer++;
	if (MainBannerTimer >= (5 * BANNERCYCLESP))
		MainBannerTimer = 0;

} // End of: void updateMainBanner(uint8_t condition){

