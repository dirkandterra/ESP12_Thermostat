/********************************************************************/
/*   Name: Screens                                                  */
/*------------------------------------------------------------------*/
/* Brief description: 	Screen Setup & Handling				        */
/*------------------------------------------------------------------*/
#include "DR_screens.h"
//#include "DR_keys.h"
#include "lcd.h"
#include <string.h>

uint8_t dowText[8][4] = {"---","SUN","MON","TUE","WED","THU","FRI","SAT"};

Screen	ScreenVars;
uint8_t g_CurrentScreen=0;
uint8_t MainBanner[17]="";// Message for Main Banner on front screen

const uint8_t unitF[2]={"F"};

// Home Screen // 
const uint8_t label_Main[15]="--  T-STAT  --";
const uint8_t label_3231[6]="3231:";
const uint8_t label_SHTC[6]="SHTC:";
 
const ScreenObj	MainMenuObj[]=
{
	{TYPE_LABEL, NULL, (uint8_t *)&MainBanner, NULL, NULL, 1, 0}, 
	
    {TYPE_LABEL, NULL, (uint8_t *)&label_3231, NULL, NULL, 1, 1},
    {TYPE_UINT_16, (uint8_t *)"%4.1d", (uint8_t *)&ScreenVars.DS3231Temp, NULL, NULL, 7, 1},
	{TYPE_LABEL, NULL, (uint8_t *)&unitF, NULL, NULL, 12, 1},
     
    {TYPE_LABEL, NULL, (uint8_t *)&label_SHTC, NULL, NULL, 1, 2},
    {TYPE_UINT_16, (uint8_t *)"%4.1d", (uint8_t *)&ScreenVars.SHTCTemp, NULL, NULL, 7, 2},
	{TYPE_LABEL, NULL, (uint8_t *)&unitF, NULL, NULL, 12, 2},

    {TYPE_STRING, (uint8_t *)&ScreenVars.TimeString, 0, 0, 5, 5, 4},
      
    {TYPE_OBJ_END, NULL, NULL, NULL, NULL, 0, 0}
};
 /*
const Keys MainMenuKeys[]=
{
	{KEY_END,    0,   0, NULL}
};
*/

/*const Keys AssemblyTestKeys[] =
{
		{KEY_NONE,    0,  0, NULL},
		{KEY_NONE,    0,  0, NULL},
		{KEY,		  11, 11, (uint8_t *)&Assembly_Test[5]},
		{KEY_NONE,    0,  0, NULL},
		{KEY_NONE,    0,  0, NULL},
		{KEY,		  5, 5, (uint8_t *)&Assembly_Test[12]},
		{KEY_END,    0,   0, NULL}
};*/



void InitVolatileScreenVariables(void) {
	//uint8_t i;

	ScreenVars.DS3231Temp = 0;
	ScreenVars.SHTCTemp = 0;
	ScreenVars.TimeString[0]=0;
	ScreenVars.DateString[0]=0;

} // End of: void InitVolatileScreenVariables(void) {

void ScreenDraw(uint8_t screen) {
	uint8_t i;
	int8_t TempString[17];

	int8_t DecimalString[5];

	const ScreenObj * CurrentMenu;
	int8_t * ptr;
	uint8_t num;
	uint8_t DecimalCharacter;
	
	g_CurrentScreen = screen;

	switch (screen) {

	default:
	case 0:
		CurrentMenu = MainMenuObj;
		break;
	} // End of: switch (screen) {

	i = 0;
	while (CurrentMenu[i].type != TYPE_OBJ_END) {
		switch (CurrentMenu[i].type) {
		case TYPE_LABEL:
			lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
				(int8_t *) CurrentMenu[i].Value);
			break;
			
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
			//if((g_CurrentScreen == MENUSCHEDULE) && (TempString[0] == ' ')) {
				// ensures time prints as: 11:03 rather than 11: 3
				//TempString[0] = '0';
			//}
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
		case TYPE_UINT_32:
			convItoA(TempString,
					(int32_t) *((uint32_t *) CurrentMenu[i].Value),
					(int8_t *) CurrentMenu[i].Format_Or_Num);
			lcdDrawString(CurrentMenu[i].Coords.x, CurrentMenu[i].Coords.y,
					(int8_t *) TempString);
			break;
		case TYPE_PHONE:
			strcpy(TempString, CurrentMenu[i].Value);
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
		} // End of: switch (CurrentMenu[i].type) {
		i++;
	} // End of: while (CurrentMenu[i].type != TYPE_OBJ_END) {
	//**********cursor();
} // End of: void ScreenDraw(uint8_t screen) {



void updateTempsAndTime(){
	ScreenVars.DS3231Temp;
	ScreenVars.SHTCTemp;
}