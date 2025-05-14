/********************************************************************/
/*   Name: lcd                                                      */
/*------------------------------------------------------------------*/
/* Brief description: LCD routines interfacing to T6963c controller */
/* File name:         	lcd.c                                       */
/* Release              0.1                                         */
/*- Description: ---------------------------------------------------*/
/*  Modified routines from Radoslaw Kwiecien                        */
/*  //http://en.radzio.dxp.pl/t6963                                 */
/*- History: -------------------------------------------------------*/
/*  0.1  15/09/08  DB   Original                                    */
/*------------------------------------------------------------------*/

//#include "core.h"
//#include "queue.h"
//#include "hw.h"
#include <Arduino.h>
#include "Wire.h"
#include "lcd.h"
#define pcfAddress 0x20
//#include "splash.h"


static void GLCD_TextGoTo(uint8_t, uint8_t);
static void GLCD_WriteString(int8_t * str);
static uint8_t GLCD_CheckStatus(void);
static void GLCD_WriteCommand(uint8_t);
static void GLCD_WriteData(uint8_t);
static uint8_t GLCD_ReadData(void);
static void GLCD_ClearCG(void);
static void GLCD_WriteChar(int8_t ch);
static void GLCD_DefineCharacter(uint8_t, uint8_t *);
static void GLCD_SetPixel(uint8_t, uint8_t, uint8_t);
static void GLCD_WriteDisplayData(uint8_t);
uint8_t GetData(void);

static void MED_DrawString(int8_t * String, int16_t xpos, int16_t ypos,
		bool invert);
static void LARGE_DrawString(int8_t * String, int16_t xpos, int16_t ypos,
		bool invert);
static void LARGE_PutChar(int8_t Char, int16_t xpos, int16_t ypos, bool invert);
static void MED_PutChar(int8_t Char, int16_t xpos, int16_t ypos, bool invert);

void SetData(uint8_t data_out);

uint16_t PCF8575_DataDir = 0xFF0F;
uint16_t PCF8575_DataCurrent = PCF8575_DataDir;
uint16_t lastRead=0x00;

const unsigned char color = 1;

#define REVERSEBITS(b) (BitReverseTable[b])

const unsigned char BitReverseTable[256] = { 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0,
		0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0, 0x08, 0x88,
		0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8,
		0x78, 0xf8, 0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94,
		0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4, 0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac,
		0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc, 0x02, 0x82,
		0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2,
		0x72, 0xf2, 0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a,
		0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa, 0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6,
		0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6, 0x0e, 0x8e,
		0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe,
		0x7e, 0xfe, 0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91,
		0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1, 0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9,
		0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 0x05, 0x85,
		0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5,
		0x75, 0xf5, 0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d,
		0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd, 0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3,
		0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3, 0x0b, 0x8b,
		0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb,
		0x7b, 0xfb, 0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97,
		0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7, 0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf,
		0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff };

void printThis(uint8_t that){
  Serial.println(that);
  delay(100);
}

uint16_t read16()
{
  uint16_t _dataIn = 0;
  if (Wire.requestFrom((uint8_t)pcfAddress, (uint8_t)2) != 2)
  {
    return lastRead;                   //  last value
  }
  _dataIn = Wire.read();            //  low 8 bits
  _dataIn |= (Wire.read() << 8);    //  high 8 bits
  lastRead=_dataIn;
#ifdef DEBUGPRINT
  Serial.print("Read: ");
  Serial.println(_dataIn,HEX);
#endif
  return _dataIn;
}

uint8_t readKey(uint8_t channel){
  switch(channel){
    case 0:
    digitalWrite(12,0);
    break;
    default:
    digitalWrite(12,1);
    break;
  }
  return (uint8_t)(read16()&0x000F);
}

bool write16(const uint16_t value)
{
  uint16_t dataOut = value;
  Wire.beginTransmission(pcfAddress);
  Wire.write(dataOut & 0xFF);      //  low 8 bits
  Wire.write(dataOut >> 8);        //  high 8 bits
#ifdef DEBUGPRINT
  Serial.println(dataOut,HEX);
#endif
  return Wire.endTransmission();
}

void SetDataDir(uint8_t in) {
	//set data lines for in or out
  if(in){
    PCF8575_DataDir |= 0xFF00; //PCF8575_DataDir |= 0x00FF;
#ifdef DEBUGPRINT
    Serial.print("DataDir In: ");
#endif
  }else{
    PCF8575_DataDir &= 0x00FF; //PCF8575_DataDir &= 0xFF00;
#ifdef DEBUGPRINT
    Serial.print("DataDir Out: ");
#endif
  }
  PCF8575_DataCurrent |= PCF8575_DataDir;
#ifdef DEBUGPRINT
  Serial.print(PCF8575_DataDir,HEX);
  Serial.print("->");
#endif
  write16(PCF8575_DataCurrent);

} 

uint8_t GetData(void) {

  uint16_t readData=0;
	uint8_t read = 0;
  read=(uint8_t)((read16()>>8) & 0x00FF);  //read=(uint8_t)(read16() & 0x00FF);

	return read;

} // End of: uint8_t GetData(void) {

void SetData(uint8_t data_out) {
#ifdef DEBUGPRINT
  Serial.print(data_out,HEX);
  Serial.print("->");
#endif
  PCF8575_DataCurrent &= 0x00FF; //PCF8575_DataCurrent &= 0xFF00;
  PCF8575_DataCurrent |= (data_out<<8); //PCF8575_DataCurrent |= data_out;
  write16(PCF8575_DataCurrent);
	
} // End of: void SetData(uint8_t data_out) {

// ################################################################################################
// ################################################################################################
//
//         LOW LEVEL OPERATIONS
//
// ################################################################################################

// ################################################################################################
//SetData
// Delay function
//	
// ################################################################################################
static void delayASM(void) {
	volatile uint16_t i;
	for (i = 0; i < (XTALFREQ); i++) {
		__asm__("nop\n\t");
		__asm__("nop\n\t");
		__asm__("nop\n\t");
	}
}

// ################################################################################################
//
// Reads dispay status
//
// ################################################################################################
static unsigned char GLCD_CheckStatus(void) {
  //return 0x03;
	uint8_t status;
	//GLCD_DATA_DDR = 0x00;  //Input
  GLCD_RD_0;  //moved above
	SetDataDir(1);

	//GLCD_CTRL_PORT &= ~((1 << GLCD_RD) | (1 << GLCD_CE));

	GLCD_CE_0;
	delayASM();
	status = GetData();

	GLCD_CE_1;

	//GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE));
	GLCD_RD_1;

	//GLCD_DATA_DDR = 0xFF; //Output
	SetDataDir(0);
#ifdef DEBUGPRINT
  Serial.print("Status: ");
  Serial.println(status);
#endif
	return status;
} // End of: static unsigned char GLCD_CheckStatus(void) {

// ################################################################################################
//
// Writes instruction 
//
// ################################################################################################GLCD_WriteCommand
static void GLCD_WriteCommand(uint8_t command) {

	while (!(GLCD_CheckStatus() & 0x03));

	//GLCD_DATA_DDR = 0xFF;  //Output
	//SetDataDir(0);

	//GLCD_DATA_PORT = REVERSEBITS(command);
  GLCD_WR_0;  //Moved above
#ifdef DEBUGPRINT
  Serial.print("Write Cmd: ");
#endif
	SetData((command));

	//GLCD_CTRL_PORT &= ~((1 << GLCD_WR) | (1 << GLCD_CE));

	GLCD_CE_0;
  
	delayASM();
	GLCD_CE_1;
	//GLCD_CTRL_PORT |= ((1 << GLCD_WR) | (1 << GLCD_CE));
	GLCD_WR_1;

}

// ################################################################################################
//
// Writes data
//
// ################################################################################################

static void GLCD_WriteData(uint8_t data) {
	while (!(GLCD_CheckStatus() & 0x03));

	//GLCD_DATA_DDR = 0xFF;  //Output
	//SetDataDir(0);GLCD_CheckStatus

  GLCD_WR_0;  //Moved above 
	GLCD_CD_0;  //Moved above

	//GLCD_DATA_PORT = (data);
#ifdef DEBUGPRINT
  Serial.print("Write Data: ");
#endif
	SetData((data));

	//GLCD_CTRL_PORT &= ~((1 << GLCD_WR) | (1 << GLCD_CE) | (1 << GLCD_CD));

	GLCD_CE_0;
	delayASM();
	GLCD_CE_1;

	//GLCD_CTRL_PORT |= ((1 << GLCD_WR) | (1 << GLCD_CE) | (1 << GLCD_CD));
	GLCD_WR_1;
	GLCD_CD_1;

}

// ################################################################################################
//
// Reads data
//
// ################################################################################################
static unsigned char GLCD_ReadData(void) {
	uint8_t read;
	while (!(GLCD_CheckStatus() & 0x03));

	GLCD_RD_0;  //Moved above
	GLCD_CD_0; //Moved above

	//GLCD_DATA_DDR = 0x00; //Input
	SetDataDir(1);

	//GLCD_CTRL_PORT &= ~((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));

	GLCD_CE_0;
	delayASM();

	//read = REVERSEBITS(GLCD_DATA_PORT);
	read = GetData();

	GLCD_CE_1;

	//GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
	GLCD_RD_1;
	GLCD_CD_1;

	//GLCD_DATA_DDR = 0xFF;  //Output
	SetDataDir(0);

	return read;
}

//-------------------------------------------------------------------------------------------------
//
// Sets address pointer for display RAM memory
//
//-------------------------------------------------------------------------------------------------
static void GLCD_SetAddressPointer(uint16_t address) {
	GLCD_WriteData((uint8_t)(address & 0xFF));
	GLCD_WriteData((uint8_t)(address >> 8));
	GLCD_WriteCommand(T6963_SET_ADDRESS_POINTER);
}

//-------------------------------------------------------------------------------------------------
//
// Clears text area of display RAM memory
//
//-------------------------------------------------------------------------------------------------
void lcdClearText(void) {
	int16_t i;
	GLCD_SetAddressPointer(GLCD_TEXT_HOME);

	for (i = 0; i < GLCD_TEXT_SIZE; i++) {
		GLCD_WriteDisplayData(0);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Clears characters generator area of display RAM memory
//
//-------------------------------------------------------------------------------------------------
static void GLCD_ClearCG(void) {
	uint16_t i;
	GLCD_SetAddressPointer(GLCD_EXTERNAL_CG_HOME);

	for (i = 0; i < 256 * 8; i++) {
		GLCD_WriteDisplayData(0);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Clears graphics area of display RAM memory
//
//-------------------------------------------------------------------------------------------------
void lcdClearGraphic(void) {
	int16_t i;
	GLCD_SetAddressPointer(GLCD_GRAPHIC_HOME);
	for (i = 0; i < GLCD_GRAPHIC_SIZE; i++) {
		GLCD_WriteDisplayData(0x00);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Writes a single character (ASCII code) to display RAM memory
//
//-------------------------------------------------------------------------------------------------
static void GLCD_WriteChar(int8_t charCode) {
	GLCD_WriteDisplayData((uint8_t)(charCode - 32));
}

//-------------------------------------------------------------------------------------------------
//
// Writes null-terminated string to display RAM memory
//
//-------------------------------------------------------------------------------------------------
static void GLCD_WriteString(int8_t * string) {
	while (*string) {
		GLCD_WriteChar(*string++);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Sets display coordinates
//
//-------------------------------------------------------------------------------------------------
static void GLCD_TextGoTo(uint8_t x, uint8_t y) {
	uint16_t address;

	address = (uint16_t)(GLCD_TEXT_HOME + x + (GLCD_TEXT_AREA * y));

	GLCD_SetAddressPointer(address);
}

//-------------------------------------------------------------------------------------------------
//
// Writes single char pattern to character generator area of display RAM memory
//
//-------------------------------------------------------------------------------------------------
static void GLCD_DefineCharacter(uint8_t charCode, uint8_t * defChar) {
	uint16_t address;
	uint8_t i;

	address = (uint16_t)(GLCD_EXTERNAL_CG_HOME + (8 * charCode));

	GLCD_SetAddressPointer(address);

	for (i = 0; i < 8; i++) {
		GLCD_WriteDisplayData(*(defChar + i));
	}
}

//-------------------------------------------------------------------------------------------------
//
// Set (if color==1) or clear (if color==0) pixel on screen
//
//-------------------------------------------------------------------------------------------------
static void GLCD_SetPixel(uint8_t x, uint8_t y, uint8_t mycolor) {
	uint8_t tmp;
	uint16_t address;

	address = (uint16_t)(GLCD_GRAPHIC_HOME + (x / GLCD_FONT_WIDTH)
			+ (GLCD_GRAPHIC_AREA * y));

	GLCD_SetAddressPointer(address);

	GLCD_WriteCommand(T6963_DATA_READ_AND_NONVARIABLE);
	tmp = GLCD_ReadData();

	if (mycolor)
		tmp |= (1 << (GLCD_FONT_WIDTH - 1 - (x % GLCD_FONT_WIDTH)));
	else
		tmp &= ~(1 << (GLCD_FONT_WIDTH - 1 - (x % GLCD_FONT_WIDTH)));

	GLCD_WriteDisplayData(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Writes display data and increment address pointer
//
//-------------------------------------------------------------------------------------------------
static void GLCD_WriteDisplayData(uint8_t x) {
	GLCD_WriteData(x);
	GLCD_WriteCommand(T6963_DATA_WRITE_AND_INCREMENT);
}

#ifdef NotUsed // SaveForHistory/Future 
//-------------------------------------------------------------------------------------------------
//
// Sets graphics coordinates
//
//-------------------------------------------------------------------------------------------------
static void GLCD_GraphicGoTo(uint8_t x, uint8_t y) {
	uint16_t address;
	address = GLCD_GRAPHIC_HOME + (x / GLCD_FONT_WIDTH)
			+ (GLCD_GRAPHIC_AREA * y);
	GLCD_SetAddressPointer(address);
}
#endif

//-------------------------------------------------------------------------------------------------
//
// Display initialization
//
//-------------------------------------------------------------------------------------------------
void LcdInit(uint32_t clockRate) {
  uint8_t i=0;
  Wire.begin();
  Wire.setClock(clockRate);
	/*
	 //Test LCD Wrtie Order
	 //GLCD_DATA_DDR = 0xFF;  //Output
	 SetDataDir (1);   //Read
	 SetData(255);
	 SetData(0);
	 
	 SetDataDir (0);    //Write
	 SetData(255);
	 SetData(0);
	 
	 
	 //GLCD_DATA_PORT = REVERSEBITS(command);
	 SetData(1);
	 SetData(2);
	 SetData(4);
	 SetData(8);
	 SetData(16);
	 SetData(32);
	 SetData(64);
	 SetData(128);
	 SetData(255);
	 SetData(0);
	 

	 */
	 delay(500);
	 	// LCD control signals
  pinMode(CE_Pin,OUTPUT);
  pinMode(12,OUTPUT);
  GLCD_CE_1;
  delay(200);
	GLCD_RS_0; //Reset LCD  
  delay(200);
	GLCD_WR_1;
	GLCD_RD_1;
	GLCD_CE_1;
	GLCD_CD_1;
  GLCD_RS_1; //Release Reset off LCD
	SetDataDir(0);
  Serial.println("Write Init01: ");
	SetData(0);
  delay(200);
	GLCD_WriteCommand(0xB2);
	GLCD_WriteCommand(0xB2);
	GLCD_WriteCommand(0xB2);
	GLCD_WriteCommand(0xB2);
	GLCD_WriteCommand(0x90);
	GLCD_WriteCommand(0x90);
	GLCD_WriteCommand(0x90);
	GLCD_WriteCommand(0x90);

	GLCD_WriteData(GLCD_GRAPHIC_HOME & 0xFF);
	GLCD_WriteData(GLCD_GRAPHIC_HOME >> 8);
	GLCD_WriteCommand(T6963_SET_GRAPHIC_HOME_ADDRESS);

	GLCD_WriteData(GLCD_GRAPHIC_AREA);
	GLCD_WriteData(0x00);
	GLCD_WriteCommand(T6963_SET_GRAPHIC_AREA);

	GLCD_WriteData(GLCD_TEXT_HOME);
	GLCD_WriteData(GLCD_TEXT_HOME >> 8);
	GLCD_WriteCommand(T6963_SET_TEXT_HOME_ADDRESS);

	GLCD_WriteData(GLCD_TEXT_AREA);
	GLCD_WriteData(0x00);
	GLCD_WriteCommand(T6963_SET_TEXT_AREA);

	GLCD_WriteData(GLCD_OFFSET_REGISTER);
	GLCD_WriteData(0x00);
	GLCD_WriteCommand(T6963_SET_OFFSET_REGISTER);

	GLCD_WriteCommand(
			T6963_DISPLAY_MODE | T6963_GRAPHIC_DISPLAY_ON | T6963_CURSOR_DISPLAY_ON | T6963_CURSOR_BLINK_ON
					| T6963_TEXT_DISPLAY_ON);

	GLCD_WriteCommand(T6963_MODE_SET | 0);
  GLCD_WriteData(0x00);
  GLCD_WriteData(0x00);
  GLCD_WriteCommand(0x24);
  
  for(i=0x00; i<0x78; i++){
    GLCD_WriteData(i);
    GLCD_WriteCommand(0xC0);
  }
lcdClearGraphic();

  delay(2000);
  Serial.println("DoneInit2");
	lcdClearText();
Serial.println("DoneInit3");
	GLCD_ClearCG();
Serial.println("DoneInit4");
	lcdClearGraphic();
  Serial.println("DoneInit");

  
}

// *****
// external routines to GUI
// *****

#ifdef NotUsed // SaveForHistory/Future 
void lcdClearBytesAbsolute(uint8_t x, uint8_t y, uint8_t numbytes) {
	//uint8_t tmp;
	uint16_t address;

	address = GLCD_GRAPHIC_HOME + (x / GLCD_FONT_WIDTH)
			+ (GLCD_GRAPHIC_AREA * y);

	GLCD_SetAddressPointer(address);

	GLCD_WriteCommand(T6963_DATA_READ_AND_NONVARIABLE);

	while (numbytes--)
		GLCD_WriteDisplayData(0x0);
}
#endif

void lcdWriteBytesAbsolute(uint8_t x, uint8_t y, const uint8_t * data,
		uint8_t numbytes, bool invert);

void lcdWriteBytesAbsolute(uint8_t x, uint8_t y, const uint8_t * data,
		uint8_t numbytes, bool invert) {
	uint16_t address;

	address = (uint16_t)(GLCD_GRAPHIC_HOME + (x / GLCD_FONT_WIDTH)
			+ (GLCD_GRAPHIC_AREA * y));

	GLCD_SetAddressPointer(address);

	GLCD_WriteCommand(T6963_DATA_READ_AND_NONVARIABLE);

	while (numbytes--) {
		if (invert == True)
			GLCD_WriteDisplayData((uint8_t)~(*data++));
		else
			GLCD_WriteDisplayData(*data++);
	}
}

void lcdSetCursor(uint8_t row, uint8_t column) {
	GLCD_WriteData(column);
	GLCD_WriteData(row);
	GLCD_WriteCommand(T6963_SET_CURSOR_POINTER);
}

void lcdCursor(uint8_t OnOff, uint8_t flash) {
	if (OnOff == 0) {
		GLCD_WriteCommand(
				T6963_DISPLAY_MODE | T6963_GRAPHIC_DISPLAY_ON
						| T6963_TEXT_DISPLAY_ON);
		GLCD_WriteCommand(T6963_CURSOR_PATTERN_SELECT | T6963_CURSOR_8_LINE);
	} else {
		if (flash == 1)
			GLCD_WriteCommand(
					T6963_DISPLAY_MODE | T6963_GRAPHIC_DISPLAY_ON
							| T6963_TEXT_DISPLAY_ON | T6963_CURSOR_DISPLAY_ON
							| T6963_CURSOR_BLINK_ON);
		else
			GLCD_WriteCommand(
					T6963_DISPLAY_MODE | T6963_GRAPHIC_DISPLAY_ON
							| T6963_TEXT_DISPLAY_ON | T6963_CURSOR_DISPLAY_ON);

		GLCD_WriteCommand(T6963_CURSOR_PATTERN_SELECT | T6963_CURSOR_8_LINE);
	}
}

uint8_t lcdReadChar(int16_t x1, int16_t y1) {
	uint8_t data;

	GLCD_TextGoTo((uint8_t)x1, (uint8_t)y1);
	GLCD_WriteCommand(T6963_DATA_READ_AND_NONVARIABLE);
	data = GLCD_ReadData();

	return (data);
}

#ifdef NotUsed // SaveForHistory/Future 
void lcdWriteChar(int16_t x1, int16_t y1, uint8_t data) {
	GLCD_TextGoTo(x1, y1);
	GLCD_WriteChar(data);
}
#endif

void lcdDrawString(int16_t x1, int16_t y1, int8_t *value) {
	GLCD_TextGoTo((uint8_t) x1, (uint8_t) y1);
	GLCD_WriteString(value);
}

#ifdef NotUsed // SaveForHistory/Future 
void lcdClearArea(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	uint8_t Xcounter;
	uint8_t Ycounter;

	if ((x1 <= x2) && (y1 <= y2) && (x2 <= GLCD_PIXELS_PER_LINE)
			&& (y2 <= GLCD_NUMBER_OF_LINES)) {
		Ycounter = y1;
		while (Ycounter <= y2) {
			Xcounter = x1;

			while (Xcounter <= x2) {
				GLCD_SetPixel(Xcounter, Ycounter, 0);
				Xcounter++;
			}
			Ycounter++;
		}
	}
}
#endif

/*
 void lcdInvertArea(uint16_t x,uint16_t y,uint16_t w,uint16_t h)
 {

 }
 */
#ifdef NotUsed // SaveForHistory/Future 
void lcdDrawDot(int16_t x1, int16_t y1) {
	GLCD_SetPixel(x1, y1, 1);
}
#endif

#ifdef NotUsed // SaveForHistory/Future 
void lcdClearDot(int16_t x1, int16_t y1) {
	GLCD_SetPixel(x1, y1, 0);
}
#endif

/*
 void lcdDrawBar(int16_t x1,int16_t y1,int16_t x2,int16_t y2, int16_t value)
 {

 }
 */

void lcdDrawBitmap(int16_t x1, int16_t y1, const ABitmap *bitmap) {
	uint16_t j;

	for (j = 0; j < bitmap->height; j++)
		lcdWriteBytesAbsolute((uint8_t)x1, (uint8_t)(y1 + j),
				(uint8_t *) (bitmap->data + j * bitmap->width), bitmap->width,
				False);
}

#ifdef NotUsed // SaveForHistory/Future 
void lcdDrawRectangle(uint8_t x, uint8_t y, uint8_t b, uint8_t a) {
	uint8_t j;

	for (j = 0; j < a; j++) {
		GLCD_SetPixel(x, y + j, color);
		GLCD_SetPixel(x + b - 1, y + j, color);
	}

	for (j = 0; j < b; j++) {
		GLCD_SetPixel(x + j, y, color);
		GLCD_SetPixel(x + j, y + a - 1, color);
	}
}
#endif

#ifdef NotUsed // SaveForHistory/Future 
void lcdDrawCircle(uint8_t cx, uint8_t cy, uint8_t radius) {
	int16_t x;
	int16_t y;
	int16_t xchange;
	int16_t ychange;
	int16_t radiusError;

	x = radius;
	y = 0;
	xchange = 1 - 2 * radius;
	ychange = 1;
	radiusError = 0;

	while (x >= y) {
		GLCD_SetPixel(cx + x, cy + y, color);
		GLCD_SetPixel(cx - x, cy + y, color);
		GLCD_SetPixel(cx - x, cy - y, color);
		GLCD_SetPixel(cx + x, cy - y, color);
		GLCD_SetPixel(cx + y, cy + x, color);
		GLCD_SetPixel(cx - y, cy + x, color);
		GLCD_SetPixel(cx - y, cy - x, color);
		GLCD_SetPixel(cx + y, cy - x, color);

		y++;

		radiusError += ychange;
		ychange += 2;

		if (2 * radiusError + xchange > 0) {
			x--;
			radiusError += xchange;
			xchange += 2;
		}
	}
}
#endif

#ifdef NotUsed // SaveForHistory/Future 
void lcdDrawLine(int16_t X1, int16_t Y1, int16_t X2, int16_t Y2) {
	int16_t CurrentX;
	int16_t CurrentY;
	int16_t Xinc;
	int16_t Yinc;
	int16_t Dx;
	int16_t Dy;
	int16_t TwoDx;
	int16_t TwoDy;
	int16_t TwoDxAccumulatedError;
	int16_t TwoDyAccumulatedError;

	Dx = (X2 - X1);
	Dy = (Y2 - Y1);

	TwoDx = Dx + Dx;
	TwoDy = Dy + Dy;

	CurrentX = X1;
	CurrentY = Y1;

	Xinc = 1;
	Yinc = 1;

	if (Dx < 0) {
		Xinc = -1;
		Dx = -Dx;
		TwoDx = -TwoDx;
	}

	if (Dy < 0) {
		Yinc = -1;
		Dy = -Dy;
		TwoDy = -TwoDy;
	}

	GLCD_SetPixel(X1, Y1, color);

	if ((Dx != 0) || (Dy != 0)) {
		if (Dy <= Dx) {
			TwoDxAccumulatedError = 0;
			do {
				CurrentX += Xinc;
				TwoDxAccumulatedError += TwoDy;
				if (TwoDxAccumulatedError > Dx) {
					CurrentY += Yinc;
					TwoDxAccumulatedError -= TwoDx;
				}
				GLCD_SetPixel(CurrentX, CurrentY, color);
			} while (CurrentX != X2);
		} else {
			TwoDyAccumulatedError = 0;
			do {
				CurrentY += Yinc;
				TwoDyAccumulatedError += TwoDx;
				if (TwoDyAccumulatedError > Dy) {
					CurrentX += Xinc;
					TwoDyAccumulatedError -= TwoDy;
				}
				GLCD_SetPixel(CurrentX, CurrentY, color);
			} while (CurrentY != Y2);
		}
	}
}
#endif

#ifdef NotUsed // SaveForHistory/Future 
// 10 x 8 font
static const uint8_t FontDataMedium[38][10] = { { 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x18, 0x18, }, // '.'
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, }, // ' '
		{ 		0x3e, // ..xxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x6b, // .xx.x.xx........
				0x6b, // .xx.x.xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   30H
		{ 		0x18, // ...xx...........
				0x38, // ..xxx...........
				0x38, // ..xxx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x3c, // ..xxxx..........
				0x3c, // ..xxxx..........
		},//   31H
		{ 		0x7e, // .xxxxxx.........
				0x7f, // .xxxxxxx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x3f, // ..xxxxxx........
				0x7e, // .xxxxxx.........
				0x60, // .xx.............
				0x60, // .xx.............
				0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
		},//   32H
		{ 		0x7e, // .xxxxxx.........
				0x7f, // .xxxxxxx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x1e, // ...xxxx........
				0x1e, // ...xxxx.........
				0x03, // ......xx........
				0x03, // ......xx........
				0x7f, // .xxxxxxx........
				0x7e, // .xxxxxx.........
		},//   33H
		{ 		0x60, // .xx.............
				0x60, // .xx.............
				0x66, // .xx..xx.........
				0x66, // .xx..xx.........
				0x66, // .xx..xx.........
				0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x06, // .....xx.........
				0x06, // .....xx.........
				0x06, // .....xx.........
		},//   34H
		{ 		0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x60, // .xx.............
				0x60, // .xx.............
				0x7e, // .xxxxxx.........
				0x7f, // .xxxxxxx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x7f, // .xxxxxxx........
				0x7e, // .xxxxxx.........
		},//   35H
		{ 		0x3e, // ..xxxxx.........
				0x7e, // .xxxxxx.........
				0x60, // .xx.............
				0x60, // .xx.............
				0x3e, // .xxxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   36H
		{ 		0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x07, // .....xxx........
				0x0e, // ....xxx.........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
		},//   37H
		{ 		0x3e, // ..xxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   38H
		{ 		0x3e, // ..xxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3f, // ..xxxxxx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x3f, // ..xxxxxx........
				0x3e, // ..xxxxx.........
		},//   39H
		{ 		0x3e, // ..xxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
		},//   41H
		{ 		0x7e, // .xxxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7e, // .xxxxxx.........
				0x7e, // .xxxxxx.........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x7e, // .xxxxxx.........
		},//   42H
		{ 		0x3f, // ..xxxxxx........
				0x7f, // .xxxxxxx........
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x7f, // .xxxxxxx........
				0x3f, // ..xxxxxx........
		},//   43H
		{ 		0x7c, // .xxxxx..........
				0x7e, // .xxxxxx.........
				0x67, // .xx..xxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x67, // .xx..xxx........
				0x7e, // .xxxxxx.........
				0x7c, // .xxxxx..........
		},//   44H
		{ 		0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x60, // .xx.............
				0x60, // .xx.............
				0x7c, // .xxxxx..........
				0x7c, // .xxxxx..........
				0x60, // .xx.............
				0x60, // .xx.............
				0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
		},//   45H
		{ 		0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x60, // .xx.............
				0x60, // .xx.............
				0x7c, // .xxxxx..........
				0x7c, // .xxxxx..........
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
		},//   46H
		{ 		0x3f, // ..xxxxxx........
				0x7f, // .xxxxxxx........
				0x60, // .xx.............
				0x60, // .xx.............
				0x6f, // .xx.xxxx........
				0x6f, // .xx.xxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   47H
		{ 		0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
		},//   48H
		{ 		0x3c, // ..xxxx..........
				0x3c, // ..xxxx..........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x3c, // ..xxxx..........
				0x3c, // ..xxxx..........
		},//   49H
		{ 		0x03, // ......xx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   4aH
		{ 		0x63, // .xx...xx........
				0x67, // .xx..xxx........
				0x6e, // .xx.xxx.........
				0x7c, // .xxxxx..........
				0x78, // .xxxx...........
				0x7c, // .xxxxx..........
				0x6e, // .xx.xxx.........
				0x67, // .xx..xxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
		},//   4bH
		{ 		0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
		},//   4cH
		{ 		0x63, // .xx...xx........
				0x77, // .xxx.xxx........
				0x77, // .xxx.xxx........
				0x7f, // .xxxxxxx........
				0x6b, // .xx.x.xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
		},//   4dH
		{ 		0x63, // .xx...xx........
				0x73, // .xxx..xx........
				0x73, // .xxx..xx........
				0x7b, // .xxxx.xx........
				0x6b, // .xx.x.xx........
				0x6f, // .xx.xxxx........
				0x67, // .xx..xxx........
				0x67, // .xx..xxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
		},//   4eH
		{ 		0x3e, // ..xxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   4fH
		{ 		0x7e, // .xxxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x7e, // .xxxxxx.........
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
				0x60, // .xx.............
		},//   50H
		{ 		0x3e, // ..xxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x6b, // .xx.x.xx........
				0x6f, // .xx.xxxx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   51H
		{ 		0x7e, // .xxxxxx.........
				0x7f, // .xxxxxxx........
				0x63, // .xx...xx........
				0x66, // .xx..xx.........
				0x78, // .xxxx...........
				0x7c, // .xxxx...........
				0x6c, // .xx.xx..........
				0x66, // .xx..xx.........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
		},//   52H
		{ 		0x3f, // ..xxxxxx........
				0x7f, // .xxxxxxx........
				0x60, // .xx.............
				0x60, // .xx.............
				0x7e, // .xxxxxx.........
				0x3f, // ..xxxxxx........
				0x03, // ......xx........
				0x03, // ......xx........
				0x7f, // .xxxxxxx........
				0x7e, // .xxxxxx.........
		},//   53H
		{ 		0x7e, // .xxxxxx.........
				0x7e, // .xxxxxx.........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
		},//   54H
		{ 		0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x7f, // .xxxxxxx........
				0x3e, // ..xxxxx.........
		},//   55H
		{ 		0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x77, // .xxx.xxx........
				0x3e, // ..xxxxx.........
				0x1c, // ...xxx..........
				0x08, // ....x...........
		},//   56H
		{ 		0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x6b, // .xx.x.xx........
				0x6b, // .xx.x.xx........
				0x7f, // .xxxxxxx........
				0x77, // .xxx.xxx........
				0x77, // .xxx.xxx........
				0x63, // .xx...xx........
		},//   57H
		{ 		0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x77, // .xxx.xxx........
				0x3e, // ..xxxxx.........
				0x1c, // ...xxx..........
				0x3e, // ..xxxxx.........
				0x77, // .xxx.xxx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
		},//   58H
		{ 		0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x63, // .xx...xx........
				0x77, // .xxx.xxx........
				0x3e, // ..xxxxx.........
				0x1c, // ...xxx..........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
				0x18, // ...xx...........
		},//   59H
		{ 		0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
				0x07, // .....xxx........
				0x0c, // ....xx..........
				0x18, // ...xx...........
				0x30, // ..xx............
				0x60, // .xx.............
				0x60, // .xx.............
				0x7f, // .xxxxxxx........
				0x7f, // .xxxxxxx........
		}//   5aH
};
#endif 

#ifdef NotUsed // SaveForHistory/Future 
// 21 x 16 font
static const uint8_t FontDataLarge[12][21 * 2] = {

{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x03,
		0xc0, 0x03, 0xc0, 0x03, 0xc0, }, // '.'

		/* Character   (0x20):
		 ht=16, width=16
		 +--------+
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+ */
		{ 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00,

		},
		/* Character 0 (0x30):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 | *****  |
		 |**   ** |
		 |**   ** |
		 |**  *** |
		 |** **** |
		 |**** ** |
		 |***  ** |
		 |**   ** |
		 |**   ** |
		 | *****  |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x3f, 0xf0, 0x3f, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0xfc, 0xf0, 0xfc,

		0xf3, 0xfc, 0xf3, 0xfc,

		0xff, 0x3c, 0xff, 0x3c,

		0xfc, 0x3c, 0xfc, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xf0, 0x3f, 0xf0,

		},
		/* Character 1 (0x31):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 |   **   |
		 | ****   |
		 |   **   |
		 |   **   |
		 |   **   |
		 |   **   |
		 |   **   |
		 |   **   |
		 |   **   |
		 | ****** |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x03, 0xc0, 0x03, 0xc0,

		0x3f, 0xc0, 0x3f, 0xc0,

		0x03, 0xc0, 0x03, 0xc0,

		0x03, 0xc0, 0x03, 0xc0,

		0x03, 0xc0, 0x03, 0xc0,

		0x03, 0xc0, 0x03, 0xc0,

		0x03, 0xc0, 0x03, 0xc0,

		0x03, 0xc0, 0x03, 0xc0,

		0x03, 0xc0, 0x03, 0xc0,

		0x3f, 0xfc, 0x3f, 0xfc,

		},
		/* Character 2 (0x32):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 | *****  |
		 |**   ** |
		 |**   ** |
		 |     ** |
		 |    **  |
		 |   **   |
		 |  **    |
		 | **     |
		 |**   ** |
		 |******* |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x3f, 0xf0, 0x3f, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0xf0, 0x00, 0xf0,

		0x03, 0xc0, 0x03, 0xc0,

		0x0f, 0x00, 0x0f, 0x00,

		0x3c, 0x00, 0x3c, 0x00,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xff, 0xfc, 0xff, 0xfc,

		},
		/* Character 3 (0x33):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 | *****  |
		 |**   ** |
		 |     ** |
		 |     ** |
		 |  ****  |
		 |     ** |
		 |     ** |
		 |     ** |
		 |**   ** |
		 | *****  |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x3f, 0xf0, 0x3f, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0x0f, 0xf0, 0x0f, 0xf0,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xf0, 0x3f, 0xf0,

		},
		/* Character 4 (0x34):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 |    **  |
		 |   ***  |
		 |  ****  |
		 | ** **  |
		 |**  **  |
		 |**  **  |
		 |******* |
		 |    **  |
		 |    **  |
		 |   **** |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x00, 0xf0, 0x00, 0xf0,

		0x03, 0xf0, 0x03, 0xf0,

		0x0f, 0xf0, 0x0f, 0xf0,

		0x3c, 0xf0, 0x3c, 0xf0,

		0xf0, 0xf0, 0xf0, 0xf0,

		0xf0, 0xf0, 0xf0, 0xf0,

		0xff, 0xfc, 0xff, 0xfc,

		0x00, 0xf0, 0x00, 0xf0,

		0x00, 0xf0, 0x00, 0xf0,

		0x03, 0xfc, 0x03, 0xfc,

		},
		/* Character 5 (0x35):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 |******* |
		 |**      |
		 |**      |
		 |**      |
		 |******  |
		 |     ** |
		 |     ** |
		 |     ** |
		 |**   ** |
		 | *****  |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0xff, 0xfc, 0xff, 0xfc,

		0xf0, 0x00, 0xf0, 0x00,

		0xf0, 0x00, 0xf0, 0x00,

		0xf0, 0x00, 0xf0, 0x00,

		0xff, 0xf0, 0xff, 0xf0,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xf0, 0x3f, 0xf0,

		},
		/* Character 6 (0x36):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 | *****  |
		 |**   ** |
		 |**      |
		 |**      |
		 |******  |
		 |**   ** |
		 |**   ** |
		 |**   ** |
		 |**   ** |
		 | *****  |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x3f, 0xf0, 0x3f, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x00, 0xf0, 0x00,

		0xf0, 0x00, 0xf0, 0x00,

		0xff, 0xf0, 0xff, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xf0, 0x3f, 0xf0,

		},
		/* Character 7 (0x37):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 |******* |
		 |**   ** |
		 |     ** |
		 |    **  |
		 |   **   |
		 |  **    |
		 |  **    |
		 |  **    |
		 |  **    |
		 |  **    |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0xff, 0xfc, 0xff, 0xfc,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0xf0, 0x00, 0xf0,

		0x03, 0xc0, 0x03, 0xc0,

		0x0f, 0x00, 0x0f, 0x00,

		0x0f, 0x00, 0x0f, 0x00,

		0x0f, 0x00, 0x0f, 0x00,

		0x0f, 0x00, 0x0f, 0x00,

		0x0f, 0x00, 0x0f, 0x00,

		},
		/* Character 8 (0x38):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 | *****  |
		 |**   ** |
		 |**   ** |
		 |**   ** |
		 | *****  |
		 |**   ** |
		 |**   ** |
		 |**   ** |
		 |**   ** |
		 | *****  |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x3f, 0xf0, 0x3f, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xf0, 0x3f, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xf0, 0x3f, 0xf0,

		},
		/* Character 9 (0x39):
		 ht=16, width=8
		 +--------+
		 |        |
		 |        |
		 | *****  |
		 |**   ** |
		 |**   ** |
		 |**   ** |
		 |**   ** |
		 | ****** |
		 |     ** |
		 |     ** |
		 |**   ** |
		 | *****  |
		 |        |
		 |        |
		 |        |
		 |        |
		 +--------+  */
		{ 0x00, 0x00,

		0x3f, 0xf0, 0x3f, 0xf0,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xfc, 0x3f, 0xfc,

		0x00, 0x3c, 0x00, 0x3c,

		0x00, 0x3c, 0x00, 0x3c,

		0xf0, 0x3c, 0xf0, 0x3c,

		0x3f, 0xf0, 0x3f, 0xf0,

		} };
#endif

#ifdef NotUsed // SaveForHistory/Future 
static void MED_DrawString(int8_t * String, int16_t xpos, int16_t ypos,
		bool invert) {
	uint8_t exitme = 0;

	// if the string will fit on the current line (height)
	if (ypos + GLCD_FONT_HEIGHT_MEDIUM <= GLCD_NUMBER_OF_LINES) {
		while ((*String != 0x0) && (exitme == 0)) {
			// if the character is not in the font 1 list
			if ((*String != '.') && (*String != ' ')
					&& ((!((*String >= '0') && (*String <= '9')))
							&& (!((*String >= 'A') && (*String <= 'Z')))))
				exitme = 1;

			// if the character will fit on the current line (width)
			if (xpos + GLCD_FONT_WIDTH_MEDIUM <= GLCD_PIXELS_PER_LINE) {
				if (exitme == 0) {
					MED_PutChar(*String, xpos, ypos, invert);
				}
			} else
				exitme = 1;

			xpos += GLCD_FONT_WIDTH_MEDIUM;
			String++;
		}
	}
}

static void LARGE_DrawString(int8_t * String, int16_t xpos, int16_t ypos,
		bool invert) {
	uint8_t exitme = 0;

	// if the string will fit on the current line (height)
	if (ypos + GLCD_FONT_HEIGHT_LARGE <= GLCD_NUMBER_OF_LINES) {
		while ((*String != 0x0) && (exitme == 0)) {
			// if the character is not in the font 1 list
			if ((*String != '.') && (*String != ' ')
					&& ((*String < '0') || (*String > '9')))
				exitme = 1;

			// if the character will fit on the current line (width)
			if (xpos + GLCD_FONT_WIDTH_LARGE <= GLCD_PIXELS_PER_LINE) {
				if (exitme == 0) {
					// put the new character
					LARGE_PutChar(*String, xpos, ypos, invert);
				}
			} else
				exitme = 1;

			xpos += GLCD_FONT_WIDTH_LARGE;
			String++;
		}
	}
}

static void LARGE_PutChar(int8_t Char, int16_t xpos, int16_t ypos, bool invert) {
	uint8_t i;
	uint8_t CharOffset;

	if (Char == '.') {
		CharOffset = 0;
	} else if (Char == ' ') {
		CharOffset = 1;
	} else {
		CharOffset = (Char - 0x30) + 2;
	}

	for (i = 0; i < GLCD_FONT_HEIGHT_LARGE; i++)
		lcdWriteBytesAbsolute(xpos, ypos + i,
				(const uint8_t *) &FontDataLarge[CharOffset][i * 2], 2, invert);
}

static void MED_PutChar(int8_t Char, int16_t xpos, int16_t ypos, bool invert) {
	uint8_t i;
//	uint8_t j;
	uint8_t CharOffset;

	if (Char == '.') {
		CharOffset = 0;
	} else if (Char == ' ') {
		CharOffset = 1;
	} else {
		if ((Char >= '0') && (Char <= '9'))
			CharOffset = (Char - '0') + 2;
		else
			CharOffset = (Char - 'A') + 12;
	}

	for (i = 0; i < GLCD_FONT_HEIGHT_MEDIUM; i++)
		lcdWriteBytesAbsolute(xpos, ypos + i,
				(const uint8_t *) &FontDataMedium[CharOffset][i], 1, invert);
}

static const uint8_t SetBar[8] = { 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
		0x00 };

static const uint8_t ClearBar[8] = { 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
		0x00 };

void lcdDrawBlock(int16_t x1, int16_t y1, uint8_t set_clear) {
	uint8_t i;

	if (set_clear == 1) {
		for (i = 0; i < 8; i++)
			lcdWriteBytesAbsolute(x1, y1 + i, (const uint8_t *) SetBar + i, 1,
					False);
	} else {
		for (i = 0; i < 8; i++)
			lcdWriteBytesAbsolute(x1, y1 + i, (const uint8_t *) ClearBar + i, 1,
					False);
	}
}
#endif
