/*
 * DS3231.h
 *
 * Arduino Library for the DS3231 Real-Time Clock chip
 *
 * (c) Eric Ayars
 * 4/1/11
 * released into the public domain. If you use this, please let me know
 * (just out of pure curiosity!) by sending me an email:
 * eric@ayars.org
 *
 * Changed the parameter type in isleapYear() to uint16_t from uint8_t
 * for two reasons: the function uses 16-bit arithmetic, i.e., (y % 400); and
 * one of the calling functions sends a 16-bit parameter.
 * David Sparks
 * 08 Sept 2022
 *
 */

// Modified by Andy Wickert 5/15/11: Spliced in stuff from RTClib
// Modified by Simon Gassner 11/28/2017: Changed Term "PM" to "PM_time" for compability with SAMD Processors
#ifndef DR_DS3231_h
#define DR_DS3231_h

// Changed the following to work on 1.0
//#include "WProgram.h"
#include <Arduino.h>
#include <time.h>
#include <Wire.h>

// Eric's original code is everything below this line
class DS3231 {
	public:

		//Constructor
		DS3231();
		DS3231(TwoWire & w);

		TwoWire & _Wire;

		// Time-retrieval functions

		// the get*() functions retrieve current values of the registers.
		byte getSecond();
		byte getMinute();
		byte getHour(bool& h12, bool& PM_time);
			// In addition to returning the hour register, this function
			// returns the values of the 12/24-hour flag and the AM/PM flag.
		byte getDoW();
		byte getDate();
		byte getMonth(bool& Century);
			// Also sets the flag indicating century roll-over.
		byte getYear();
			// Last 2 digits only

		// Time-setting functions
		// Note that none of these check for sensibility: You can set the
		// date to July 42nd and strange things will probably result.

		// set epoch function gives the epoch as parameter and feeds the RTC
		// epoch = UnixTime and starts at 01.01.1970 00:00:00
		void setEpoch(time_t epoch = 0, bool flag_localtime = false);

		void setSecond(byte Second);
			// In addition to setting the seconds, this clears the
			// "Oscillator Stop Flag".
		void setMinute(byte Minute);
			// Sets the minute
		void setHour(byte Hour);
			// Sets the hour
		void setDoW(byte DoW);
			// Sets the Day of the Week (1-7);
		void setDate(byte Date);
			// Sets the Date of the Month
		void setMonth(byte Month);
			// Sets the Month of the year
		void setYear(byte Year);
			// Last two digits of the year
		void setClockMode(bool h12);
			// Set 12/24h mode. True is 12-h, false is 24-hour.

		// Temperature function

		float getTemperature();

		bool status32Hz();		

		// Oscillator functions

		void enableOscillator(bool TF, bool battery, byte frequency);
			// turns oscillator on or off. True is on, false is off.
			// if battery is true, turns on even for battery-only operation,
			// otherwise turns off if Vcc is off.
			// frequency must be 0, 1, 2, or 3.
			// 0 = 1 Hz
			// 1 = 1.024 kHz
			// 2 = 4.096 kHz
			// 3 = 8.192 kHz (Default if frequency byte is out of range);
		void enable32kHz(bool TF);
			// Turns the 32kHz output pin on (true); or off (false).
		bool oscillatorCheck();;
			// Checks the status of the OSF (Oscillator Stop Flag);.
			// If this returns false, then the clock is probably not
			// giving you the correct time.
			// The OSF is cleared by function setSecond();.

	private:

		byte decToBcd(byte val);
			// Convert normal decimal numbers to binary coded decimal
		byte bcdToDec(byte val);
			// Convert binary coded decimal to normal decimal numbers

	protected:

		byte readControlByte(bool which);
			// Read selected control byte: (0); reads 0x0e, (1) reads 0x0f
		void writeControlByte(byte control, bool which);
			// Write the selected control byte.
			// which == false -> 0x0e, true->0x0f.

};

#endif
