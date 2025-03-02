#ifndef GLCD12864_H
#define GLCD12864_H
#define FONT_USED c64font
#define FONT_PRINT_WIDTH 8
#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define LCD_WIDTH 128
#define LCD_HEIGHT 64
#define LCD_CHIP_WIDTH 64

#define NONETWORK 128
#define WIFINETWORK 127

#define CUSTOM_FONT_INDEX GameBoyFont__index
#define CUSTOM_FONT_BITMAP GameBoyFont__bitmap
#include <stdint.h>

void glcdInit(uint32_t clockRate, uint8_t latch);
void printPhrase(char *ph);
void lcdPrintChar(char c);
void lcdCustomPrintChar(unsigned char c);
void lcdClearScreen();
void setCoord(uint8_t x, uint8_t y);
#endif