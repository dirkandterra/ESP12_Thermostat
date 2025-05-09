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
#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#define bool uint8_t
#define False 0
#define True 1
#define XTALFREQ 72

extern uint16_t PCF8575_DataDir;
extern uint16_t PCF8575_DataCurrent;

// time before reading in key's after setting
#define WAITTIME 		2

#define WR 4
#define RD 5
#define CD 6
#define RS 7
#define CE_Pin 14

#define	GLCD_WR_1		PCF8575_DataCurrent |= (0x0001<<WR)     // WR	Data write (active low)
#define	GLCD_WR_0		PCF8575_DataCurrent &= ~(0x0001<<WR)  
#define	GLCD_RD_1		PCF8575_DataCurrent |= (0x0001<<RD)     // RD	Data read (active low)
#define	GLCD_RD_0		PCF8575_DataCurrent &= ~(0x0001<<RD)
#define	GLCD_CE_1		digitalWrite(CE_Pin,1)      // CE	Chip enable (active low)
#define	GLCD_CE_0		digitalWrite(CE_Pin,0)
#define	GLCD_CD_1		PCF8575_DataCurrent |= (0x0001<<CD)      // CD	CD=1, WR=0: command write
#define	GLCD_CD_0		PCF8575_DataCurrent &= ~(0x0001<<CD)
#define	GLCD_RS_1		PCF8575_DataCurrent |= (0x0001<<RS)      // RST	Module reset (active low)
#define	GLCD_RS_0		PCF8575_DataCurrent &= ~(0x0001<<RS)
//      			                        CD=1, WR=1: command read
//           	                            CD=0, WR=0: data write
//                                          CD=0, WR=1: data read 

// display properties
#define GLCD_NUMBER_OF_LINES				64
#define GLCD_PIXELS_PER_LINE				128
#define GLCD_FONT_WIDTH						8
#define GLCD_FONT_WIDTH_MEDIUM				8
#define GLCD_FONT_HEIGHT_MEDIUM				10
#define GLCD_FONT_WIDTH_LARGE				16
#define GLCD_FONT_HEIGHT_LARGE				21

#define GLCD_GRAPHIC_AREA					(GLCD_PIXELS_PER_LINE / GLCD_FONT_WIDTH)
#define GLCD_TEXT_AREA						(GLCD_PIXELS_PER_LINE / GLCD_FONT_WIDTH)
#define GLCD_GRAPHIC_SIZE					(GLCD_GRAPHIC_AREA * GLCD_NUMBER_OF_LINES)
#define GLCD_TEXT_SIZE						(GLCD_TEXT_AREA * (GLCD_NUMBER_OF_LINES/8))

#define GLCD_TEXT_HOME						0
#define GLCD_GRAPHIC_HOME					(GLCD_TEXT_HOME + GLCD_TEXT_SIZE)
#define GLCD_OFFSET_REGISTER				2
#define GLCD_EXTERNAL_CG_HOME				(GLCD_OFFSET_REGISTER << 11)

#define T6963_SET_CURSOR_POINTER			0x21
#define T6963_SET_OFFSET_REGISTER			0x22
#define T6963_SET_ADDRESS_POINTER			0x24

#define T6963_SET_TEXT_HOME_ADDRESS			0x40
#define T6963_SET_TEXT_AREA					0x41
#define T6963_SET_GRAPHIC_HOME_ADDRESS		0x42
#define T6963_SET_GRAPHIC_AREA				0x43

#define T6963_MODE_SET						0x80

#define T6963_DISPLAY_MODE					0x90
#define T6963_CURSOR_BLINK_ON			0x01
#define T6963_CURSOR_DISPLAY_ON			0x02
#define T6963_TEXT_DISPLAY_ON			0x04
#define T6963_GRAPHIC_DISPLAY_ON		0x08				

#define T6963_CURSOR_PATTERN_SELECT			0xA0
#define T6963_CURSOR_1_LINE				0x00
#define T6963_CURSOR_2_LINE				0x01
#define T6963_CURSOR_3_LINE				0x02
#define T6963_CURSOR_4_LINE				0x03
#define T6963_CURSOR_5_LINE				0x04
#define T6963_CURSOR_6_LINE				0x05
#define T6963_CURSOR_7_LINE				0x06
#define T6963_CURSOR_8_LINE				0x07

#define T6963_SET_DATA_AUTO_WRITE			0xB0
#define T6963_SET_DATA_AUTO_READ			0xB1
#define T6963_AUTO_RESET					0xB2

#define T6963_DATA_WRITE_AND_INCREMENT		0xC0
#define T6963_DATA_READ_AND_INCREMENT		0xC1
#define T6963_DATA_WRITE_AND_DECREMENT		0xC2
#define T6963_DATA_READ_AND_DECREMENT		0xC3
#define T6963_DATA_WRITE_AND_NONVARIALBE	0xC4
#define T6963_DATA_READ_AND_NONVARIABLE		0xC5

#define T6963_SCREEN_PEEK					0xE0
#define T6963_SCREEN_COPY					0xE8

#define NO_ATTRS                 0x00
#define CHARSET_DEFAULT          0x00
#define CHARSET_SMALL            0x00
#define CHARSET_MEDIUM           0x01
#define CHARSET_LARGE	         0x02

typedef struct {			// Definition of a 64 * 64 pixel bitmap:-
	uint8_t width;		// - width in bytes
	uint8_t height;		// - height in pixels
	uint8_t data[8 * 60];	// - stream of pixels
} ABitmap;

void SetDataDir(uint8_t in);
void SetData(uint8_t n);

void LcdInit(uint32_t clockRate);
void lcdDrawBitmap(int16_t x1, int16_t y1, const ABitmap *bitmap);
void lcdClearGraphic(void);
void lcdClearText(void);

void lcdSetCursor(uint8_t row, uint8_t column);
void lcdCursor(uint8_t OnOff, uint8_t flash);
uint8_t lcdReadChar(int16_t x1, int16_t y1);
void lcdDrawString(int16_t x1, int16_t y1, int8_t *value);
void printThis(uint8_t that);

#endif

