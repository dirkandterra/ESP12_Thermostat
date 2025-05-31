/********************************************************************/
/*   Name: keys                                                     */
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/

#include "lcd.h"
#include "utils.h"
#include "DR_screens.h"
#include "DR_keys.h"

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

	//Menu screen increment
	if (!Left) {
		// navigate screen to the right
		if (g_CurrentMenu == (NOMENU-1))	//If the screen is for eeprom error, jump to main menu page.
			g_CurrentMenu = MENUMAIN;
		else {
			g_CurrentMenu++;
		}
	} else {
		// navigate screen to the left
		// Standard Screens
		if (g_CurrentMenu != MENUMAIN)
			g_CurrentMenu--;    //Main Screens
		else
			g_CurrentMenu = NOMENU - 1; //Wrap around

	} // End of: move screen right/left
	
	EditMode = False;

	if (g_CurrentMenu == NOMENU)
		g_CurrentMenu = MENUMAIN;

	lcdCursor(0, 0);	 //Turn Off Cursor

	switch (g_CurrentMenu) {
	case MENUMAIN:
		CurrentKeys = MainMenuKeys;
		lcdCursor(0, 0);	 //Turn Off Cursor
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
} // End of: void MoveScreen(uint8_t Left) {

void ProcessKeyPressController(uint8_t key) {
	uint8_t data;
	uint8_t * pointer_eight;
	uint16_t * pointer_sixteen;
	uint32_t * pointer_thirtytwo;
	uint32_t thirtytwo;
	int8_t TempString[17];
	int8_t DecimalString[5];
	const ScreenObj * CurrentScreen;
	uint32_t Precision;
	int8_t * ptr;
	uint8_t num;
	//uint16_t baudrate;
	uint8_t PhoneCharacter;
	uint8_t DecimalValue;
	//uint8_t atoiChar;
	uint16_t adjustedRow = 0;

	switch (key) {
	// ####################################
	//
	//  Key MENU
	// ####################################
	case KeyMenu:
		//This will bring it to first screen of basic menu
		goToHome();
		break;

	// ####################################
	//
	//                 Key OK
	//
	// ####################################
	case KeyOK:
		switch (g_CurrentMenu) {
		case MENUMAIN:
			// under KeyOK
			//if (EditMode)
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
			if ((g_CurrentMenu != MENUMAIN)) {
				EditMode = True;

				if (CurrentRow != 0) {
					CurrentColumn = CurrentKeys[CurrentRow].RightColumn;
					// flash cursor at correct position
					lcdCursor(1, 1);
					lcdSetCursor((uint8_t) CurrentRow, (uint8_t) CurrentColumn);
				}
			}

		}
		break; // End of KeyOK

		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//   Key LEFT	
		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%					
	case KeyLeft:
		switch (g_CurrentMenu) {
		case MENUMAIN:
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

				CurrentScreen = (ScreenObj *) CurrentKeys[CurrentRow].Value;
				if (CurrentScreen->type == TYPE_PHONE) {
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
		
		} // End of: switch (g_CurrentMenu) {

		break; // break of: case KeyLeft

		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		//                                 Key RIGHT		
		// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	case KeyRight:
		switch (g_CurrentMenu) {
		case MENUMAIN:
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
		case MENUMAIN:
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
				case MENUMAIN:
					if (CurrentKeys[CurrentRow].KeyType == KEY) {
						// we need to increase integer value																
						CurrentScreen =
								(ScreenObj *) CurrentKeys[CurrentRow].Value;

						Precision = GetPrecision((uint8_t) CurrentRow,
								(uint8_t) CurrentColumn);
						ptr = TempString;

						switch (CurrentScreen->type) {
						case TYPE_INT_8:
						case TYPE_UINT_8:
							pointer_eight = (uint8_t *) CurrentScreen->Value;

							if (((int16_t) *pointer_eight + Precision)
									< CurrentScreen->MaxValue) {
								if (((int16_t) *pointer_eight + Precision)
										/ (10 * Precision)
										!= (int16_t) *pointer_eight
												/ (10 * Precision))
									MoveColumnLeft();

								*pointer_eight = *pointer_eight
										+ (uint8_t) Precision;
							} else {

								*pointer_eight =
										(uint8_t) CurrentScreen->MaxValue;
							}
							convItoA(ptr, (uint8_t) *pointer_eight,
									(int8_t *) CurrentScreen->Format_Or_Num);
							break;

						case TYPE_INT_16:
						case TYPE_UINT_16:
							pointer_sixteen = (uint16_t *) CurrentScreen->Value;

							if ((*pointer_sixteen + Precision)
									< CurrentScreen->MaxValue) {
								if ((*pointer_sixteen + Precision)
										/ (10 * Precision)
										!= *pointer_sixteen / (10 * Precision))
									MoveColumnLeft();

								*pointer_sixteen = *pointer_sixteen
										+ (uint16_t) Precision;
							} else {
								*pointer_sixteen =
										(uint16_t) CurrentScreen->MaxValue;
							}
							convItoA(ptr, (uint16_t) *pointer_sixteen,
									(int8_t *) CurrentScreen->Format_Or_Num);
							break;

						case TYPE_INT_32:
							break;
#ifdef ChemCalNumber_Bigger
						case TYPE_UINT_32:
							pointer_thirtytwo = (uint32_t *) CurrentScreen->Value;

							if ((*pointer_thirtytwo + Precision)
									< CurrentScreen->MaxValue) {
								if ((*pointer_thirtytwo + Precision)
										/ (10 * Precision)
										!= *pointer_thirtytwo / (10 * Precision))
									MoveColumnLeft();

								*pointer_thirtytwo = *pointer_thirtytwo
										+ Precision;
							} else {
								*pointer_thirtytwo =
										CurrentScreen->MaxValue;
							}
							convItoA(ptr, (int32_t) *pointer_thirtytwo,
									(int8_t *) CurrentScreen->Format_Or_Num);
							break;
#endif

						case TYPE_PHONE:
							pointer_eight = (uint8_t *) CurrentScreen->Value
									+ CurrentColumn
									- CurrentKeys[CurrentRow].LeftColumn;

							if (*pointer_eight != '9') {
								*pointer_eight = *pointer_eight + 1;
							}
							//convItoA (ptr,(uint32_t)thirtytwo, (int8_t *)CurrentScreen->Format_Or_Num);																																																																																																	
							//*((uint32_t *)CurrentScreen->Value) = (uint32_t)thirtytwo;

							// fill in the leading spaces with zero's
							PhoneCharacter = 0;
							while (PhoneCharacter < 10) {
								if (*ptr == ' ')
									*ptr = '0';
								PhoneCharacter++;
								ptr++;
							}
							*ptr = 0x0;

							ptr = (uint8_t *) CurrentScreen->Value;

							break;

						case TYPE_DECIMAL:
							ptr = DecimalString;
							pointer_thirtytwo =
									(uint32_t *) CurrentScreen->Value;
							thirtytwo =
									(uint32_t) *((uint32_t *) CurrentScreen->Value);

							data = lcdReadChar(CurrentColumn, CurrentRow) + 32;
							if (data != '9') {
								if (((uint32_t) thirtytwo + (uint32_t) Precision)
										<= (uint32_t) CurrentScreen->MaxValue)
									thirtytwo = thirtytwo + Precision;
							}
							convItoA(ptr, (int32_t) thirtytwo,
									(int8_t *) CurrentScreen->Format_Or_Num);
							*((uint32_t *) CurrentScreen->Value) =
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
									(uint8_t *) CurrentScreen->Format_Or_Num;

							if (*pointer_eight
									== (uint8_t) CurrentScreen->MaxValue)
								*pointer_eight =
										(uint8_t) CurrentScreen->MinValue;
							else
								*pointer_eight = *pointer_eight + 1;

							if (CurrentScreen->type == TYPE_STRING) {
								ptr = (int8_t *) CurrentScreen->Value;

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
						}	//CurrentScreen->type
						lcdDrawString(CurrentScreen->Coords.x,
								CurrentScreen->Coords.y, ptr);
					} // if CurrentKeys[CurrentRow].KeyType == KEY														
					break;
				default:
					break;
				} //switch (g_CurrentMenu) (2)																	
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
		case MENUMAIN:
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
				case MENUMAIN:
					if (CurrentKeys[CurrentRow].KeyType == KEY) {
						// we need to decrease value																
						CurrentScreen =
								(ScreenObj *) CurrentKeys[CurrentRow].Value;

						Precision = GetPrecision((uint8_t) CurrentRow,
								(uint8_t) CurrentColumn);
						ptr = TempString;

						switch (CurrentScreen->type) {
						case TYPE_INT_8:
						case TYPE_UINT_8:
							pointer_eight = (uint8_t *) CurrentScreen->Value;

							if (((*pointer_eight - Precision) > Precision)
									&& ((*pointer_eight - Precision)
											> (uint8_t) CurrentScreen->MinValue)
									&& (*pointer_eight != 0)) {
								*pointer_eight = *pointer_eight
										- (uint8_t) Precision;
							} else if (((*pointer_eight - Precision)
									> (uint8_t) CurrentScreen->MinValue)
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
										(uint8_t) CurrentScreen->MinValue;
								CurrentColumn =
										CurrentKeys[CurrentRow].RightColumn;
								lcdSetCursor((uint8_t) CurrentRow,
										(uint8_t) CurrentColumn);
							}
							convItoA(ptr, (uint8_t) *pointer_eight,
									(int8_t *) CurrentScreen->Format_Or_Num);
							break;

						case TYPE_INT_16:
						case TYPE_UINT_16:
							pointer_sixteen = (uint16_t *) CurrentScreen->Value;
							if (((*pointer_sixteen - Precision) > Precision)
									&& ((*pointer_sixteen - Precision)
											> (uint16_t) CurrentScreen->MinValue)
									&& (*pointer_sixteen != 0)) {
								*pointer_sixteen = *pointer_sixteen
										- (uint16_t) Precision;
							} else if (((*pointer_sixteen - Precision)
									> (uint16_t) CurrentScreen->MinValue)
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
										(uint16_t) CurrentScreen->MinValue;
								CurrentColumn =
										CurrentKeys[CurrentRow].RightColumn;
								lcdSetCursor((uint8_t) CurrentRow,
										(uint8_t) CurrentColumn);
							}
							convItoA(ptr, (uint16_t) *pointer_sixteen,
									(int8_t *) CurrentScreen->Format_Or_Num);
							break;

						case TYPE_INT_32:
							break;
#ifdef ChemCalNumber_Bigger
						case TYPE_UINT_32:
							pointer_thirtytwo = (uint32_t *) CurrentScreen->Value;
							if (((*pointer_thirtytwo - Precision) > Precision)
									&& ((*pointer_thirtytwo - Precision)
											> CurrentScreen->MinValue)
									&& (*pointer_thirtytwo != 0)) {
								*pointer_thirtytwo = *pointer_thirtytwo
										- Precision;
							} else if (((*pointer_thirtytwo - Precision)
									> CurrentScreen->MinValue)
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
										CurrentScreen->MinValue;
								CurrentColumn =
										CurrentKeys[CurrentRow].RightColumn;
								lcdSetCursor((uint8_t) CurrentRow,
										(uint8_t) CurrentColumn);
							}
							convItoA(ptr, (int32_t) *pointer_thirtytwo,
									(int8_t *) CurrentScreen->Format_Or_Num);
							break;
#endif
						case TYPE_PHONE:
							pointer_eight = (uint8_t *) CurrentScreen->Value
									+ CurrentColumn
									- CurrentKeys[CurrentRow].LeftColumn;

							if (*pointer_eight != '0') {
								*pointer_eight = *pointer_eight - 1;
							}
							break;

						case TYPE_DECIMAL:

							ptr = DecimalString;
							pointer_thirtytwo =
									(uint32_t *) CurrentScreen->Value;
							thirtytwo =
									(uint32_t) *((uint32_t *) CurrentScreen->Value);

							data = lcdReadChar(CurrentColumn, CurrentRow) + 32;
							if (data != '9') {
								if (((uint32_t) thirtytwo + (uint32_t) Precision)
										<= (uint32_t) CurrentScreen->MaxValue)
									thirtytwo = thirtytwo + Precision;
							}
							convItoA(ptr, (int32_t) thirtytwo,
									(int8_t *) CurrentScreen->Format_Or_Num);
							*((uint32_t *) CurrentScreen->Value) =
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
									(uint8_t *) CurrentScreen->Format_Or_Num;

							if (*pointer_eight
									== (uint8_t) CurrentScreen->MinValue)
								*pointer_eight =
										(uint8_t) CurrentScreen->MaxValue;
							else
								*pointer_eight = *pointer_eight - 1;

							if (CurrentScreen->type == TYPE_STRING) {
								ptr = (int8_t *) CurrentScreen->Value;

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
						lcdDrawString(CurrentScreen->Coords.x,
								CurrentScreen->Coords.y, (int8_t *) ptr);
					}
					break;
				default:

					break;
				}
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

		lcdClearText();
		lcdClearGraphic();
		//lcdClearArea(30,0,64,64);

		ScreenDraw(g_CurrentMenu);

		EditMode = False;

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
	g_CurrentMenu = NOMENU - 1;
	MoveScreen(0);
}

void jumpToScreen(uint8_t NextScreen) {
	// jump directly to specified NextScreen

	lcdClearText();
	lcdClearGraphic();
	g_CurrentMenu = NextScreen;
	switch (g_CurrentMenu) {
	case MENUMAIN:
		CurrentKeys = MainMenuKeys;
		//lcdCursor(1, 0); // Cursor On
		break;
	}

	CurrentRow = 0;
	CurrentColumn = 0;
	while (CurrentKeys[CurrentRow].KeyType != KEY)
		CurrentRow++;

	//lcdCursor(1, 0); // Cursor On
	//lcdSetCursor((uint8_t)CurrentRow, (uint8_t)CurrentColumn);
	ScreenDraw(g_CurrentMenu);
}
