/*

An Arduino library for the Sensirion SHTC3 humidity and temerature sensor

*/

#include "DR_SHTC3.h"

SHTC3_Status_TypeDef SHTC3::sendCommand(SHTC3_Commands_TypeDef cmd)
{
	uint8_t res = 0;

	_wire->beginTransmission(SHTC3_ADDR_7BIT);
	_wire->write((((uint16_t)cmd) >> 8));
	_wire->write((((uint16_t)cmd) & 0x00FF));
	res = _wire->endTransmission();

	if (res)
	{
		return SHTC3_Status_Error;
	}

	return SHTC3_Status_Nominal;
}

SHTC3_Status_TypeDef SHTC3::sendCommand(SHTC3_MeasurementModes_TypeDef cmd)
{
	return sendCommand((SHTC3_Commands_TypeDef)cmd);
}

SHTC3::SHTC3()
{
	_mode = SHTC3_CMD_CSE_TF_NPM; // My default pick


	passRHcrc = false;
	passTcrc = false;
	passIDcrc = false;

	RH = 0x00;
	T = 0x00;
	ID = 0x00;
}

float SHTC3::toDegC()
{
	return SHTC3_raw2DegC(T);
}

float SHTC3::toDegF()
{
	return SHTC3_raw2DegF(T);
}

float SHTC3::toPercent()
{
	return SHTC3_raw2Percent(RH);
}

SHTC3_Status_TypeDef SHTC3::begin(TwoWire &wirePort)
{
	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;
	;

	_wire = &wirePort; // Associate the I2C port

	return retval;
}

SHTC3_Status_TypeDef SHTC3::softReset()
{
	return sendCommand(SHTC3_CMD_SFT_RST);
	delayMicroseconds(500);
}

SHTC3_Status_TypeDef SHTC3::update()
{
	SHTC3_Status_TypeDef retval = SHTC3_Status_Nominal;

	const uint8_t numBytesRequest = 6;
	uint8_t numBytesRx = 0;

	uint8_t RHhb = 0x00;
	uint8_t RHlb = 0x00;
	uint8_t RHcs = 0x00;

	uint8_t Thb = 0x00;
	uint8_t Tlb = 0x00;
	uint8_t Tcs = 0x00;

	retval = sendCommand(_mode); // Send the appropriate command - Note: incorrect commands are excluded by the 'setMode' command and _mode is a protected variable
	if (retval != SHTC3_Status_Nominal)
	{
		return retval;
	}

	switch (_mode) // Handle the two different ways of waiting for a measurement (polling or clock stretching)
	{
	case SHTC3_CMD_CSE_RHF_NPM:
	case SHTC3_CMD_CSE_RHF_LPM:
	case SHTC3_CMD_CSE_TF_NPM:
	case SHTC3_CMD_CSE_TF_LPM:	   // Address+read will yield an ACK and then clock stretching will occur
		numBytesRx = _wire->requestFrom((uint8_t)SHTC3_ADDR_7BIT, numBytesRequest);
		break;

	// CSD modes not yet supported (polling - need to figure out how to repeatedly send just address+read and look for ACK)
	default:
		return SHTC3_Status_Error; // You really should never get to this code because setMode disallows non-approved values of _mode (type SHTC3_MeasurementModes_TypeDef)
		break;
	}

	// Now handle the received data
	if (numBytesRx != numBytesRequest)
	{
		return SHTC3_Status_Error;
	} // Hopefully we got the right number of bytes

	switch (_mode) // Switch for the order of the returned results
	{
	case SHTC3_CMD_CSE_RHF_NPM:
	case SHTC3_CMD_CSE_RHF_LPM:
	case SHTC3_CMD_CSD_RHF_NPM:
	case SHTC3_CMD_CSD_RHF_LPM:
		// RH First
		RHhb = _wire->read();
		RHlb = _wire->read();
		RHcs = _wire->read();

		Thb = _wire->read();
		Tlb = _wire->read();
		Tcs = _wire->read();
		break;

	case SHTC3_CMD_CSE_TF_NPM:
	case SHTC3_CMD_CSE_TF_LPM:
	case SHTC3_CMD_CSD_TF_NPM:
	case SHTC3_CMD_CSD_TF_LPM:
		// T First
		Thb = _wire->read();
		Tlb = _wire->read();
		Tcs = _wire->read();

		RHhb = _wire->read();
		RHlb = _wire->read();
		RHcs = _wire->read();
		break;

	default:
		return SHTC3_Status_Error; // Again, you should never experience this section of code
		break;
	}

	// Update values
	RH = ((uint16_t)RHhb << 8) | ((uint16_t)RHlb << 0);
	T = ((uint16_t)Thb << 8) | ((uint16_t)Tlb << 0);

	passRHcrc = false;
	passTcrc = false;

	if (checkCRC(RH, RHcs) == SHTC3_Status_Nominal)
	{
		passRHcrc = true;
	}
	if (checkCRC(T, Tcs) == SHTC3_Status_Nominal)
	{
		passTcrc = true;
	}

	if (retval != SHTC3_Status_Nominal)
	{
		return retval;
	}

	return retval;
}

SHTC3_Status_TypeDef SHTC3::checkCRC(uint16_t packet, uint8_t cs)
{
	uint8_t upper = packet >> 8;
	uint8_t lower = packet & 0x00FF;
	uint8_t data[2] = {upper, lower};
	uint8_t crc = 0xFF;
	uint8_t poly = 0x31;

	for (uint8_t indi = 0; indi < 2; indi++)
	{
		crc ^= data[indi];

		for (uint8_t indj = 0; indj < 8; indj++)
		{
			if (crc & 0x80)
			{
				crc = (uint8_t)((crc << 1) ^ poly);
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	if (cs ^ crc)
	{
		return SHTC3_Status_CRC_Fail;
	}
	return SHTC3_Status_Nominal;
}

float SHTC3_raw2DegC(uint16_t T)
{
	return -45 + 175 * ((float)T / 65535);
}

float SHTC3_raw2DegF(uint16_t T)
{
	return SHTC3_raw2DegC(T) * (9.0 / 5) + 32.0;
}

float SHTC3_raw2Percent(uint16_t RH)
{
	return 100 * ((float)RH / 65535);
}
