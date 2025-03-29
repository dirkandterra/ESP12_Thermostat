/********************************************************************/
/*   Name: StartScreen                                              */
/*------------------------------------------------------------------*/
/* Brief description: 	Start up screen					        	*/
/* File name:         	StartScreen.c                               */
/* Release              0.1                                         */
/*- Description: ---------------------------------------------------*/
/*- History: -------------------------------------------------------*/
/*  0.1  06/11/08  DB   Original                                    */
/*------------------------------------------------------------------*/

//#include "core.h"
#include "lcd.h"
#include "startscreen.h"
//#include "string.h"
//#include "hw.h"

void StartScreen(void) {
	uint16_t i, j; //, k;
	//lcdClearArea(30,0,64,64);

	// draw on the Splash logo
#ifndef DRFLASHTEST
	lcdDrawBitmap(32, 0, (const ABitmap *) &SplashStartup);
#else
	// Alternative to splash screen to save memory
	//uint8_t c[14] = {VERSION};
	lcdDrawString(4,3,"DR_TEST");
#endif

	//WatchDog Enabled, kick it 
	__RESET_WATCHDOG(); //1 Sec timout as event handeler interrupt not running

	i = 0;
	j = 0;
	while ((j < 75)) {
		j++;
		while ((i < 32000)) {
			i++;

			//WatchDog Enabled, kick it 
			__RESET_WATCHDOG(); //1 Sec timout as event handeler interrupt not running

		}
		i = 0;
	}

	__RESET_WATCHDOG();

	// clear the Splash logo

	lcdClearGraphic();
	//for (i=0;i<SplashStartup.height;i++)	
	//lcdClearBytesAbsolute(32, i, SplashStartup.width);  //TODO:  Traded the top line for these two to save flash space and not load this function	

	//lcdClearArea(30,0,64,64);

}
