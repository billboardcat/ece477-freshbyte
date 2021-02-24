/*
 * bq27441.c
 *
 *  Created on: Feb 22, 2021
 *      Author: pcrain
 *      STM32L0 interfacing with BQ27441 PMIC
 *      functions that start with BQ27441_ were copied + refactored from the c++ library
 *      here - https://github.com/sparkfun/Battery_Babysitter
 */


#include "bq27441.h"
#include <stdlib.h>

// For debug
#include <stdio.h>
#include "stm32l0xx_hal.h"

extern I2C_HandleTypeDef hi2c1; // TODO: How do we handle this if I2C controller is changed?


int bq_init(){
	// return BAT_INIT_FAIL if not able to init (BAT_INIT_SUCCESS for success)

	int retval;

	/* === Set BQ Max Battery Capacity === */
	uint8_t capMSB = BAT_CAP_MAX >> 8;
	uint8_t capLSB = BAT_CAP_MAX & 0x00FF;
	uint8_t capacityData[2] = {capMSB, capLSB};
	retval = BQ27441_writeExtendedData(BQ27441_ID_STATE, 10, capacityData, 2);
	if (retval != BAT_INIT_SUCCESS)
		return BAT_INIT_FAIL;


	return BAT_INIT_SUCCESS;
}

/*****************************************************************************
 ********************** Battery Characteristics Functions ********************
 *****************************************************************************/
// Reads and returns the battery voltage
uint16_t BQ27441_voltage(void)
{
	return BQ27441_readWord(BQ27441_COMMAND_VOLTAGE);
}

// Reads and returns the specified current measurement
int16_t BQ27441_current(current_measure type)
{
	int16_t current = 0;
	switch (type)
	{
	case AVG:
		current = (int16_t) BQ27441_readWord(BQ27441_COMMAND_AVG_CURRENT);
		break;
	case STBY:
		current = (int16_t) BQ27441_readWord(BQ27441_COMMAND_STDBY_CURRENT);
		break;
	case MAX:
		current = (int16_t) BQ27441_readWord(BQ27441_COMMAND_MAX_CURRENT);
		break;
	}

	return current;
}

// Reads and returns the specified capacity measurement
uint16_t BQ27441_capacity(capacity_measure type)
{
	uint16_t capacity = 0;
	switch (type)
	{
	case REMAIN:
		return BQ27441_readWord(BQ27441_COMMAND_REM_CAPACITY);
		break;
	case FULL:
		return BQ27441_readWord(BQ27441_COMMAND_FULL_CAPACITY);
		break;
	case AVAIL:
		capacity = BQ27441_readWord(BQ27441_COMMAND_NOM_CAPACITY);
		break;
	case AVAIL_FULL:
		capacity = BQ27441_readWord(BQ27441_COMMAND_AVAIL_CAPACITY);
		break;
	case REMAIN_F:
		capacity = BQ27441_readWord(BQ27441_COMMAND_REM_CAP_FIL);
		break;
	case REMAIN_UF:
		capacity = BQ27441_readWord(BQ27441_COMMAND_REM_CAP_UNFL);
		break;
	case FULL_F:
		capacity = BQ27441_readWord(BQ27441_COMMAND_FULL_CAP_FIL);
		break;
	case FULL_UF:
		capacity = BQ27441_readWord(BQ27441_COMMAND_FULL_CAP_UNFL);
		break;
	case DESIGN:
		capacity = BQ27441_readWord(BQ27441_EXTENDED_CAPACITY);
	}

	return capacity;
}

// Reads and returns measured average power
int16_t BQ27441_power(void)
{
	return (int16_t) BQ27441_readWord(BQ27441_COMMAND_AVG_POWER);
}

// Reads and returns specified state of charge measurement
uint16_t BQ27441_soc(soc_measure type)
{
	uint16_t socRet = 0;
	switch (type)
	{
	case FILTERED:
		socRet = BQ27441_readWord(BQ27441_COMMAND_SOC);
		break;
	case UNFILTERED:
		socRet = BQ27441_readWord(BQ27441_COMMAND_SOC_UNFL);
		break;
	}

	return socRet;
}

// Reads and returns specified state of health measurement
uint8_t BQ27441_soh(soh_measure type)
{
	uint16_t sohRaw = BQ27441_readWord(BQ27441_COMMAND_SOH);
	uint8_t sohStatus = sohRaw >> 8;
	uint8_t sohPercent = sohRaw & 0x00FF;

	if (type == PERCENT)
		return sohPercent;
	else
		return sohStatus;
}

// Reads and returns specified temperature measurement
uint16_t BQ27441_temperature(temp_measure type)
{
	uint16_t temp = 0;
	switch (type)
	{
	case BATTERY:
		temp = BQ27441_readWord(BQ27441_COMMAND_TEMP);
		break;
	case INTERNAL_TEMP:
		temp = BQ27441_readWord(BQ27441_COMMAND_INT_TEMP);
		break;
	}
	return temp;
}


/*****************************************************************************
 ************************** Extended Data Commands ***************************
 *****************************************************************************/

// Write a specified number of bytes to extended data specifying a
// class ID, position offset.
int BQ27441_writeExtendedData(uint8_t classID, uint8_t offset, uint8_t * data, uint8_t len)
{
	HAL_StatusTypeDef ret;	// I2C return status

	if (len > 32)
		return BAT_INIT_FAIL;

	ret = BQ27441_blockDataControl();
	if (ret != HAL_OK) // // enable block data memory control
		return BAT_INIT_FAIL; // Return false if enable fails
	if (BQ27441_blockDataClass(classID) != HAL_OK) // Write class ID using DataBlockClass()
		return BAT_INIT_FAIL;

	BQ27441_blockDataOffset(offset / 32); // Write 32-bit block offset (usually 0)
	BQ27441_computeBlockChecksum(); // Compute checksum going in
	uint8_t oldCsum = BQ27441_blockDataChecksum();

	// Write data bytes:
	for (int i = 0; i < len; i++)
	{
		// Write to offset, mod 32 if offset is greater than 32
		// The blockDataOffset above sets the 32-bit block
		BQ27441_writeBlockData((offset % 32) + i, data[i]);
	}

	// Write new checksum using BlockDataChecksum (0x60)
	uint8_t newCsum = BQ27441_computeBlockChecksum(); // Compute the new checksum
	BQ27441_writeBlockChecksum(newCsum);


	return BAT_INIT_SUCCESS;
}

// Read a 16-bit command word from the BQ27441-G1A
uint16_t BQ27441_readWord(uint16_t subAddress)
{
	uint8_t data[2];
//	i2cReadBytes(subAddress, data, 2);
	HAL_I2C_Mem_Read(&hi2c1, (BQ_ADDR << 1), subAddress, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
	return ((uint16_t) data[1] << 8) | data[0];
}

// Issue a BlockDataControl() command to enable BlockData access
HAL_StatusTypeDef BQ27441_blockDataControl(void)
{
	uint8_t enableByte = 0x00;
//	return i2cWriteBytes(BQ27441_EXTENDED_CONTROL, &enableByte, 1);
	return HAL_I2C_Mem_Write(&hi2c1, (BQ_ADDR << 1), BQ27441_EXTENDED_CONTROL, I2C_MEMADD_SIZE_8BIT, &enableByte, 1, HAL_MAX_DELAY);
}

// Issue a DataClass() command to set the data class to be accessed
HAL_StatusTypeDef BQ27441_blockDataClass(uint8_t id)
{
//	return i2cWriteBytes(BQ27441_EXTENDED_DATACLASS, &id, 1);
	return HAL_I2C_Mem_Write(&hi2c1, (BQ_ADDR << 1), BQ27441_EXTENDED_DATACLASS, I2C_MEMADD_SIZE_8BIT, &id, 1, HAL_MAX_DELAY);

}

// Issue a DataBlock() command to set the data block to be accessed
HAL_StatusTypeDef BQ27441_blockDataOffset(uint8_t offset)
{
//	return i2cWriteBytes(BQ27441_EXTENDED_DATABLOCK, &offset, 1);
	return HAL_I2C_Mem_Write(&hi2c1, (BQ_ADDR << 1), BQ27441_EXTENDED_DATABLOCK, I2C_MEMADD_SIZE_8BIT, &offset, 1, HAL_MAX_DELAY);

}

// Read the current checksum using BlockDataCheckSum()
uint8_t BQ27441_blockDataChecksum(void)
{
	uint8_t csum;
//	i2cReadBytes(BQ27441_EXTENDED_CHECKSUM, &csum, 1);
	HAL_I2C_Mem_Read(&hi2c1, (BQ_ADDR << 1), BQ27441_EXTENDED_CHECKSUM, I2C_MEMADD_SIZE_8BIT, &csum, 1, HAL_MAX_DELAY);
	return csum;
}

// Read all 32 bytes of the loaded extended data and compute a
// checksum based on the values.
uint8_t BQ27441_computeBlockChecksum(void)
{
	uint8_t data[32];
	// i2cReadBytes(BQ27441_EXTENDED_BLOCKDATA, data, 32);
	// TODO: Does this work? - unsure about 32 bytes
	HAL_I2C_Mem_Read(&hi2c1, (BQ_ADDR << 1), BQ27441_EXTENDED_CHECKSUM, I2C_MEMADD_SIZE_8BIT, data, 32, HAL_MAX_DELAY);


	uint8_t csum = 0;
	for (int i=0; i<32; i++)
	{
		csum += data[i];
	}
	csum = 255 - csum;

	return csum;
}

// Use the BlockDataCheckSum() command to write a checksum value
HAL_StatusTypeDef BQ27441_writeBlockChecksum(uint8_t csum)
{
	//return i2cWriteBytes(BQ27441_EXTENDED_CHECKSUM, &csum, 1);
	return HAL_I2C_Mem_Write(&hi2c1, (BQ_ADDR << 1), BQ27441_EXTENDED_CHECKSUM, I2C_MEMADD_SIZE_8BIT, &csum, 1, HAL_MAX_DELAY);
}

// Use BlockData() to write a byte to an offset of the loaded data
HAL_StatusTypeDef BQ27441_writeBlockData(uint8_t offset, uint8_t data)
{
	uint8_t address = offset + BQ27441_EXTENDED_BLOCKDATA;
	//return i2cWriteBytes(address, &data, 1);
	return HAL_I2C_Mem_Write(&hi2c1, (BQ_ADDR << 1), address, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

}
