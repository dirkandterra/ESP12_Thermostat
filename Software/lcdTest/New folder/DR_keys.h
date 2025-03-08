//--------------------------------------------------------
//  $Id: keys.h 303 2010-07-08 03:43:32Z david $
//  $Date: 2010-07-08 11:43:32 +0800 (Thu, 08 Jul 2010) $
//  $Rev: 303 $
//  Brief description: 	Key Handling
//  Last changed by $Author: david $
//--------------------------------------------------------
#ifndef KEYS_H
#define KEYS_H

#include "event.h"

void ProcessKeyPressController(AEvent event);
void KeyPressInit(void);
void totalResetUpdate(void);
void goToHome(void);
void ShowEEPROMError(void);
void TestingMenu(void);
void cursor(void);

#endif
