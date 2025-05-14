//--------------------------------------------------------
//  $Id: utils.h 303 2010-07-08 03:43:32Z david $
//  $Date: 2010-07-08 11:43:32 +0800 (Thu, 08 Jul 2010) $
//  $Rev: 303 $
//  Brief description: 	header file for common system utilities	
//  Last changed by $Author: david $
//--------------------------------------------------------
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

uint8_t sumbuffer(int8_t* buff, int16_t len);
uint8_t xorbuffer(int8_t* buff, int16_t len);
void strcpyn(uint8_t *dest, uint8_t *src, uint8_t maxlen);
void convItoA(int8_t *string, int32_t number, int8_t *format);
//int32_t labs(int32_t value);
double stringToInt(uint8_t *c, int len, int base, int pointerInc);
char *strtokWEmptyCheck(char *s, char delim);
#ifdef CodeChange_6_23_2022
// changing to uint8_t because that is how it is used in code, and that serves the purpose.
uint8_t atoi_c(uint8_t ascii);
#else
int8_t atoi_c(int8_t ascii);
#endif
void skipChar232_2(uint8_t skips, uint8_t ch);

#endif

