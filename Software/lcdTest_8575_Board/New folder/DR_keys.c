/********************************************************************/
/*   Name: keys                                                     */
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/

#include "core.h"
#include "comms.h"
#include "global.h"
#include "queue.h"
#include "hw.h"
#include "lcd.h"
#include "utils.h"
#include "screens.h"
#include "event.h"
#include "nvdata.h"
#include "usb.h"
#include "pulse.h"
#include "motor.h"
#include "pid.h"
#include "system.h"
#include "keys.h"
#include "Modem.h"

//#define DebugLclProp 1
//#include "serial.h"

static uint32_t GetPrecision(uint8_t Row, uint8_t Column);
static void MoveColumnRight(void);
static void MoveColumnLeft(void);
void MoveScreen(uint8_t Left);
#ifdef CodeChange_6_23_2022
// removing because this is in utils.h, where it should be.
#else
int8_t atoi_c(int8_t ascii);
#endif
void jumpToScreen(uint8_t NextScreen);

int8_t CurrentRow;
int8_t CurrentColumn;
bool EditMode;
bool ProgMode;
int8_t ProgPos = 1;
const Keys * CurrentKeys;

void KeyPressInit(void) {
	CurrentRow = 0;
	CurrentColumn = 0;
	EditMode = False;
	ProgMode = False;
}

void MoveScreen(uint8_t Left) {
	lcdClearText();
	lcdClearGraphic();

	// If we switch from altering tank level without hitting okay to enact change
	if (g_tankPulseEdit == 1) //1 means it was being edited, throw away change and reset variable to 0
		g_tankPulseEdit = 0;

	// If we switch from clock setting screen, update time
	if (g_CurrentMenu == MENUCLOCK) {
		writeTimeReq = True;
	} else if (g_CurrentMenu == MENURATE)
		FinishChemUnitsChange();			//see if L<->G conversion is needed

	//Menu screen increment
	if (!Left) {
		// navigate screen to the right
		if (g_CurrentMenu == EEPROMERR)	//If the screen is for eeprom error, jump to main menu page.
			g_CurrentMenu = MENUMAIN;
		else {
			g_CurrentMenu++;
			// It seems like these screens need to be in ascending order to work as intended
			if ((g_CurrentMenu == PROP_CTRL)
					&& (NVdata.showPropCtrlScreen == 0)){
				// skip screen
				g_CurrentMenu++;
			}
			if ((g_CurrentMenu == MENUBATCH) && (NVdata.showBatchScreen == 0)){
				// skip screen
				g_CurrentMenu++;
			}
			if ((g_CurrentMenu == REMOTE) && (NVdata.showRemoteScreen == 0)){
				// skip screen
				g_CurrentMenu++;
			}
			if ((g_CurrentMenu == MENUSCHEDULE)
					&& (NVdata.showScheduleScreen == 0)){
				// skip screen
				g_CurrentMenu++;
			}
			if ((g_CurrentMenu == MENUAUXINFO)
					&& (NVdata.showAuxInfoScreen == 0)){
				// skip screen
				g_CurrentMenu++;
			}
#ifdef EnableGPSRate
			if((g_CurrentMenu == MENUGPSRATE) && (NVdata.showGPSRateScreen == 0)){
				// skip screen
				g_CurrentMenu++;
			}
#endif
		}
	} else {
		// navigate screen to the left
		if (g_CurrentMenu > 199) {
			// Assembly Testing Screens
			if (g_CurrentMenu != 200)
				g_CurrentMenu--;
			else
				g_CurrentMenu = NOMENUTEST - 1; //Wrap around
		} else if (g_CurrentMenu > 99) {
			// Advanced Menu Screens
			if (g_CurrentMenu != 100)
				g_CurrentMenu--;
			else
				g_CurrentMenu = NOMENUADV - 1; //Wrap around		  
		} else {
			// Standard Screens
			if (g_CurrentMenu != MENUMAIN)
				g_CurrentMenu--;    //Main Screens
			else
				g_CurrentMenu = NOMENU - 1; //Wrap around

			// It seems like these screens need to be in descending order to work as intended
#ifdef EnableGPSRate
			if((g_CurrentMenu == MENUGPSRATE) && (NVdata.showGPSRateScreen == 0)){
				// skip screen
				g_CurrentMenu--;
			}
#endif
			if ((g_CurrentMenu == MENUAUXINFO)
					&& (NVdata.showAuxInfoScreen == 0)){
				// skip screen
				g_CurrentMenu--;
			}
			if ((g_CurrentMenu == MENUSCHEDULE)
					&& (NVdata.showScheduleScreen == 0)){
				// skip screen
				g_CurrentMenu--;
			}
			if ((g_CurrentMenu == REMOTE) && (NVdata.showRemoteScreen == 0)){
				// skip screen
				g_CurrentMenu--;
			}
			if ((g_CurrentMenu == MENUBATCH) && (NVdata.showBatchScreen == 0)){
				// skip screen
				g_CurrentMenu--;
			}
			if ((g_CurrentMenu == PROP_CTRL)
					&& (NVdata.showPropCtrlScreen == 0)){
				// skip screen
				g_CurrentMenu--;
			}
		}

	} // End of: move screen right/left
	
	EditMode = False;

	if (g_CurrentMenu == NOMENU)
		g_CurrentMenu = MENUMAIN;
	if (g_CurrentMenu == NOMENUADV)
		g_CurrentMenu = MENUDIAGNOSTIC1;
	if (g_CurrentMenu == NOMENUTEST)
		g_CurrentMenu = CAL420;

	lcdCursor(0, 0);	 //Turn Off Cursor

	switch (g_CurrentMenu) {
	case MENUMAIN:
		CurrentKeys = MainMenuKeys;
		lcdCursor(0, 0);	 //Turn Off Cursor
		break;

	case MENURATE:
		CurrentKeys = RateMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor			
		break;

	case MENUBATCH:
		CurrentKeys = BatchMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;

	case MENUSCHEDULE:
		CurrentKeys = ScheduleMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;

	case MENUALARM:
		CurrentKeys = AlarmMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;

	case MENUCHEMICAL:
		CurrentKeys = ChemicalMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor			
		break;

	case MENUWATER:
		CurrentKeys = WaterMenuKeys;
		lcdCursor(1, 0); //Turn On Cursor					
		break;

	case MENUSETTINGS2:
		CurrentKeys = SettingMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor		
		break;

	case MENUAUXINFO:
		CurrentKeys = AuxInfoMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;
#ifdef EnableGPSRate
	case MENUGPSRATE:
		CurrentKeys = GPSRateMenuKeys;
		lcdCursor(1,0); // cursor on
		break;
#endif
	case FEATURES:
		CurrentKeys = FeaturesMenuKeys;
		lcdCursor(1, 0); // Cursor On
		break;
	case PROP_CTRL:
		CurrentKeys = PropCtrlMenuKeys;
		lcdCursor(1, 0); // Cursor On
		break;
	case REMOTE:
		CurrentKeys = RemoteMenuKeys;
		lcdCursor(1, 0); // Cursor On
		break;
	case MENUCLOCK:
		CurrentKeys = ClockMenuKeys;

		// todo: should this be here, or somewhere else? 8-3-2022
#ifndef refactor1
		ScreenVariables.setHour = atoi_c(ScreenVariables.TimeString[0]) * 10
				+ atoi_c(ScreenVariables.TimeString[1]);
		ScreenVariables.setMin = atoi_c(ScreenVariables.TimeString[3]) * 10
				+ atoi_c(ScreenVariables.TimeString[4]);
		ScreenVariables.setSec = atoi_c(ScreenVariables.TimeString[6]) * 10
				+ atoi_c(ScreenVariables.TimeString[7]);
		ScreenVariables.setDay = atoi_c(ScreenVariables.DateString[3]) * 10
				+ atoi_c(ScreenVariables.DateString[4]);
		ScreenVariables.setMonth = atoi_c(ScreenVariables.DateString[0]) * 10
				+ atoi_c(ScreenVariables.DateString[1]);
		ScreenVariables.setYear = atoi_c(ScreenVariables.DateString[6]) * 10
				+ atoi_c(ScreenVariables.DateString[7]);
#endif		
		
		lcdCursor(1, 0);	 //Turn ON Cursor

		break;

	case MENUDIAGNOSTIC1:
		CurrentKeys = Diagnostic1MenuKeys;
		lcdCursor(0, 0);	 //Turn Off Cursor
		break;

	case MENUDIAGNOSTIC2:
		CurrentKeys = Diagnostic2MenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;

	case MENUDIAGNOSTIC3:
		CurrentKeys = Diagnostic3MenuKeys;
		lcdCursor(0, 0);	 //Turn Off Cursor
		break;

	case MENUPID:
		CurrentKeys = PIDMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;

	case MENUCELL:
		CurrentKeys = CellMenuKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;
		
	case MENUCELL2:
		CurrentKeys = SMSMenuKeys;
		lcdCursor(1,0);	 //Turn ON Cursor
		break;

	case MENUINFO:
		CurrentKeys = ADV_InfoKeys;
		lcdCursor(1, 0);	 //Turn ON Cursor
		break;

	case MENUTOTALS:
		CurrentKeys = TotalMenuKeys;
		lcdCursor(1, 0);	 //Turn On Cursor
		break;

	case MENUBATT:
		CurrentKeys = BattMenuKeys;
		lcdCursor(0, 0);	 //Turn Off Cursor
		break;

	case EEPROMERR:
		CurrentKeys = EEPROMErrorKeys;
		lcdCursor(0, 0);	 //Turn Off Cursor
		break;

	case CAL420:
		CurrentKeys = Testing420Keys;
		lcdCursor(1, 0);	 //Turn On Cursor
		break;

	case ASSEMBLYTEST:
		CurrentKeys = AssemblyTestKeys;
		lcdCursor(1, 0);	 //Turn On Cursor
		break;

	default:
		// No Keys here
		lcdCursor(0, 0);	 //Turn Off Cursor
		break;
	} // End of: switch (g_CurrentMenu) {

	CurrentRow = 0;
	while (CurrentKeys[CurrentRow].KeyType != KEY){
		CurrentRow++;
	}

	CurrentColumn = 0;
	lcdSetCursor((uint8_t) CurrentRow, (uint8_t) CurrentColumn);
	ScreenDraw(g_CurrentMenu);
	// Update in EEPROM any variables that have changed
	CheckAndUpdateEEPROM(False);
} // End of: void MoveScreen(uint8_t Left) {

void ProcessKeyPressController(AEvent event) {
	uint8_t data;
	uint8_t * pointer_eight;
	uint16_t * pointer_sixteen;
	uint32_t * pointer_thirtytwo;
	uint32_t thirtytwo;
	int8_t TempString[17];
	int8_t DecimalString[5];
	const Gadgets * CurrentGadget;
	uint32_t Precision;
	int8_t * ptr;
	uint8_t num;
	//uint16_t baudrate;
	uint8_t PhoneCharacter;
	uint8_t DecimalValue;
	//uint8_t atoiChar;
	uint16_t adjustedRow = 0;

	AdvMenuTimer = 0;

	switch (event.parameter) {
	// ####################################
	//
	//  Key MENU
	// ####################################
	case KeyMenu:
		if (g_CurrentMenu == MENUCLOCK) {
			writeTimeReq = True;
		}
		//This will bring it to first screen of basic menu
		if (g_CurrentMenu != EEPROMERR)
			goToHome();
		break;

	// ####################################
	//
	//                 Key OK
	//
	// ####################################
	case KeyOK:
		switch (g_CurrentMenu) {
		case PROP_CTRL:
			// under KeyOK
			if (EditMode == True) {
				if ((NVdata.WaterFlowEnabled == 0) || (NVdata.showPropCtrlScreen == 0)) {
					// Disable feature; Hide screen
#ifdef DebugLclProp
					Serialt("PropOff\r\n", UART2);
#endif
					NVdata.WaterFlowEnabled = 0;
					NVdata.showPropCtrlScreen = 0;
				}
			}
			break;
		case MENUBATCH:
			// under KeyOK
			if (EditMode == True) {
				if ((NVdata.BatchEnabled == 0) || (NVdata.showBatchScreen == 0)) {
					// Disable feature; Hide screen
					NVdata.BatchEnabled = 0;
					NVdata.showBatchScreen = 0;
				}
				// We need to update this info on the server
				m1.connectStatus.logExtDataToServer = 1;
			}
			break;
		case REMOTE:
			// under KeyOK
			if (EditMode == True) {
				if ((NVdata.PropModeEnabled == 0) || (NVdata.showRemoteScreen == 0)) {
					// Disable feature; Hide screen
					NVdata.PropModeEnabled = 0;
					NVdata.showRemoteScreen = 0;
					// todo: yes or no on this stop? // stopPumpAndControl(CANCELKEY_STOP);
				}
			}
			break;
#ifdef myTest1
		case MENUAUXINFO:
			// under KeyOK
			if(NVdata.WaterFlowEnabled == 0){
				// also hide screen this feature screen
				NVdata.showAuxInfoScreen = 0;
			}
			if(NVdata.showAuxInfoScreen == 0){
				// also disable this feature
				NVdata.WaterFlowEnabled = 0;
			}
			break;
#endif
		case MENUSCHEDULE:
			// under KeyOK
			if (EditMode == True){
#ifdef Testing1
			switch (CurrentRow) {
			//If editing start time, disable start mode
			case 2:
				NVdata.SchStartTime.Mode = SCH_SS_DISABLE;
				break;
				//case 3:

				//If editing stop time, disable stop mode
				//case 6:
			case 4:
				NVdata.SchStopTime.Mode = 0;
				break;
			default:
				break;
			}
#endif
				// todo: Look through this stuff. 10-20-2022
				if ((NVdata.showScheduleScreen == 0)
						|| ((NVdata.SchStartTime.Mode == SCH_SS_DISABLE)
								&& (NVdata.SchStopTime.Mode == SCH_SS_DISABLE))) {
					// Disable feature; Hide screen
					NVdata.SchStartTime.Mode = SCH_SS_DISABLE;
					NVdata.SchStopTime.Mode = SCH_SS_DISABLE;
					NVdata.showScheduleScreen = 0;
				}
				// We need to update this info on the server
				m1.connectStatus.logExtDataToServer = 1;
			}
			break;
		case MENUCELL2:
			if (CurrentRow == 4 && EditMode) {
				if (ScreenVariables.ProvRequest) {
					ScreenVariables.ProvRequest = 0;
					m1.provision = 1;
				}
			}
			break;
		case MENUMAIN:
			// under KeyOK
			startPumpAndControl();
			break;
		case MENURATE:
			// under KeyOK
			//do not allow change of units when running (row 2 on menu rate page)
			if (mMotor.Enabled && CurrentRow == 2)
				EditMode = True;//Set to true so it jumps back to false edit mode.

			if (CurrentRow == 6) {
				//If editing Tank level, don't let current calcs overwrite
				if (EditMode)
					g_tankPulseEdit = 2;	//2 means that it needs saved off
				else
					g_tankPulseEdit = 1;//1 means it is being edited, don't overwrite with new calculations, 
			}
			break;
		case MENUDIAGNOSTIC1:
			// under KeyOK
			startPumpAndControl();
			//ScreenVariables.ChemPulsesIn = 0;
			//ScreenVariables.WtrPulsesIn = 0;
			// StartDiagnostic(NVdata.ControlStringNum);
			break;
		case EEPROMERR:
			// under KeyOK
			goToHome();
			break;
		case FEATURES:
			// under KeyOK
			// if a screen changes from disabled to enabled, 
			// need to jump to the newly enabled screen
			if (EditMode == True) {
				// taking action after edit
				//EditMode = False;
				switch (CurrentRow) {
				case 1:
					// PROP CTRL
					if (NVdata.showPropCtrlScreen){
						// Only jump if set to enabled, no jump if disabled
#ifdef DebugLclProp
						Serialt("PropOn\r\n", UART2);
#endif
						NVdata.WaterFlowEnabled = 1; // Enable feature
						jumpToScreen(PROP_CTRL);
					} else {
						NVdata.WaterFlowEnabled = 0; // Disable feature
					}
					break;
				case 2:
					// Batching
					// We need to update this info on the server
					m1.connectStatus.logExtDataToServer = 1;
					if (NVdata.showBatchScreen) {
						NVdata.BatchEnabled = 1; // Enable feature
						// Only jump if set to enabled, no jump if disabled
						jumpToScreen(MENUBATCH);
					} else {
						NVdata.BatchEnabled = 0; // Disable feature
					}
					break;
				case 3:
					// Remote Spt
					if (NVdata.showRemoteScreen) {
						NVdata.PropModeEnabled = 1; // Enable feature
						// Only jump if set to enabled, no jump if disabled
						jumpToScreen(REMOTE);
					} else {
						NVdata.PropModeEnabled = 0; // Disable feature
					}
					break;
				case 4:
					// Schedule
					if (NVdata.showScheduleScreen) {
						// Only jump if set to enabled, no jump if disabled
						jumpToScreen(MENUSCHEDULE);
					} else {
						// also disable this feature
						NVdata.SchStartTime.Mode = SCH_SS_DISABLE;
						NVdata.SchStopTime.Mode = SCH_SS_DISABLE;
						// We need to update this info on the server
						m1.connectStatus.logExtDataToServer = 1;
					}
					break;
				case 5:
					// Aux Info
					if (NVdata.showAuxInfoScreen)
						// Only jump if set to enabled, no jump if disabled
						jumpToScreen(MENUAUXINFO);
					break;
#ifdef EnableGPSRate
				case 6:
					// GPS Rate
					if(NVdata.showGPSRateScreen){
						// Only jump if set to enabled, no jump if disabled
						jumpToScreen(MENUGPSRATE);
					}
					break;
#endif
				}
			} 
			break; // End of: case FEATURES: of KeyOK
		//case MENUSETTINGS1:
		case MENUCHEMICAL:
		case MENUALARM:
		case MENUWATER:	   // 2		
		case MENUCLOCK:
		case MENUAUXINFO:
		case MENUSETTINGS2:
		case MENUPID:
		case MENUCELL:
		case MENUTOTALS:
		case MENUINFO:
		case CAL420:
		case ASSEMBLYTEST:
#ifdef EnableGPSRate
		case MENUGPSRATE:
#endif
			// under KeyOK
			break;
		default:
			// under KeyOK
			break;
		} // End of: switch (g_CurrentMenu) {

		if (EditMode == True) {
			EditMode = False;
			CurrentColumn = 0; // added by dr
			lcdSetCursor((uint8_t) CurrentRow, 0);
			lcdCursor(1, 0);
		} else {
			if ((g_CurrentMenu != MENUMAIN)
					&& (g_CurrentMenu != MENUDIAGNOSTIC1)
					&& (g_CurrentMenu != MENUDIAGNOSTIC3)) {
				EditMode = True;

				if (CurrentRow != 0) {
					CurrentColumn = CurrentKeys[CurrentRow].RightColumn;
					// flash cursor at correct position
					lcdCursor(1, 1);
					lcdSetCursor((uint8_t) CurrentRow, (uint8_t) CurrentColumn);
				}
			}

		}
		CheckAndUpdateEEPROM(False);

		break; // End of KeyOK

		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//   Key LEFT	
		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%					
	case KeyLeft:
		switch (g_CurrentMenu) {
		case MENUMAIN:
		case MENURATE:
		case MENUBATCH:
			//case MENUSCHEDULE:
			//case MENUSETTINGS1:
		case MENUALARM:
		case MENUCHEMICAL:
		case MENUWATER:
		case MENUSETTINGS2:
		case MENUCLOCK:
		case MENUPID:
		case MENUDIAGNOSTIC3:
		case MENUDIAGNOSTIC2:
		case MENUDIAGNOSTIC1:
		case MENUCELL:
		case MENUCELL2:
		case MENUINFO:
		case MENUBATT:
		case MENUTOTALS:
		case MENUAUXINFO:
#ifdef EnableGPSRate
		case MENUGPSRATE:
#endif
		case FEATURES:
		case PROP_CTRL:
		case REMOTE:
		case CAL420:
		case ASSEMBLYTEST:
			if (EditMode == True) {
				adjustedRow = CurrentRow;

				// we are editing system variables
				CurrentColumn--;

				if (CurrentColumn < CurrentKeys[CurrentRow].LeftColumn)
					CurrentColumn = CurrentKeys[CurrentRow].RightColumn;

				data = lcdReadChar((int16_t) CurrentColumn,
						(int16_t) adjustedRow) + 32;

				if (data == '.') //Skip Decimal
					CurrentColumn--;

				CurrentGadget = (Gadgets *) CurrentKeys[CurrentRow].Value;
				if (CurrentGadget->subtype == TYPE_PHONE) {
					if (CurrentColumn != 15) {
						// we can only move one empty space for phone numbers
						data = lcdReadChar((int16_t) CurrentColumn + 1,
								(int16_t) adjustedRow) + 32;
						if (data == ' ')
							CurrentColumn++;
					}
				}

				if (data == ' ')   //Skip Space
					CurrentColumn = CurrentKeys[adjustedRow].RightColumn;

				if (CurrentColumn < CurrentKeys[CurrentRow].LeftColumn)
					CurrentColumn = CurrentKeys[CurrentRow].RightColumn;

				lcdSetCursor((uint8_t) adjustedRow, (uint8_t) CurrentColumn);
			} else {
				MoveScreen(True);
			}
			break;
		case MENUSCHEDULE:
			// This is a key Left movement.
			if (EditMode == True) {
				// we are editing system variables
				adjustedRow = CurrentRow;
				CurrentColumn--;
				if (CurrentColumn < CurrentKeys[CurrentRow].LeftColumn) {
					CurrentColumn = CurrentKeys[CurrentRow].RightColumn;
					//CurrentGadget = (Gadgets *) CurrentKeys[CurrentRow].Value;
				}

				data = lcdReadChar((int16_t) CurrentColumn,
						(int16_t) adjustedRow) + 32;
				if (data == ':') {
					CurrentColumn--;
					//CurrentGadget = (Gadgets *) CurrentKeys[9].Value;
				}
				if (CurrentColumn < CurrentKeys[CurrentRow].LeftColumn) {
					CurrentColumn = CurrentKeys[CurrentRow].RightColumn;
					//CurrentGadget = (Gadgets *) CurrentKeys[CurrentRow].Value;
				}
				lcdSetCursor((uint8_t) adjustedRow, (uint8_t) CurrentColumn);
			} else {
				MoveScreen(True);
			}
			break;
		} // End of: switch (g_CurrentMenu) {

		break; // break of: case KeyLeft

		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//                                 Key RIGHT		
		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	case KeyRight:
		switch (g_CurrentMenu) {
		case MENUMAIN:
		case MENURATE:
		case MENUBATCH:
			//case MENUSCHEDULE:
			//case MENUSETTINGS1:
		case MENUALARM:
		case MENUCHEMICAL:
		case MENUWATER:
		case MENUSETTINGS2:
		case MENUCLOCK:
		case MENUPID:
		case MENUDIAGNOSTIC3:
		case MENUDIAGNOSTIC2:
		case MENUDIAGNOSTIC1:
		case MENUCELL:
		case MENUCELL2:
		case MENUINFO:
		case MENUBATT:
		case MENUAUXINFO:
#ifdef EnableGPSRate
		case MENUGPSRATE:
#endif
		case FEATURES:
		case PROP_CTRL:
		case REMOTE:
		case MENUTOTALS:
		case CAL420:
		case ASSEMBLYTEST:
			adjustedRow = CurrentRow;
			if (EditMode == True) {
				// we are editing system variables
				CurrentColumn++;

				if (CurrentColumn > CurrentKeys[CurrentRow].RightColumn) {
					CurrentColumn = CurrentKeys[CurrentRow].LeftColumn;

					while ((lcdReadChar((int16_t) CurrentColumn,
							(int16_t) adjustedRow) + 32) == ' ')
						CurrentColumn++;
				}

				data = lcdReadChar((int16_t) CurrentColumn,
						(int16_t) adjustedRow) + 32;
				if (data == '.')   //Skip Decimal
					CurrentColumn++;

				if (CurrentColumn > CurrentKeys[CurrentRow].RightColumn)
					CurrentColumn = CurrentKeys[CurrentRow].LeftColumn;

				lcdSetCursor((uint8_t) adjustedRow, (uint8_t) CurrentColumn);
			} else {
				MoveScreen(False);
			}
			break;
		case MENUSCHEDULE:
			// This is a key Right movement.
			if (EditMode == True) {
				// we are editing system variables
				adjustedRow = CurrentRow;
				
#ifdef NotTesting1
				switch (CurrentRow) {
				//If editing start time, disable start mode
				case 2:
					NVdata.SchStartTime.Mode = SCH_SS_DISABLE;
					break;
					//case 3:

					//If editing stop time, disable stop mode
					//case 6:
				case 4:
					NVdata.SchStopTime.Mode = 0;
					break;
				default:
					break;
				}
#endif
				
				CurrentColumn++;
				if (CurrentColumn > CurrentKeys[CurrentRow].RightColumn) {
					CurrentColumn = CurrentKeys[CurrentRow].LeftColumn;
					//CurrentGadget = (Gadgets *) CurrentKeys[9].Value;
				}

				data = lcdReadChar((int16_t) CurrentColumn,
						(int16_t) adjustedRow) + 32;
				if (data == ':') {
					CurrentColumn++;
					//CurrentGadget = (Gadgets *) CurrentKeys[CurrentRow].Value;
				}
				lcdSetCursor((uint8_t) adjustedRow, (uint8_t) CurrentColumn);
			} else {
				MoveScreen(False);
			}
			break;
		default:

			break;
		} // End of: switch (g_CurrentMenu) {
		break; // break of: case KeyRight

		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//                     Key UP			
		//
		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	case KeyUp:
		switch (g_CurrentMenu) {
		case MENUCHEMICAL:
		case MENURATE:
		case MENUBATCH:
		case MENUSCHEDULE:
			//case MENUSETTINGS1:
		case MENUALARM:
		case MENUWATER:
		case MENUSETTINGS2:
		case MENUAUXINFO:
#ifdef EnableGPSRate
		case MENUGPSRATE:
#endif
		case FEATURES:
		case PROP_CTRL:
		case REMOTE:
		case MENUCLOCK:
		case MENUPID:
		case MENUTOTALS:
		case MENUCELL:
		case MENUCELL2:
		case MENUINFO:
		case MENUDIAGNOSTIC2:
		case CAL420:
		case ASSEMBLYTEST:

			if (EditMode == False) {
				// we are scrolling UP through menu items
				CurrentRow--;
				while ((CurrentKeys[CurrentRow].KeyType != KEY)
						&& (CurrentRow != 0))
					CurrentRow--;

				if (CurrentRow == 0) {
					CurrentRow = 0;
					// let's find the last row
					while (CurrentKeys[CurrentRow].KeyType != KEY_END)
						CurrentRow++;

					// let's find the first row used from the bottom up
					while (CurrentKeys[CurrentRow].KeyType != KEY)
						CurrentRow--;
				}

				while (CurrentKeys[CurrentRow].KeyType != KEY)
					CurrentRow--;

				lcdSetCursor((uint8_t) CurrentRow, 0);
			} else {
				// we are editing system variables
				switch (g_CurrentMenu) {
				case MENUCHEMICAL:
				case MENURATE:
				case MENUBATCH:
					//case MENUSCHEDULE:
					//case MENUSETTINGS1:
				case MENUALARM:
				case MENUWATER:
				case MENUSETTINGS2:
				case MENUAUXINFO:
#ifdef EnableGPSRate
				case MENUGPSRATE:
#endif
				case FEATURES:
				case PROP_CTRL:
				case REMOTE:
				case MENUCLOCK:
				case MENUPID:
				case MENUTOTALS:
				case MENUDIAGNOSTIC2:
				case MENUCELL:
				case MENUCELL2:
				case MENUINFO:
				case CAL420:
				case ASSEMBLYTEST:
					if (CurrentKeys[CurrentRow].KeyType == KEY) {
						// we need to increase integer value																
						CurrentGadget =
								(Gadgets *) CurrentKeys[CurrentRow].Value;

						Precision = GetPrecision((uint8_t) CurrentRow,
								(uint8_t) CurrentColumn);
						ptr = TempString;

						switch (CurrentGadget->subtype) {
						case TYPE_INT_8:
						case TYPE_UINT_8:
							pointer_eight = (uint8_t *) CurrentGadget->Value;

							if (((int16_t) *pointer_eight + Precision)
									< CurrentGadget->MaxValue) {
								if (((int16_t) *pointer_eight + Precision)
										/ (10 * Precision)
										!= (int16_t) *pointer_eight
												/ (10 * Precision))
									MoveColumnLeft();

								*pointer_eight = *pointer_eight
										+ (uint8_t) Precision;
							} else {

								*pointer_eight =
										(uint8_t) CurrentGadget->MaxValue;
							}
							convItoA(ptr, (uint8_t) *pointer_eight,
									(int8_t *) CurrentGadget->Format_Or_Num);
							break;

						case TYPE_INT_16:
						case TYPE_UINT_16:
							pointer_sixteen = (uint16_t *) CurrentGadget->Value;

							if ((*pointer_sixteen + Precision)
									< CurrentGadget->MaxValue) {
								if ((*pointer_sixteen + Precision)
										/ (10 * Precision)
										!= *pointer_sixteen / (10 * Precision))
									MoveColumnLeft();

								*pointer_sixteen = *pointer_sixteen
										+ (uint16_t) Precision;
							} else {
								*pointer_sixteen =
										(uint16_t) CurrentGadget->MaxValue;
							}
							convItoA(ptr, (uint16_t) *pointer_sixteen,
									(int8_t *) CurrentGadget->Format_Or_Num);
							break;

						case TYPE_INT_32:
							break;
#ifdef ChemCalNumber_Bigger
						case TYPE_UINT_32:
							pointer_thirtytwo = (uint32_t *) CurrentGadget->Value;

							if ((*pointer_thirtytwo + Precision)
									< CurrentGadget->MaxValue) {
								if ((*pointer_thirtytwo + Precision)
										/ (10 * Precision)
										!= *pointer_thirtytwo / (10 * Precision))
									MoveColumnLeft();

								*pointer_thirtytwo = *pointer_thirtytwo
										+ Precision;
							} else {
								*pointer_thirtytwo =
										CurrentGadget->MaxValue;
							}
							convItoA(ptr, (int32_t) *pointer_thirtytwo,
									(int8_t *) CurrentGadget->Format_Or_Num);
							break;
#endif

						case TYPE_PHONE:
							pointer_eight = (uint8_t *) CurrentGadget->Value
									+ CurrentColumn
									- CurrentKeys[CurrentRow].LeftColumn;

							if (*pointer_eight != '9') {
								*pointer_eight = *pointer_eight + 1;
							}
							//convItoA (ptr,(uint32_t)thirtytwo, (int8_t *)CurrentGadget->Format_Or_Num);																																																																																																	
							//*((uint32_t *)CurrentGadget->Value) = (uint32_t)thirtytwo;

							// fill in the leading spaces with zero's
							PhoneCharacter = 0;
							while (PhoneCharacter < 10) {
								if (*ptr == ' ')
									*ptr = '0';
								PhoneCharacter++;
								ptr++;
							}
							*ptr = 0x0;

							ptr = (uint8_t *) CurrentGadget->Value;

							break;

						case TYPE_DECIMAL:
							ptr = DecimalString;
							pointer_thirtytwo =
									(uint32_t *) CurrentGadget->Value;
							thirtytwo =
									(uint32_t) *((uint32_t *) CurrentGadget->Value);

							data = lcdReadChar(CurrentColumn, CurrentRow) + 32;
							if (data != '9') {
								if (((uint32_t) thirtytwo + (uint32_t) Precision)
										<= (uint32_t) CurrentGadget->MaxValue)
									thirtytwo = thirtytwo + Precision;
							}
							convItoA(ptr, (int32_t) thirtytwo,
									(int8_t *) CurrentGadget->Format_Or_Num);
							*((uint32_t *) CurrentGadget->Value) =
									(uint32_t) thirtytwo;

							// fill in the leading spaces with zero's
							DecimalValue = 0;
							while (DecimalValue < 10) {
								if (*ptr == ' ')
									*ptr = '0';
								DecimalValue++;
								ptr++;
							}
							*ptr = 0x0;

							ptr = DecimalString;

							break;

						case TYPE_STRING:
						case TYPE_SINGLESTRING:
							pointer_eight =
									(uint8_t *) CurrentGadget->Format_Or_Num;

							if (*pointer_eight
									== (uint8_t) CurrentGadget->MaxValue)
								*pointer_eight =
										(uint8_t) CurrentGadget->MinValue;
							else
								*pointer_eight = *pointer_eight + 1;

							if (CurrentGadget->subtype == TYPE_STRING) {
								ptr = (int8_t *) CurrentGadget->Value;

								num = 0;
								while (num != *pointer_eight) {
									ptr++;
									if (*ptr == 0x0) {
										num++;
										ptr++;
									}
								}
							}
							break;

						default:
							break;
						}	//CurrentGadget->subtype
						lcdDrawString(CurrentGadget->Coords.x,
								CurrentGadget->Coords.y, ptr);
					} // if CurrentKeys[CurrentRow].KeyType == KEY														
					break;
				case MENUSCHEDULE:
					// This is Key Up movement

					if (CurrentKeys[CurrentRow].KeyType == KEY) {
						// hours and minutes are on same row
						if ((CurrentColumn == 14) || (CurrentColumn == 15))
							CurrentGadget =
									(Gadgets *) (CurrentKeys[CurrentRow].Value);
						else if ((CurrentColumn == 11)
								|| (CurrentColumn == 12)) {
							if (CurrentRow == 2)
								CurrentGadget =
										(Gadgets *) CurrentKeys[9].Value;
							else if (CurrentRow == 4)
								CurrentGadget =
										(Gadgets *) CurrentKeys[10].Value;
						}
						switch (CurrentGadget->subtype) {
#ifdef CodeChange_1_10_2022
						case TYPE_HR_MIN:
							
#else
						case TYPE_INT_8:
						case TYPE_UINT_8:
#endif
							// we need to increase integer value			
							ptr = TempString;
							pointer_eight = (uint8_t *) CurrentGadget->Value;
							if ((CurrentColumn == 11)
									|| (CurrentColumn == 14)) {
								// 10s place of hour or minute
								/*Precision = 10;
								 if((*pointer_eight + Precision)	< (uint8_t) CurrentGadget->MaxValue){
								 *pointer_eight = *pointer_eight + (uint8_t)Precision;
								 }else{
								 *pointer_eight = (uint8_t) CurrentGadget->MinValue;
								 }*/
								if (*pointer_eight
										!= (uint8_t) CurrentGadget->MaxValue) {
									if ((*pointer_eight + 10)
											< (uint8_t) CurrentGadget->MaxValue) {
										*pointer_eight = *pointer_eight + 10;
									} else {
										*pointer_eight =
												(uint8_t) CurrentGadget->MaxValue;
									}
								}
							} else if ((CurrentColumn == 12)
									|| (CurrentColumn == 15)) {
								// 1s place of hour or minute
								/*Precision = 1;
								 if((*pointer_eight + Precision)	< (uint8_t) CurrentGadget->MaxValue){
								 *pointer_eight = *pointer_eight + (uint8_t)Precision;
								 }else{
								 *pointer_eight = (uint8_t) CurrentGadget->MinValue;
								 }*/
								if (*pointer_eight
										!= (uint8_t) CurrentGadget->MaxValue) {
									if ((*pointer_eight + 1)
											< (uint8_t) CurrentGadget->MaxValue) {
										*pointer_eight = *pointer_eight + 1;
									} else {
										*pointer_eight =
												(uint8_t) CurrentGadget->MaxValue;
									}
								}
							}

							convItoA(ptr, (uint8_t) *pointer_eight,
									(int8_t *) CurrentGadget->Format_Or_Num);
							// fill in the leading spaces with zero's
							if (TempString[0] == ' ')
								TempString[0] = '0';
							break;
						case TYPE_STRING:
						case TYPE_SINGLESTRING:
							pointer_eight =
									(uint8_t *) CurrentGadget->Format_Or_Num;

							if (*pointer_eight
									== (uint8_t) CurrentGadget->MaxValue)
								*pointer_eight =
										(uint8_t) CurrentGadget->MinValue;
							else
								*pointer_eight = *pointer_eight + 1;

							if (CurrentGadget->subtype == TYPE_STRING) {
								ptr = (int8_t *) CurrentGadget->Value;

								num = 0;
								while (num != *pointer_eight) {
									ptr++;
									if (*ptr == 0x0) {
										num++;
										ptr++;
									}
								}
							}
							break;
						}

						lcdDrawString(CurrentGadget->Coords.x,
								CurrentGadget->Coords.y, ptr);
					}
					break; // End of case MENUSCHEDULE:
				default:
					break;
				} //switch (g_CurrentMenu) (2)																	
			}
			break;

		case MENUDIAGNOSTIC1:
		case MENUMAIN:
			if (ScreenVariables.ButtonPressed == 1) {
				ScreenVariables.ButtonPressed = 0;
			} else {
				ScreenVariables.ButtonPressed = 1;
			}
			break;
		default:
			break;
		}	//switch (g_CurrentMenu)(1)
		break; // break of: case KeyUp
		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//
		//                 Key DOWN																		
		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	case KeyDown:
		switch (g_CurrentMenu) {
		case MENUCHEMICAL:
		case MENURATE:
			//case MENUSETTINGS1:
		case MENUBATCH:
		case MENUSCHEDULE:
		case MENUALARM:
		case MENUWATER:
		case MENUSETTINGS2:
		case MENUAUXINFO:
#ifdef EnableGPSRate
		case MENUGPSRATE:
#endif
		case FEATURES:
		case PROP_CTRL:
		case REMOTE:
		case MENUCLOCK:
		case MENUPID:
		case MENUTOTALS:
		case MENUDIAGNOSTIC2:
		case MENUCELL:
		case MENUCELL2:
		case MENUINFO:
		case CAL420:
		case ASSEMBLYTEST:

			if (EditMode == False) {
				// we are scrolling DOWN through menu items
				CurrentRow++;
				while ((CurrentKeys[CurrentRow].KeyType != KEY)
						&& ((CurrentKeys[CurrentRow].KeyType != KEY_END)))
					CurrentRow++;

				if (CurrentKeys[CurrentRow].KeyType == KEY_END) {
					CurrentRow = 0;
					// let's find the first row used from the top down
					while (CurrentKeys[CurrentRow].KeyType != KEY)
						CurrentRow++;
				}
				lcdSetCursor((uint8_t) CurrentRow, 0);
			} else {
				// we are editing system variables
				switch (g_CurrentMenu) {
				case MENUCHEMICAL:
				case MENURATE:
					//case MENUSETTINGS1:
				case MENUBATCH:
					//case MENUSCHEDULE:
				case MENUALARM:
				case MENUWATER:
				case MENUSETTINGS2:
				case MENUAUXINFO:
#ifdef EnableGPSRate
				case MENUGPSRATE:
#endif
				case FEATURES:
				case PROP_CTRL:
				case REMOTE:
				case MENUCLOCK:
				case MENUPID:
				case MENUTOTALS:
				case MENUDIAGNOSTIC2:
				case MENUCELL:
				case MENUCELL2:
				case MENUINFO:
				case CAL420:
				case ASSEMBLYTEST:

					if (CurrentKeys[CurrentRow].KeyType == KEY) {
						// we need to decrease value																
						CurrentGadget =
								(Gadgets *) CurrentKeys[CurrentRow].Value;

						Precision = GetPrecision((uint8_t) CurrentRow,
								(uint8_t) CurrentColumn);
						ptr = TempString;

						switch (CurrentGadget->subtype) {
						case TYPE_INT_8:
						case TYPE_UINT_8:
							pointer_eight = (uint8_t *) CurrentGadget->Value;

							if (((*pointer_eight - Precision) > Precision)
									&& ((*pointer_eight - Precision)
											> (uint8_t) CurrentGadget->MinValue)
									&& (*pointer_eight != 0)) {
								*pointer_eight = *pointer_eight
										- (uint8_t) Precision;
							} else if (((*pointer_eight - Precision)
									> (uint8_t) CurrentGadget->MinValue)
									&& (*pointer_eight != 0)) {
								*pointer_eight = *pointer_eight
										- (uint8_t) Precision;

								while (*pointer_eight < Precision) {
									MoveColumnRight();
									Precision = GetPrecision(
											(uint8_t) CurrentRow,
											(uint8_t) CurrentColumn);
								}
							} else {
								*pointer_eight =
										(uint8_t) CurrentGadget->MinValue;
								CurrentColumn =
										CurrentKeys[CurrentRow].RightColumn;
								lcdSetCursor((uint8_t) CurrentRow,
										(uint8_t) CurrentColumn);
							}
							convItoA(ptr, (uint8_t) *pointer_eight,
									(int8_t *) CurrentGadget->Format_Or_Num);
							break;

						case TYPE_INT_16:
						case TYPE_UINT_16:
							pointer_sixteen = (uint16_t *) CurrentGadget->Value;
							if (((*pointer_sixteen - Precision) > Precision)
									&& ((*pointer_sixteen - Precision)
											> (uint16_t) CurrentGadget->MinValue)
									&& (*pointer_sixteen != 0)) {
								*pointer_sixteen = *pointer_sixteen
										- (uint16_t) Precision;
							} else if (((*pointer_sixteen - Precision)
									> (uint16_t) CurrentGadget->MinValue)
									&& (*pointer_sixteen != 0)) {

								*pointer_sixteen = *pointer_sixteen
										- (uint16_t) Precision;

								while (*pointer_sixteen < Precision) {
									MoveColumnRight();
									Precision = GetPrecision(
											(uint8_t) CurrentRow,
											(uint8_t) CurrentColumn);
								}
							} else {
								*pointer_sixteen =
										(uint16_t) CurrentGadget->MinValue;
								CurrentColumn =
										CurrentKeys[CurrentRow].RightColumn;
								lcdSetCursor((uint8_t) CurrentRow,
										(uint8_t) CurrentColumn);
							}
							convItoA(ptr, (uint16_t) *pointer_sixteen,
									(int8_t *) CurrentGadget->Format_Or_Num);
							break;

						case TYPE_INT_32:
							break;
#ifdef ChemCalNumber_Bigger
						case TYPE_UINT_32:
							pointer_thirtytwo = (uint32_t *) CurrentGadget->Value;
							if (((*pointer_thirtytwo - Precision) > Precision)
									&& ((*pointer_thirtytwo - Precision)
											> CurrentGadget->MinValue)
									&& (*pointer_thirtytwo != 0)) {
								*pointer_thirtytwo = *pointer_thirtytwo
										- Precision;
							} else if (((*pointer_thirtytwo - Precision)
									> CurrentGadget->MinValue)
									&& (*pointer_thirtytwo != 0)) {

								*pointer_thirtytwo = *pointer_thirtytwo
										- (uint32_t) Precision;

								while (*pointer_thirtytwo < Precision) {
									MoveColumnRight();
									Precision = GetPrecision(
											(uint8_t) CurrentRow,
											(uint8_t) CurrentColumn);
								}
							} else {
								*pointer_thirtytwo =
										CurrentGadget->MinValue;
								CurrentColumn =
										CurrentKeys[CurrentRow].RightColumn;
								lcdSetCursor((uint8_t) CurrentRow,
										(uint8_t) CurrentColumn);
							}
							convItoA(ptr, (int32_t) *pointer_thirtytwo,
									(int8_t *) CurrentGadget->Format_Or_Num);
							break;
#endif
						case TYPE_PHONE:
							pointer_eight = (uint8_t *) CurrentGadget->Value
									+ CurrentColumn
									- CurrentKeys[CurrentRow].LeftColumn;

							if (*pointer_eight != '0') {
								*pointer_eight = *pointer_eight - 1;
							}
							break;

						case TYPE_DECIMAL:

							ptr = DecimalString;
							pointer_thirtytwo =
									(uint32_t *) CurrentGadget->Value;
							thirtytwo =
									(uint32_t) *((uint32_t *) CurrentGadget->Value);

							data = lcdReadChar(CurrentColumn, CurrentRow) + 32;
							if (data != '9') {
								if (((uint32_t) thirtytwo + (uint32_t) Precision)
										<= (uint32_t) CurrentGadget->MaxValue)
									thirtytwo = thirtytwo + Precision;
							}
							convItoA(ptr, (int32_t) thirtytwo,
									(int8_t *) CurrentGadget->Format_Or_Num);
							*((uint32_t *) CurrentGadget->Value) =
									(uint32_t) thirtytwo;

							// fill in the leading spaces with zero's
							DecimalValue = 0;
							while (DecimalValue < 10) {
								if (*ptr == ' ')
									*ptr = '0';
								DecimalValue++;
								ptr++;
							}
							*ptr = 0x0;

							ptr = DecimalString; //Reset Back to Start

							break;

						case TYPE_STRING:
						case TYPE_SINGLESTRING:
							pointer_eight =
									(uint8_t *) CurrentGadget->Format_Or_Num;

							if (*pointer_eight
									== (uint8_t) CurrentGadget->MinValue)
								*pointer_eight =
										(uint8_t) CurrentGadget->MaxValue;
							else
								*pointer_eight = *pointer_eight - 1;

							if (CurrentGadget->subtype == TYPE_STRING) {
								ptr = (int8_t *) CurrentGadget->Value;

								num = 0;
								while (num != *pointer_eight) {
									ptr++;
									if (*ptr == 0x0) {
										num++;
										ptr++;
									}
								}
							}
							break;
						default:

							break;
						}
						lcdDrawString(CurrentGadget->Coords.x,
								CurrentGadget->Coords.y, (int8_t *) ptr);
					}
					break;
				case MENUSCHEDULE:
					// This is Key Down movement
					// we need to decrease integer value
					if (CurrentKeys[CurrentRow].KeyType == KEY) {
						// hours and minutes are on same row 
						if ((CurrentColumn == 14) || (CurrentColumn == 15))
							CurrentGadget =
									(Gadgets *) (CurrentKeys[CurrentRow].Value);
						else if ((CurrentColumn == 11) || (CurrentColumn == 12))
							if (CurrentRow == 2)
								CurrentGadget =
										(Gadgets *) CurrentKeys[9].Value;
							else if (CurrentRow == 4)
								CurrentGadget =
										(Gadgets *) CurrentKeys[10].Value;
						switch (CurrentGadget->subtype) {
#ifdef CodeChange_1_10_2022
						case TYPE_HR_MIN:
							
#else
						case TYPE_INT_8:
						case TYPE_UINT_8:
#endif
							ptr = TempString;
							pointer_eight = (uint8_t *) CurrentGadget->Value;
							if ((CurrentColumn == 11)
									|| (CurrentColumn == 14)) {
								// 10s place of hour or minute
								//Precision = 10;
								//if(*pointer_eight > (Precision - Precision / 10)){
								if (*pointer_eight > 9) {
									//if((*pointer_eight - Precision)	> (uint8_t) CurrentGadget->MinValue){
									if ((*pointer_eight - 10)
											> (uint8_t) CurrentGadget->MinValue) {
										//*pointer_eight = *pointer_eight - (uint8_t)Precision;
										*pointer_eight = *pointer_eight - 10;
									} else {
										*pointer_eight =
												(uint8_t) CurrentGadget->MinValue;
									}
								}
							} else if ((CurrentColumn == 12)
									|| (CurrentColumn == 15)) {
								// 1s place of hour or minute
								/*Precision = 1;
								 if((*pointer_eight - Precision)	> (uint8_t) CurrentGadget->MinValue){
								 *pointer_eight = *pointer_eight - (uint8_t)Precision;
								 }else{
								 *pointer_eight = (uint8_t) CurrentGadget->MinValue;
								 }*/
								if (*pointer_eight
										!= (uint8_t) CurrentGadget->MinValue) {
									if ((*pointer_eight - 1)
											> (uint8_t) CurrentGadget->MinValue) {
										*pointer_eight = *pointer_eight - 1;
									} else {
										*pointer_eight =
												(uint8_t) CurrentGadget->MinValue;
									}
								}
							}
							convItoA(ptr, (uint8_t) *pointer_eight,
									(int8_t *) CurrentGadget->Format_Or_Num);
							if (TempString[0] == ' ')
								TempString[0] = '0';
							break;
						case TYPE_STRING:
						case TYPE_SINGLESTRING:
							pointer_eight =
									(uint8_t *) CurrentGadget->Format_Or_Num;

							if (*pointer_eight
									== (uint8_t) CurrentGadget->MinValue)
								*pointer_eight =
										(uint8_t) CurrentGadget->MaxValue;
							else
								*pointer_eight = *pointer_eight - 1;

							if (CurrentGadget->subtype == TYPE_STRING) {
								ptr = (int8_t *) CurrentGadget->Value;

								num = 0;
								while (num != *pointer_eight) {
									ptr++;
									if (*ptr == 0x0) {
										num++;
										ptr++;
									}
								}
							}
							break;
						}

						lcdDrawString(CurrentGadget->Coords.x,
								CurrentGadget->Coords.y, ptr);

					}
					break;
				default:

					break;
				}
			}
			break;
		case MENUDIAGNOSTIC1:
		case MENUMAIN:

			if (ScreenVariables.ButtonPressed == 2) {
				ScreenVariables.ButtonPressed = 0;
			} else {
				ScreenVariables.ButtonPressed = 2;
			}
			break;
		default:
			break;
		}

		break; // break of: case KeyDown

		// ####################################
		//
		//                          Key CANCEL
		// ####################################

	case KeyCancel:

		if (g_CurrentMenu == EEPROMERR || g_CurrentMenu > 199)
			break; //do no action if on EEPROMERR or in tesing screens, we want the OK button pushed.
#ifdef CodeChanged_10_7_2022
		// why writeTime is Cancel?
		if (g_CurrentMenu == MENUCLOCK)
			writeTimeReq = True;
#endif
		if (g_CurrentMenu != MENUDIAGNOSTIC1
				&& g_CurrentMenu != MENUDIAGNOSTIC2)
			g_CurrentMenu = MENUMAIN; //Dont exit_me Pump Screen

		stopPumpAndControl(CANCELKEY_STOP);

		if (g_CurrentMenu == MENUMAIN) {
			resetBatchPivotKill(); //Reset Delayed batch pivot kill when on front screen only
		}

		if (g_CurrentMenu == ASSEMBLYTEST) {
			ScreenVariables.AssemblyTestCmd = 0; //Kill test if on assembly test screen
		}

		lcdClearText();
		lcdClearGraphic();
		//lcdClearArea(30,0,64,64);

		ScreenDraw(g_CurrentMenu);

		EditMode = False;

		if (g_CurrentMenu != MENUDIAGNOSTIC2) {
			lcdCursor(0, 0);
			CurrentRow = 0;
			CurrentColumn = 0;
		}

		// Update in EEPROM any variables that have changed
		CheckAndUpdateEEPROM(False);
		break;
	} // End of: switch (event.parameter) {
} // End of: void ProcessKeyPressController(AEvent event) {

static uint32_t GetPrecision(uint8_t Row, uint8_t Column) {
	uint32_t PrecisionRow;
	uint8_t exit_me;
	uint8_t data;
	uint8_t i;

	PrecisionRow = 1;
	exit_me = 0;

	i = (uint8_t) CurrentKeys[Row].RightColumn;

	/*if(g_CurrentMenu==MENURATE)
	 {
	 if(Row>6)
	 {
	 if(Row==7)
	 {
	 Row=6;
	 }
	 if(Row==8)
	 {
	 Row=7;
	 }
	 }
	 else
	 {
	 i = CurrentKeys[Row].RightColumn;
	 }
	 }
	 else
	 {
	 i = CurrentKeys[Row].RightColumn;
	 }*/

	while (exit_me == 0) {
		if (i == Column)
			exit_me = 1;

		data = lcdReadChar(i, Row) + 32;

		if ((exit_me == 0) && (data != '.'))
			PrecisionRow *= 10;

		i--;
	}

	return (PrecisionRow);
}

static void MoveColumnRight(void) {
	uint8_t data;

	CurrentColumn++;

	data = lcdReadChar(CurrentColumn, CurrentRow) + 32;
	if (data == '.')
		CurrentColumn++;

	if (CurrentColumn > CurrentKeys[CurrentRow].RightColumn)
		CurrentColumn = CurrentKeys[CurrentRow].RightColumn;

	lcdSetCursor((uint8_t) CurrentRow, (uint8_t) CurrentColumn);
}

static void MoveColumnLeft(void) {
	uint8_t data;

	CurrentColumn--;

	data = lcdReadChar(CurrentColumn, CurrentRow) + 32;
	if (data == '.')
		CurrentColumn--;

	if (CurrentColumn < CurrentKeys[CurrentRow].LeftColumn)
		CurrentColumn = CurrentKeys[CurrentRow].LeftColumn;

	lcdSetCursor((uint8_t) CurrentRow, (uint8_t) CurrentColumn);
}

// This function it essentially like pressing okay after selecting "Reset" on reset totals.
// It allows the screen to refresh back to "      " on display and take it out of edit mode.

void totalResetUpdate(void) {
	EditMode = False;
	CurrentColumn = 0;
	lcdSetCursor((uint8_t) CurrentRow, 0);
	lcdCursor(1, 0);
	ScreenDraw(g_CurrentMenu);

}

void cursor(void) {
	lcdSetCursor((uint8_t) CurrentRow, (uint8_t) CurrentColumn);
}

void goToHome() {
	//This will bring it to first screen of basic menu
	if (g_CurrentMenu == MENURATE)
		FinishChemUnitsChange();			//see if L<->G conversion is needed
	g_CurrentMenu = NOMENU - 1;
	MoveScreen(0);
}

void ShowEEPROMError() {
	g_CurrentMenu = EEPROMERR - 1;
	MoveScreen(0);
}
void TestingMenu() {
	g_CurrentMenu = 200 - 1;
	MoveScreen(0);
}

void jumpToScreen(uint8_t NextScreen) {
	// jump directly to specified NextScreen

	lcdClearText();
	lcdClearGraphic();
	g_CurrentMenu = NextScreen;
	switch (g_CurrentMenu) {
	case PROP_CTRL:
		CurrentKeys = PropCtrlMenuKeys;
		//lcdCursor(1, 0); // Cursor On
		break;
	case MENUBATCH:
		CurrentKeys = BatchMenuKeys;
		//lcdCursor(1, 0); // Cursor On
		break;
	case REMOTE:
		CurrentKeys = RemoteMenuKeys;
		//lcdCursor(1, 0); // Cursor On
		break;
	case MENUSCHEDULE:
		CurrentKeys = ScheduleMenuKeys;
		//lcdCursor(1, 0); // Cursor On
		break;
	case MENUAUXINFO:
		CurrentKeys = AuxInfoMenuKeys;
		//lcdCursor(1, 0); // Cursor On
		break;
#ifdef EnableGPSRate
	case MENUGPSRATE:
		CurrentKeys = GPSRateMenuKeys;
		break;
#endif
	}

	CurrentRow = 0;
	CurrentColumn = 0;
	while (CurrentKeys[CurrentRow].KeyType != KEY)
		CurrentRow++;

	//lcdCursor(1, 0); // Cursor On
	//lcdSetCursor((uint8_t)CurrentRow, (uint8_t)CurrentColumn);
	ScreenDraw(g_CurrentMenu);
}
