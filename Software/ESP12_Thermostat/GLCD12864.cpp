#include <Arduino.h>
#include "GLCD12864.h"
#include "Wire.h"
#include "c64Font.c"
#include "GameBoyFont.c"
#define pcfAddress 0x20

uint8_t customX=0;
uint8_t InvertFont=0;

typedef struct _xy{
  uint8_t x;
  uint8_t y;
}Coordinates;

Coordinates textCoord;
uint16_t lastRead=0x00;

uint8_t LatchPin;

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
  return _dataIn;
}

bool write16(const uint16_t value)
{
  uint16_t dataOut = value;
  Wire.beginTransmission(pcfAddress);
  Wire.write(dataOut & 0xFF);      //  low 8 bits
  Wire.write(dataOut >> 8);        //  high 8 bits
  return Wire.endTransmission();
}


void lcdWrite(uint16_t data){
  write16(data);
  digitalWrite(LatchPin,1);
  digitalWrite(LatchPin,1);
  digitalWrite(LatchPin,0);
  digitalWrite(LatchPin,0);
}
void setCoord(uint8_t x, uint8_t y){
  textCoord.x = x;
  customX=x;
  textCoord.y = y;
}

void setBitCoordX(uint8_t x, uint8_t chip){
  uint16_t data=0;
  data = (chip<<2)*256+0x40+x;
  lcdWrite(data);
}

void setBitCoordY(uint8_t y, uint8_t chip){
  uint16_t data=0;
  data = (chip<<2)*256+0xB8+y;
  lcdWrite(data);
}

void lcdClearScreen(){
  uint8_t chipSel=3;
  uint16_t temp;
  uint8_t data=0x00;
  int ii=0,jj=0;
  if(InvertFont){
    data=0xFF;
  }
  for(ii=0;ii<(LCD_HEIGHT/8);ii++){
    setBitCoordY(ii,chipSel); //Set page or line
    setBitCoordX(0,chipSel); //Set horizontal pixel
    for(jj=0;jj<LCD_CHIP_WIDTH;jj++){
      lcdWrite((chipSel<<2)*256 + 0x200 + data);
    }
  }
}

void lcdCustomPrintChar(unsigned char c){
  c=c-0x20;
  //Serial.println(c,DEC);
  uint16_t temp=0;
  uint16_t fontBase=CUSTOM_FONT_INDEX[c];
  uint8_t fontPrintWidth=CUSTOM_FONT_BITMAP[fontBase];
  uint8_t fontWidth=fontPrintWidth+0;
  uint8_t chipSel=0x00;
  uint8_t pixelX=customX;
  uint8_t lastLine=fontWidth;
  int ii=0;
  fontBase++;
  //If the char won't fit on this line, jump to the next
  if((pixelX+fontPrintWidth)>=LCD_WIDTH){
    chipSel=1;
    pixelX=0;    
    textCoord.x=0;
    textCoord.y++;
    //If next line won't fit on the screen, start at the top
    if((textCoord.y*FONT_HEIGHT)>=LCD_HEIGHT){
      textCoord.y=0;
    }
  }
  // If the character begines on chip two, get the chip select correct
  else if(pixelX>=LCD_CHIP_WIDTH){
    //Do we have room for the full spacer after the char before LCD width end?
    if((pixelX+fontWidth)>=LCD_WIDTH){
      lastLine=LCD_WIDTH-pixelX-1;      //Print only char and blank lines that fit
    }
    chipSel=2;
    pixelX=pixelX-LCD_CHIP_WIDTH;
  }
  else{
    chipSel=1;
  }
  setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
  setBitCoordX(pixelX,chipSel); //Set horizontal pixel
  for(ii=0;ii<fontPrintWidth;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    temp=CUSTOM_FONT_BITMAP[fontBase+ii];
    if(InvertFont){
      temp = ~temp&0xFF;
    }
    lcdWrite((chipSel<<2)*256+0x0200+temp);
    pixelX++;
  }
  for(ii=fontPrintWidth;ii<=lastLine;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    if(!InvertFont){
      lcdWrite((chipSel<<2)*256+0x0200+0x00);
    }else{
      lcdWrite((chipSel<<2)*256+0x0200+0xFF);
    }
    pixelX++;
  }
  customX=pixelX+(chipSel-1)*64;
}

void lcdPrintChar(char c){
  uint16_t temp=0;
  uint8_t chipSel=0x00;
  uint8_t pixelX=textCoord.x*FONT_WIDTH;
  uint8_t lastLine=FONT_WIDTH;
  int ii=0;
  //If the char won't fit on this line, jump to the next
  if((pixelX+FONT_PRINT_WIDTH)>=LCD_WIDTH){
    chipSel=1;
    pixelX=0;    
    textCoord.x=0;
    textCoord.y++;
    //If next line won't fit on the screen, start at the top
    if((textCoord.y*FONT_HEIGHT)>=LCD_HEIGHT){
      textCoord.y=0;
    }
  }
  // If the character begines on chip two, get the chip select correct
  else if(pixelX>=LCD_CHIP_WIDTH){
    //Do we have room for the full spacer after the char before LCD width end?
    if((pixelX+FONT_WIDTH)>=LCD_WIDTH){
      lastLine=LCD_WIDTH-pixelX-1;      //Print only char and blank lines that fit
    }
    chipSel=2;
    pixelX=pixelX-LCD_CHIP_WIDTH;
  }
  else{
    chipSel=1;
  }
  setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
  setBitCoordX(pixelX,chipSel); //Set horizontal pixel
  for(ii=0;ii<FONT_PRINT_WIDTH;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    temp=FONT_USED[(c-0x20)*FONT_PRINT_WIDTH+ii];
    if(InvertFont){
      temp = ~temp&0xFF;
    }
    lcdWrite((chipSel<<2)*256+0x0200+temp);
    pixelX++;
  }
  for(ii=FONT_PRINT_WIDTH;ii<=lastLine;ii++){
    if(pixelX==LCD_CHIP_WIDTH){
      pixelX=0;
      chipSel=2;
      setBitCoordY((textCoord.y*FONT_HEIGHT)/8,chipSel); //Set page or line
      setBitCoordX(pixelX,chipSel); //Set horizontal pixel
    }
    if(!InvertFont){
      lcdWrite((chipSel<<2)*256+0x0200+0x00);
    }else{
      lcdWrite((chipSel<<2)*256+0x0200+0xFF);
    }
    pixelX++;
  }
  textCoord.x++;
  if((textCoord.x*FONT_WIDTH)>=LCD_WIDTH){
    textCoord.x=0;
    textCoord.y++;
    if((textCoord.y*FONT_HEIGHT)>=LCD_HEIGHT){
      textCoord.y=0;
    }
  }
}

void printPhrase(char *ph){
  int x=0;
  while(ph[x]!=0x00){
    lcdPrintChar(ph[x]);
    x++;
  }
}

void glcdInit(uint32_t clockRate, uint8_t latch){
  pinMode(latch,OUTPUT);
  LatchPin=latch;
  Wire.setClock(clockRate);
  Wire.begin();
  delay(500);
  lcdWrite(0x0C3F);
  lcdWrite(0x0C40);
  lcdWrite(0x0CC0);
  delay(500);
  lcdClearScreen();
}