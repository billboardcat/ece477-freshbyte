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

#define I2C hi2c1

/**************************************************************************/
/*!
    @brief  Enable threshold interrupt and set the threshold values
*/
/**************************************************************************/

void VCNL4010_enable_Interrupt() {
  VCNL4010_setLEDcurrent(20);
  //Set INT_THRES_EN
  uint8_t temp = VCNL4010_read8(VCNL4010_INTCONTROL);
  temp &= 0xf0; //clear first 4 bits
  temp |= VCNL4010_INT_THRES_EN; //set INT_THRES_EN bit
  VCNL4010_write8(VCNL4010_INTCONTROL, temp);

  //set low threshold - not really using... so set to 0.
  VCNL4010_write16(VCNL4010_LOWTHRESHOLD, 0);

  //set high threshold
  VCNL4010_write16(VCNL4010_HITHRESHOLD, 2500);

  //set proximity reading to be continuous
  temp = VCNL4010_read8(VCNL4010_COMMAND);
  temp |= VCNL4010_SELFTIMED_EN;
  temp |= VCNL4010_PERIOD_PROX_EN;
  VCNL4010_write8(VCNL4010_COMMAND, temp);

}

/**************************************************************************/
/*!
    @brief  Clear Interrupt Status Register
*/
/**************************************************************************/

void VCNL4010_ack_ISR() {
  //Set INT_THRES_EN
  uint8_t temp = VCNL4010_read8(VCNL4010_INTSTAT);
  temp |= 0x0f; //clear 4 interrupt bits by writing 1???
  VCNL4010_write8(VCNL4010_INTSTAT, temp);

}

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
*/
/**************************************************************************/

uint16_t VCNL4010_readProximity(void) {
  uint8_t i = VCNL4010_read8(VCNL4010_INTSTAT);
  uint32_t attempts = 0;
  i &= ~0x08;
  VCNL4010_write8(VCNL4010_INTSTAT, i);

  uint8_t temp = VCNL4010_read8(VCNL4010_COMMAND);
  //start one shot prox. reading
  temp |= VCNL4010_MEASUREPROXIMITY;
  //turn off continuous measurment
  temp &= ~(VCNL4010_SELFTIMED_EN | VCNL4010_PERIOD_PROX_EN);
  VCNL4010_write8(VCNL4010_COMMAND, temp);
  while (attempts < VCNL4010_MAX_SENSOR_RDY_WAIT) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = VCNL4010_read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_PROXIMITYREADY) {
      uint16_t retval = VCNL4010_read16(VCNL4010_PROXIMITYDATA);

      //turn on continuous measurement
      temp |= (VCNL4010_SELFTIMED_EN | VCNL4010_PERIOD_PROX_EN);
      VCNL4010_write8(VCNL4010_COMMAND, temp);

      return retval;
    }
    HAL_Delay(1);
    attempts++;
  }
  return VCNL4010_FAIL;
}

/**************************************************************************/
/*!
    @brief  Get ambient light measurement
    @return Raw 16-bit reading value, unit-less!
*/
/**************************************************************************/

uint16_t VCNL4010_readAmbient(void) {
  uint32_t attempts = 0;
  uint8_t i = VCNL4010_read8(VCNL4010_INTSTAT);
  i &= ~0x40;
  VCNL4010_write8(VCNL4010_INTSTAT, i);

  VCNL4010_write8(VCNL4010_COMMAND, VCNL4010_MEASUREAMBIENT);
  while (attempts < VCNL4010_MAX_SENSOR_RDY_WAIT) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = VCNL4010_read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_AMBIENTREADY) {
      return VCNL4010_read16(VCNL4010_AMBIENTDATA);
    }
    HAL_Delay(1);
    attempts++;
  }
  return VCNL4010_FAIL;
}

HAL_StatusTypeDef VCNL4010_write8(uint8_t subAddress, uint8_t data)
{
	return HAL_I2C_Mem_Write(&I2C, (VCNL4010_I2CADDR_DEFAULT << 1), subAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

}

HAL_StatusTypeDef VCNL4010_write16(uint8_t subAddress, uint16_t data)
{

  uint8_t temp[2];
  temp[0] = (uint8_t) 0xFF & (data >> 8); //high byte is stored first
  temp[1] = (uint8_t) (0xFF & data);      //low byte is stored second

  return HAL_I2C_Mem_Write(&I2C, (VCNL4010_I2CADDR_DEFAULT << 1), subAddress, I2C_MEMADD_SIZE_8BIT, temp, 2, HAL_MAX_DELAY);

}

uint8_t VCNL4010_read8(uint8_t subAddress){

	uint8_t data;
	HAL_I2C_Mem_Read(&I2C, (VCNL4010_I2CADDR_DEFAULT << 1), subAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
	return ((uint8_t) data);
}

uint16_t VCNL4010_read16(uint8_t subAddress){

	uint8_t data[2];
	HAL_I2C_Mem_Read(&I2C, (VCNL4010_I2CADDR_DEFAULT << 1), subAddress, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
	return ((uint16_t) data[0] << 8) | data[1];
}
