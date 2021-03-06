/*
 * vcnl4010.c
 *
 *  Created on: Feb 28, 2021
 *      Author: pcrain
 *
 *      Based off of the Arduino library
 *      https://github.com/adafruit/VCNL4010/blob/master/VCNL4010.cpp
 *
 *      TODO: INT Pin can be setup to signal an interrupt when an event occurs
 */

#include "vcnl4010.h"
#include "stm32l0xx_hal.h"
#include "i2c.h"

#define I2C hi2c2

/**************************************************************************/
/*!
    @brief  Set the LED current.
    @param  current_10mA  Can be any value from 0 to 20, each number represents
   10 mA, so if you set it to 5, its 50mA. Minimum is 0 (0 mA, off), max is 20
   (200mA)
*/
/**************************************************************************/

void VCNL4010_setLEDcurrent(uint8_t current_10mA) {
  if (current_10mA > 20)
    current_10mA = 20;
  VCNL4010_write8(VCNL4010_IRLED, current_10mA);
}

/**************************************************************************/
/*!
    @brief  Get the LED current
    @return  The value directly from the register. Each bit represents 10mA so 5
   == 50mA
*/
/**************************************************************************/

uint8_t VCNL4010_getLEDcurrent(void) { return VCNL4010_read8(VCNL4010_IRLED); }

/**************************************************************************/
/*!
    @brief  Set the measurement signal frequency
    @param  freq Sets the measurement rate for proximity. Can be VCNL4010_1_95
   (1.95 measurements/s), VCNL4010_3_90625 (3.9062 meas/s), VCNL4010_7_8125
   (7.8125 meas/s), VCNL4010_16_625 (16.625 meas/s), VCNL4010_31_25 (31.25
   meas/s), VCNL4010_62_5 (62.5 meas/s), VCNL4010_125 (125 meas/s) or
   VCNL4010_250 (250 measurements/s)
*/
/**************************************************************************/

void VCNL4010_setFrequency(vcnl4010_freq freq) {
	VCNL4010_write8(VCNL4010_PROXRATE, freq);
}

/**************************************************************************/
/*!
    @brief  Get proximity measurement
    @return Raw 16-bit reading value, will vary with LED current, unit-less!
    TODO - Fix infinite loop, add timeout!
*/
/**************************************************************************/

uint16_t VCNL4010_readProximity(void) {
  uint8_t i = VCNL4010_read8(VCNL4010_INTSTAT);
  i &= ~0x80;
  VCNL4010_write8(VCNL4010_INTSTAT, i);

  VCNL4010_write8(VCNL4010_COMMAND, VCNL4010_MEASUREPROXIMITY);
  while (1) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = VCNL4010_read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_PROXIMITYREADY) {
      return VCNL4010_read16(VCNL4010_PROXIMITYDATA);
    }
//    delay(1);
  }
}

/**************************************************************************/
/*!
    @brief  Get ambient light measurement
    @return Raw 16-bit reading value, unit-less!
    TODO - Fix infinite loop, add timeout!
*/
/**************************************************************************/

uint16_t VCNL4010_readAmbient(void) {
  uint8_t i = VCNL4010_read8(VCNL4010_INTSTAT);
  i &= ~0x40;
  VCNL4010_write8(VCNL4010_INTSTAT, i);

  VCNL4010_write8(VCNL4010_COMMAND, VCNL4010_MEASUREAMBIENT);
  while (1) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = VCNL4010_read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_AMBIENTREADY) {
      return VCNL4010_read16(VCNL4010_AMBIENTDATA);
    }
  }
}

HAL_StatusTypeDef VCNL4010_write8(uint8_t subAddress, uint8_t data)
{
	return HAL_I2C_Mem_Write(&I2C, (VCNL4010_I2CADDR_DEFAULT << 1), subAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

}

uint8_t VCNL4010_read8(uint8_t subAddress){

	uint8_t data;
	HAL_I2C_Mem_Read(&I2C, (VCNL4010_I2CADDR_DEFAULT << 1), subAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
	return ((uint8_t) data);
}

uint16_t VCNL4010_read16(uint8_t subAddress){

	uint8_t data[2];
	HAL_I2C_Mem_Read(&I2C, (VCNL4010_I2CADDR_DEFAULT << 1), subAddress, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
	return ((uint16_t) data[1] << 8) | data[0];
}
