#include "DR_screens.h"
#include "lcd.h"
#include "splash.h"
#include <stdint.h>

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  delay(1000);
  LcdInit(200000);
}

void loop() {
  int8_t strang[2] = {'M','y'};
  const ABitmap clearFlame = {1,8,0,0,0,0,0,0,0,0};
  // put your main code here, to run repeatedly:
  //lcdDrawBitmap(32, 0, (const ABitmap *) &SplashStartup);
  delay(500);
    Serial.println("StartLoop");
  for(int ii=0;ii<5;ii++){
    lcdDrawBitmap(0,0,(const ABitmap *) &bigFlame1);
    lcdDrawBitmap(6,32,(const ABitmap *) &bigFlake1);
    delay(500);
      Serial.println("G1");
    lcdDrawBitmap(0,0,(const ABitmap *) &bigFlame2);
    lcdDrawBitmap(6,32,(const ABitmap *) &bigFlake2);
    delay(500);
    Serial.println("G2");
  }
  ScreenDraw(0);
Serial.println("G3");
if(readKey(2)&0x04){
  Serial.println("OFF");
}else{
  Serial.println("ON");
}
  delay(2000);
  lcdClearGraphic();
  Serial.println("G4");
  delay(2000);
  lcdClearText();
  Serial.println("G5");
  delay(1000);
}