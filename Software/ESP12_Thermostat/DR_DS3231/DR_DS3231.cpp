/*
DS3231.cpp: DS3231 Real-Time Clock library
Eric Ayars
4/1/11

Spliced in DateTime all-at-once reading (to avoid rollover) and unix time
from Jean-Claude Wippler and Limor Fried
Andy Wickert
5/15/11

Fixed problem with SD processors(no function call) by replacing all occurences of the term PM, which
is defined as a macro on SAMD controllers by PM_time.
Simon Gassner
11/28/2017

Fixed setting 12-hour clock in setHour function so that 12:xx AM is not stored as 00:xx and corrected
the setting of the PM flag for 12:xx PM.  These address certain DS3231 errors in properly setting the
AM/PM (bit 5) flag in the 02h register when passing from AM to PM and PM to AM.
David Merrifield
04/14/2020

Changed parameter to uint16_t in isleapYear() because the function performs 16-bit arithmetic
at (y % 400) and because date2days() calls it with a uint16_t parameter. Grouped and typecast certain parameters and intermediate results in the constructor DateTime::DateTime (uint32_t t) to resolve a couple of non-fatal compiler warnings.
David Sparks
08 Sept 2022

Released into the public domain.
*/

#include "DS3231.h"

// These included for the DateTime class inclusion; will try to find a way to
// not need them in the future...
#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif
// Changed the following to work on 1.0
//#include "WProgram.h"
#include <Arduino.h>


#define CLOCK_ADDRESS 0x68

#define SECONDS_FROM_1970_TO_2000 946684800


// Constructor
DS3231::DS3231() : _Wire(Wire) {
	// nothing to do for this constructor.
}

DS3231::DS3231(TwoWire & w) : _Wire(w) {
}

byte DS3231::getSecond() {
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x00);
	_Wire.endTransmission();

	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	return bcdToDec(_Wire.read());
}

byte DS3231::getMinute() {
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x01);
	_Wire.endTransmission();

	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	return bcdToDec(_Wire.read());
}

byte DS3231::getHour(bool& h12, bool& PM_time) {
	byte temp_buffer;
	byte hour;
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x02);
	_Wire.endTransmission();

	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	temp_buffer = _Wire.read();
	h12 = temp_buffer & 0b01000000;
	if (h12) {
		PM_time = temp_buffer & 0b00100000;
		hour = bcdToDec(temp_buffer & 0b00011111);
	} else {
		hour = bcdToDec(temp_buffer & 0b00111111);
	}
	return hour;
}

byte DS3231::getDoW() {
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x03);
	_Wire.endTransmission();

	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	return bcdToDec(_Wire.read());
}

byte DS3231::getDate() {
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x04);
	_Wire.endTransmission();

	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	return bcdToDec(_Wire.read());
}

byte DS3231::getMonth(bool& Century) {
	byte temp_buffer;
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x05);
	_Wire.endTransmission();

	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	temp_buffer = _Wire.read();
	Century = temp_buffer & 0b10000000;
	return (bcdToDec(temp_buffer & 0b01111111)) ;
}

byte DS3231::getYear() {
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x06);
	_Wire.endTransmission();

	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	return bcdToDec(_Wire.read());
}

// setEpoch function gives the epoch as parameter and feeds the RTC
// epoch = UnixTime and starts at 01.01.1970 00:00:00
// HINT: => the AVR time.h Lib is based on the year 2000
void DS3231::setEpoch(time_t epoch, bool flag_localtime) {
#if defined (__AVR__)
	epoch -= SECONDS_FROM_1970_TO_2000;
#endif
	struct tm tmnow;
	if (flag_localtime) {
		localtime_r(&epoch, &tmnow);
	}
	else {
		gmtime_r(&epoch, &tmnow);
	}
	setSecond(tmnow.tm_sec);
	setMinute(tmnow.tm_min);
	setHour(tmnow.tm_hour);
	setDoW(tmnow.tm_wday + 1U);
	setDate(tmnow.tm_mday);
	setMonth(tmnow.tm_mon + 1U);
	setYear(tmnow.tm_year - 100U);
}

void DS3231::setSecond(byte Second) {
	// Sets the seconds
	// This function also resets the Oscillator Stop Flag, which is set
	// whenever power is interrupted.
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x00);
	_Wire.write(decToBcd(Second));
	_Wire.endTransmission();
	// Clear OSF flag
	byte temp_buffer = readControlByte(1);
	writeControlByte((temp_buffer & 0b01111111), 1);
}

void DS3231::setMinute(byte Minute) {
	// Sets the minutes
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x01);
	_Wire.write(decToBcd(Minute));
	_Wire.endTransmission();
}

// Following setHour revision by David Merrifield 4/14/2020 correcting handling of 12-hour clock

void DS3231::setHour(byte Hour) {
	// Sets the hour, without changing 12/24h mode.
	// The hour must be in 24h format.

	bool h12;
	byte temp_hour;

	// Start by figuring out what the 12/24 mode is
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x02);
	_Wire.endTransmission();
	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	h12 = (_Wire.read() & 0b01000000);
	// if h12 is true, it's 12h mode; false is 24h.

	if (h12) {
		// 12 hour
		bool am_pm = (Hour > 11);
		temp_hour = Hour;
		if (temp_hour > 11) {
			temp_hour = temp_hour - 12;
		}
		if (temp_hour == 0) {
			temp_hour = 12;
		}
		temp_hour = decToBcd(temp_hour) | (am_pm << 5) | 0b01000000;
	} else {
		// 24 hour
		temp_hour = decToBcd(Hour) & 0b10111111;
	}

	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x02);
	_Wire.write(temp_hour);
	_Wire.endTransmission();
}

void DS3231::setDoW(byte DoW) {
	// Sets the Day of Week
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x03);
	_Wire.write(decToBcd(DoW));
	_Wire.endTransmission();
}

void DS3231::setDate(byte Date) {
	// Sets the Date
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x04);
	_Wire.write(decToBcd(Date));
	_Wire.endTransmission();
}

void DS3231::setMonth(byte Month) {
	// Sets the month
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x05);
	_Wire.write(decToBcd(Month));
	_Wire.endTransmission();
}

void DS3231::setYear(byte Year) {
	// Sets the year
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x06);
	_Wire.write(decToBcd(Year));
	_Wire.endTransmission();
}

void DS3231::setClockMode(bool h12) {
	// sets the mode to 12-hour (true) or 24-hour (false).
	// One thing that bothers me about how I've written this is that
	// if the read and right happen at the right hourly millisecnd,
	// the clock will be set back an hour. Not sure how to do it better,
	// though, and as long as one doesn't set the mode frequently it's
	// a very minimal risk.
	// It's zero risk if you call this BEFORE setting the hour, since
	// the setHour() function doesn't change this mode.

	byte temp_buffer;

	// Start by reading byte 0x02.
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x02);
	_Wire.endTransmission();
	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	temp_buffer = _Wire.read();

	// Set the flag to the requested value:
	if (h12) {
		temp_buffer = temp_buffer | 0b01000000;
	} else {
		temp_buffer = temp_buffer & 0b10111111;
	}

	// Write the byte
	_Wire.beginTransmission(CLOCK_ADDRESS);
	_Wire.write(0x02);
	_Wire.write(temp_buffer);
	_Wire.endTransmission();
}

float DS3231::getTemperature() {
	// Checks the internal thermometer on the DS3231 and returns the
	// temperature as a floating-point value.

  // Updated / modified a tiny bit from "Coding Badly" and "Tri-Again"
  // http://forum.arduino.cc/index.php/topic,22301.0.html

  byte tMSB, tLSB;
  float temp3231;

  // temp registers (11h-12h) get updated automatically every 64s
  _Wire.beginTransmission(CLOCK_ADDRESS);
  _Wire.write(0x11);
  _Wire.endTransmission();
  _Wire.requestFrom(CLOCK_ADDRESS, 2);

  // Should I do more "if available" checks here?
  if(_Wire.available()) {
    tMSB = _Wire.read(); //2's complement int portion
    tLSB = _Wire.read(); //fraction portion

    int16_t  itemp  = ( tMSB << 8 | (tLSB & 0xC0) );  // Shift upper byte, add lower
    temp3231 = ( (float)itemp / 256.0 );              // Scale and return
  }
  else {
    temp3231 = -9999; // Impossible temperature; error value
  }

  return temp3231;
}

bool DS3231::status32Hz(){
	byte temp_buffer = readControlByte(1);
	temp_buffer = temp_buffer & 0b00001000;
	if(temp_buffer){
		return 1;
	}else{
		return 0;
	}
}

void DS3231::enableOscillator(bool TF, bool battery, byte frequency) {
	// turns oscillator on or off. True is on, false is off.
	// if battery is true, turns on even for battery-only operation,
	// otherwise turns off if Vcc is off.
	// frequency must be 0, 1, 2, or 3.
	// 0 = 1 Hz
	// 1 = 1.024 kHz
	// 2 = 4.096 kHz
	// 3 = 8.192 kHz (Default if frequency byte is out of range)
	if (frequency > 3) frequency = 3;
	// read control byte in, but zero out current state of RS2 and RS1.
	byte temp_buffer = readControlByte(0) & 0b11100111;
	if (battery) {
		// turn on BBSQW flag
		temp_buffer = temp_buffer | 0b01000000;
	} else {
		// turn off BBSQW flag
		temp_buffer = temp_buffer & 0b10111111;
	}
	if (TF) {
		// set ~EOSC to 0 and INTCN to zero.
		temp_buffer = temp_buffer & 0b01111011;
	} else {
		// set ~EOSC to 1, leave INTCN as is.
		temp_buffer = temp_buffer | 0b10000000;
	}
	// shift frequency into bits 3 and 4 and set.
	frequency = frequency << 3;
	temp_buffer = temp_buffer | frequency;
	// And write the control bits
	writeControlByte(temp_buffer, 0);
}

void DS3231::enable32kHz(bool TF) {
	// turn 32kHz pin on or off
	byte temp_buffer = readControlByte(1);
	if (TF) {
		// turn on 32kHz pin
		temp_buffer = temp_buffer | 0b00001000;
	} else {
		// turn off 32kHz pin
		temp_buffer = temp_buffer & 0b11110111;
	}
	writeControlByte(temp_buffer, 1);
}

bool DS3231::oscillatorCheck() {
	// Returns false if the oscillator has been off for some reason.
	// If this is the case, the time is probably not correct.
	byte temp_buffer = readControlByte(1);
	bool result = true;
	if (temp_buffer & 0b10000000) {
		// Oscillator Stop Flag (OSF) is set, so return false.
		result = false;
	}
	return result;
}

/*****************************************
	Private Functions
 *****************************************/

byte DS3231::decToBcd(byte val) {
// Convert normal decimal numbers to binary coded decimal
	return ( (val/10*16) + (val%10) );
}

byte DS3231::bcdToDec(byte val) {
// Convert binary coded decimal to normal decimal numbers
	return ( (val/16*10) + (val%16) );
}

byte DS3231::readControlByte(bool which) {
	// Read selected control byte
	// first byte (0) is 0x0e, second (1) is 0x0f
	_Wire.beginTransmission(CLOCK_ADDRESS);
	if (which) {
		// second control byte
		_Wire.write(0x0f);
	} else {
		// first control byte
		_Wire.write(0x0e);
	}
	_Wire.endTransmission();
	_Wire.requestFrom(CLOCK_ADDRESS, 1);
	return _Wire.read();
}

void DS3231::writeControlByte(byte control, bool which) {
	// Write the selected control byte.
	// which=false -> 0x0e, true->0x0f.
	_Wire.beginTransmission(CLOCK_ADDRESS);
	if (which) {
		_Wire.write(0x0f);
	} else {
		_Wire.write(0x0e);
	}
	_Wire.write(control);
	_Wire.endTransmission();
}
