/*

An Arduino library for the Sensirion SHTC3 humidity and temerature sensor

*/

#ifndef SF_SHTC3
#define SF_SHTC3

#include <Arduino.h>
#include <Wire.h>

#define SHTC3_ADDR_7BIT 0b1110000
#define SHTC3_ADDR_WRITE 0b11100000
#define SHTC3_ADDR_READ 0b11100001

#define SHTC3_MAX_CLOCK_FREQ 1000000

typedef enum
{
	SHTC3_CMD_WAKE = 0x3517,
	SHTC3_CMD_SLEEP = 0xB098,

	SHTC3_CMD_SFT_RST = 0x805D,

	SHTC3_CMD_READ_ID = 0xEFC8,
} SHTC3_Commands_TypeDef;

typedef enum
{
	SHTC3_CMD_CSE_RHF_NPM = 0x5C24, // Clock stretching, RH first, Normal power mode
	SHTC3_CMD_CSE_RHF_LPM = 0x44DE, // Clock stretching, RH first, Low power mode
	SHTC3_CMD_CSE_TF_NPM = 0x7CA2,	// Clock stretching, T first, Normal power mode
	SHTC3_CMD_CSE_TF_LPM = 0x6458,	// Clock stretching, T first, Low power mode

	SHTC3_CMD_CSD_RHF_NPM = 0x58E0, // Polling, RH first, Normal power mode
	SHTC3_CMD_CSD_RHF_LPM = 0x401A, // Polling, RH first, Low power mode
	SHTC3_CMD_CSD_TF_NPM = 0x7866,	// Polling, T first, Normal power mode
	SHTC3_CMD_CSD_TF_LPM = 0x609C	// Polling, T first, Low power mode
} SHTC3_MeasurementModes_TypeDef;

typedef enum
{
	SHTC3_Status_Nominal = 0, // The one and only "all is good" return value
	SHTC3_Status_Error,		  // The most general of error values - can mean anything depending on the context
	SHTC3_Status_CRC_Fail,	  // This return value means the computed checksum did not match the provided value
	SHTC3_Status_ID_Fail	  // This status means that the ID of the device did not match the format for SHTC3
} SHTC3_Status_TypeDef;

class SHTC3
{
private:
protected:
	TwoWire *_wire;

	SHTC3_MeasurementModes_TypeDef _mode;

	SHTC3_Status_TypeDef sendCommand(SHTC3_Commands_TypeDef cmd);
	SHTC3_Status_TypeDef sendCommand(SHTC3_MeasurementModes_TypeDef cmd);							// Overloaded version of send command to support the "measurement type" commands

public:
	SHTC3(); // Constructor

	SHTC3_Status_TypeDef lastStatus; // Stores the most recent status result for access by the user

	bool passRHcrc; // Indicates if the current value of RH has passed the CRC check
	bool passTcrc;	// Indicates if the current value of T has passed the CRC check
	bool passIDcrc; // Indicates if the current value of ID has passed the CRC check

	uint16_t RH; // Current raw RH data from the sensor
	uint16_t T;	 // Current raw T data from the sensor
	uint16_t ID; // Current raw ID data from the sensor

	float toDegC();	   // Returns the floating point value of T in deg C
	float toDegF();	   // Returns the floating point value of T in deg F
	float toPercent(); // Returns the floating point value of RH in % RH

	SHTC3_Status_TypeDef begin(TwoWire &wirePort = Wire);									   // Initializes the sensor
	SHTC3_Status_TypeDef softReset();														   // Resets the sensor into a known state through software
	
	SHTC3_Status_TypeDef update(); // Tells the sensor to take a measurement and updates the member variables of the object

	SHTC3_Status_TypeDef checkCRC(uint16_t packet, uint8_t cs); // Checks CRC values
};

float SHTC3_raw2DegC(uint16_t T);	  // Converts SHTC3 T data to deg C
float SHTC3_raw2DegF(uint16_t T);	  // Converts SHTC3 T data to deg F
float SHTC3_raw2Percent(uint16_t RH); // Converts SHTC3 RH data to % RH


#endif /* SF_SHTC3 */