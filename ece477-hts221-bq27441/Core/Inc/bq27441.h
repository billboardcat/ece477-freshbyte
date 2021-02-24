/*
 * bq27441.h
 *
 *  Created on: Feb 22, 2021
 *      Author: pcrain
 *      Based on Sparkfun C++  arduino library
 *      https://github.com/sparkfun/SparkFun_BQ27441_Arduino_Library/tree/master/src
 *      (see header for more info on register addresses as needed)
 */

#ifndef SRC_BQ27441_H_
#define SRC_BQ27441_H_

#include "stm32l0xx_hal.h"
#include "BQ27441_Definitions.h"

/* === Battery Init SUCCESS == */
#define BAT_INIT_SUCCESS 1
#define BAT_INIT_FAIL -1


/* === Battery Capacity == */
#define BAT_CAP_MAX 2500 //2500mah rated battery capacity


/* === I2C ADDRESS == */
#define BQ_ADDR 0x55 //aa or ab depening on write or read... (after left shift)

/* === REGISTER ADDRESSES ===  */


/* === FUNCTION DECLARATIONS === */
int bq_init();

int BQ27441_writeExtendedData(uint8_t classID, uint8_t offset, uint8_t * data, uint8_t len);
HAL_StatusTypeDef BQ27441_blockDataControl(void);
HAL_StatusTypeDef BQ27441_blockDataClass(uint8_t id);
HAL_StatusTypeDef BQ27441_blockDataOffset(uint8_t offset);
uint8_t BQ27441_blockDataChecksum(void);
uint8_t BQ27441_computeBlockChecksum(void);
HAL_StatusTypeDef BQ27441_writeBlockChecksum(uint8_t csum);
HAL_StatusTypeDef BQ27441_writeBlockData(uint8_t offset, uint8_t data);

uint16_t BQ27441_voltage(void);
int16_t BQ27441_current(current_measure type);
uint16_t BQ27441_capacity(capacity_measure type);
int16_t BQ27441_power(void);
uint16_t BQ27441_soc(soc_measure type);
uint8_t BQ27441_soh(soh_measure type);
uint16_t BQ27441_temperature(temp_measure type);
uint16_t BQ27441_readWord(uint16_t subAddress);

#endif /* SRC_BQ27441_H_ */
