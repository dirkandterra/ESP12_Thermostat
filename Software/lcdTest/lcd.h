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
#include <avr/io.h>
#define bool uint8_t
#define False 0
#define True 1
#define XTALFREQ 16
// LCD data port
#define GLCD_DATA_RD_0 PINB & 0x01
#define GLCD_DATA_RD_1 PINB & 0x02
#define GLCD_DATA_RD_2 PIND & 0x04
#define GLCD_DATA_RD_3 PIND & 0x08
#define GLCD_DATA_RD_4 PIND & 0x10
#define GLCD_DATA_RD_5 PIND & 0x20
#define GLCD_DATA_RD_6 PIND & 0x40
#define GLCD_DATA_RD_7 PIND & 0x80

#define BitSet(Port,Bit) (Port|=(1<<Bit))
#define BitClear(Port,Bit) (Port&=~(1<<Bit))

#define GLCD_DATA_0_SET BitSet(PORTB,0);
#define GLCD_DATA_0_CLR BitClear(PORTB,0);
#define GLCD_DATA_1_SET BitSet(PORTB,1);
#define GLCD_DATA_1_CLR BitClear(PORTB,1);
#define GLCD_DATA_2_SET BitSet(PORTD,2);
#define GLCD_DATA_2_CLR BitClear(PORTD,2);
#define GLCD_DATA_3_SET BitSet(PORTD,3);
#define GLCD_DATA_3_CLR BitClear(PORTD,3);
#define GLCD_DATA_4_SET BitSet(PORTD,4);
#define GLCD_DATA_4_CLR BitClear(PORTD,4);
#define GLCD_DATA_5_SET BitSet(PORTD,5);
#define GLCD_DATA_5_CLR BitClear(PORTD,5);
#define GLCD_DATA_6_SET BitSet(PORTD,6);
#define GLCD_DATA_6_CLR BitClear(PORTD,6);
#define GLCD_DATA_7_SET BitSet(PORTD,7);
#define GLCD_DATA_7_CLR BitClear(PORTD,7);

#define GLCD_DATA_INPUT   DDRD&=0b00000011;DDRB&=0b11111100;
#define GLCD_DATA_OUTPUT  DDRD|=0b11111100;DDRB|=0b00000011;

#define DEFINE_PORTC DDRC|=0b00110000
#define DEFINE_PORTB DDRB|=0b00011100

// time before reading in key's after setting
#define WAITTIME 		2

#define	GLCD_WR_1		BitSet(PORTC,5)     // WR	Data write (active low)
#define	GLCD_WR_0		BitClear(PORTC,5)   
#define	GLCD_RD_1		BitSet(PORTC,4)     // RD	Data read (active low)
#define	GLCD_RD_0		BitClear(PORTC,4)
#define	GLCD_CE_1		BitSet(PORTB,2)     // CE	Chip enable (active low)
#define	GLCD_CE_0		BitClear(PORTB,2)
#define	GLCD_CD_1		BitSet(PORTB,3)     // CD	CD=1, WR=0: command write
#define	GLCD_CD_0		BitClear(PORTB,3)
#define	GLCD_RS_1		BitSet(PORTB,4)     // RST	Module reset (active low)
#define	GLCD_RS_0		BitClear(PORTB,4)
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

void LcdInit(void);
void lcdDrawBitmap(int16_t x1, int16_t y1, const ABitmap *bitmap);
void lcdClearGraphic(void);
void lcdClearText(void);

void lcdSetCursor(uint8_t row, uint8_t column);
void lcdCursor(uint8_t OnOff, uint8_t flash);
uint8_t lcdReadChar(int16_t x1, int16_t y1);
void lcdDrawString(int16_t x1, int16_t y1, int8_t *value);

#endif

