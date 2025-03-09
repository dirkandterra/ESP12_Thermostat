#include "DR_screens.h"
#include "lcd.h"
#include "splash.h"
#include <stdint.h>

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  delay(1000);
  LcdInit();
}

void loop() {
  const ABitmap clearFlame = {1,8,0,0,0,0,0,0,0,0};
  // put your main code here, to run repeatedly:
  //lcdDrawBitmap(32, 0, (const ABitmap *) &SplashStartup);
  delay(500);
  for(int ii=0;ii<5;ii++){
    //lcdDrawBitmap(0,0,(const ABitmap *) &bigFlame1);
    lcdDrawBitmap(0,32,(const ABitmap *) &bigFlake1);
    delay(500);
    lcdDrawBitmap(0,0,(const ABitmap *) &bigFlame2);
    lcdDrawBitmap(0,32,(const ABitmap *) &bigFlake2);
    delay(500);
  }
  ScreenDraw(0);

  delay(2000);
  lcdClearGraphic();
  delay(2000);
  lcdClearText();
  delay(1000);
}