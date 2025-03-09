/********************************************************************/
/*   Name: Utils		                                            */
/*------------------------------------------------------------------*/
/* Brief description: 	Common system utilities				        */
/* File name:         	utils.c	                            		*/
/* Release:				   0.2                                      */
/*- Description: ---------------------------------------------------*/
/*- History: -------------------------------------------------------*/
/*  0.1  12/05/05  DB   Original									*/
/*  0.2  03/08/09  DB   Changed to allow greater than 10 places		*/
/********************************************************************/

#include <string.h>
#include <stdint.h>
#include "utils.h"

uint8_t sumbuffer(int8_t* buff, int16_t len) {
	uint8_t sum = 0;
	for (; len > 0; len--)
		sum += *buff++;
	return (sum);
}

uint8_t xorbuffer(int8_t* buff, int16_t len) {
	uint8_t xord = 0;

	for (; len > 0; len--)
		xord ^= *buff++;
	return xord;

}

// copy max of maxlen characters into dest and null terminate
void strcpyn(uint8_t *dest, uint8_t *src, uint8_t maxlen) {
	volatile unsigned char destSize;
	volatile unsigned char srcSize;

	destSize = sizeof(dest);
	srcSize = sizeof(src);

	dest[maxlen] = 0; //Null Terminate
	while ((*src != 0) && (maxlen > 0)) {
		*dest++ = *src++;
		maxlen--;
	}
	*dest = 0;
}

void convItoA(int8_t *string, int32_t number, int8_t *format) {
	int8_t *cPtr, *dPtr, buffer[16];
	int16_t intLen, fracLen, magnitude;
	int32_t li;

	cPtr = format; /* Initialise the formatting values */
	intLen = 0;
	fracLen = 0;
	magnitude = 0;
	while (cPtr) {
		/* Parse that format string... */
		if (*cPtr == '%') /* Skip '%' anyway */
			cPtr++;
		if (*cPtr == 0)
			break;

		if ((*cPtr >= '0') && (*cPtr <= '9')){ 
			// Get field length 
			intLen = (int16_t)(*cPtr++ - '0');
		}else if ((*cPtr >= 'a') && (*cPtr <= 'e')){
			intLen = (int16_t)(*cPtr++ - 'a' + 10);
		}

		if (*cPtr == '.') /* Skip field length/precision */
			cPtr++; /* separator.                  */
		if (cPtr == 0)
			break;
		if ((*cPtr >= '0') && (*cPtr <= '9')) {
			/* This is the */
			fracLen = (int16_t)(*cPtr++ - '0'); /* precision   */
			if (fracLen > (intLen - 2)) /* Fractional part cannot be */
				fracLen = intLen - 2; /* longer than this.         */
		}
		if (*cPtr == 'd')
			magnitude = 1;
		else if (*cPtr == 'c')
			magnitude = 2;
		else if (*cPtr == 'm')
			magnitude = 3;
		else if (*cPtr == 'D')
			magnitude = 4;
		else if (*cPtr == 'C')
			magnitude = 5;
		else if (*cPtr == 'M')
			magnitude = 6;
		break;
	}
	if (intLen) {
		/* Convert field length to integer part length */
		intLen = intLen - fracLen - 1;
		if (fracLen == 0)
			intLen++; /* adjust as no decimal */
	} else {
		/* No field length - no formatting */
		fracLen = 0;
		magnitude = 0;
	}

	if (number >= 0)
		li = number;
	else
		li = -number;
	cPtr = buffer;
	dPtr = &(buffer[magnitude]);/* That's where the decimal point */
	/* lives...                       */
	while (li > 0) {
		/* Convert number and store in reverse order */
		*cPtr++ = (int8_t)((li % 10) + '0');
		li = li / 10;
	}
	while (cPtr <= dPtr) /* Fill the buffer with 0es up to the */
		*cPtr++ = '0'; /* decimal point and one more         */
	if (number < 0) /* Add a minus sign if required */
		*cPtr++ = '-';
	while (cPtr < &(buffer[magnitude + intLen]))/* Pad with spaces*/
		*cPtr++ = ' ';
	if (intLen == 0) { 
		/* In the case that there is no */
		/* field size, work it out now. */
		intLen = (int16_t)(cPtr - dPtr);
		fracLen = (int16_t)(dPtr - buffer);
	}
	for (li = 0; li < (intLen + fracLen); li++) {
		if (cPtr == buffer) /* Access falls outside buffer */
			*string++ = '0';
		else {
			*string++ = *--cPtr;
			if ((cPtr == dPtr) && (fracLen))
				*string++ = '.';
		}
	}
	*string = 0;
}

int32_t labs(int32_t value) {
	int32_t mod;

	if (value < 0)
		mod = 0 - value;
	else
		mod = value;

	return (mod);
}

//****************************************************************
// stringToInt
//****************************************************************
//
// 'c' is the original pointer address
// 'len' is how long the string is
// 'base' is what base the number should be
// 'pointerInc' is what offset from the original pointer
//
// By including pointerInc, it lets you pass in a string and jump
// to an offset, but first checking that the string is indeed that long
// (might be a null string or only 3 chars long and you want to look at 
// chars 4,5,6)
//*****************************************************************
double stringToInt(uint8_t *c, int len, int base, int pointerInc) {
	double retInt = 0;
	int ii = 0;

	if (c == 0) {
		return 0;
	}
	for (ii = 0; ii < pointerInc; ii++) {
		if (c[ii] == 0) {
			return 0;
		}
	}
	c += pointerInc;
	while (len > 0) {
		if ((*c >= '0') && (*c <= '9')) {
			retInt = retInt * base;
			retInt += (int) (*c - 0x30);
		} else if ((*c >= 'A') && (*c <= 'F')) {
			retInt = retInt * base;
			retInt += (int) (*c - 0x41 + 10);
		} else if ((*c >= 'a') && (*c <= 'f')) {
			retInt = retInt * base;
			retInt += (int) (*c - 0x61 + 10);
		} else if (*c == 0) {
			return retInt;
		} else {
			return -1;
		}
		*c++;
		len--;
	}
	return retInt;
} // End of: double stringToInt(uint8_t *c, int len, int base, int pointerInc) {

//****************************************************************
// strtokWEmptyCheck
//****************************************************************
//
//  This function works like strtok, but it won't skip empty sets.
//  Also notice that it only looks for one delimiter instead of an array.
//  (char not char*)
//  For example strtok(",,,1,",",") will return a pointer to the '1'
//  char.  strtokWEmptyCheck(",,,1,",",") will change the first ',' into 
//  a 0x00 and point to that address.  When looking at the string, you
//  can know that it is empty instead of strtok returning '1' even though
//  that is what comes before the 4th found delim in that string.  Like
//  strtok, if you pass 'NULL' into the 's' variable, it will continue
//  with the last pointer passed into it.
//
// 's' is the original pointer address
// 'delim' is that char you are wanting to break on
//
//*****************************************************************
char *strtokWEmptyCheck(char *s, char delim) {
	static char* p = 0;
	char c[2] = { delim, 0 };// have to cast a single char into a const char array
	const char *d = c;			// with these two lines
	if (s)
		p = s;
	else if (!p)
		return 0;
	s = p;
	if (*p == delim) {
		*p = 0;
		p++;
	} else {
		p += strcspn(p, d);
		p = *p ? *p = 0, p + 1 : p; //Increment pointer if current value isn't 0
	}
	return s;
}

#ifdef CodeChange_6_23_2022
uint8_t atoi_c(uint8_t ascii) {
	uint8_t retVal = 0;

	if (ascii > 47)
		retVal = ascii - 48;

	return retVal;
}
#else
int8_t atoi_c(int8_t ascii) {
	int8_t retVal = 0;

	if (ascii > 47)
		retVal = ascii - 48;

	return retVal;
}
#endif

void skipChar232_2(uint8_t skips, uint8_t ch){

}
