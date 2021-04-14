/*
 * vcnl4010.h
 *
 *  Created on: Feb 28, 2021
 *      Author: pcrain
 *
 *      Based off adafruit Arduino library
 *      https://github.com/adafruit/Adafruit_VCNL4010/blob/master/Adafruit_VCNL4010.h
 */

#ifndef SRC_VCNL4010_H_
#define SRC_VCNL4010_H_

#include "stm32l0xx_hal.h"

#define VCNL4010_I2CADDR_DEFAULT 0x13 ///< I2C address of the sensor

/** Registers */
#define VCNL4010_COMMAND 0x80          ///< Command
#define VCNL4010_PRODUCTID 0x81        ///< Product ID Revision
#define VCNL4010_PROXRATE 0x82         ///< Proximity rate
#define VCNL4010_IRLED 0x83            ///< IR LED current
#define VCNL4010_AMBIENTPARAMETER 0x84 ///< Ambient light parameter
#define VCNL4010_AMBIENTDATA 0x85      ///< Ambient light result (16 bits)
#define VCNL4010_PROXIMITYDATA 0x87    ///< Proximity result (16 bits)
#define VCNL4010_INTCONTROL 0x89       ///< Interrupt control
#define VCNL4010_LOWTHRESHOLD 0x8A     ///< Low threshold value (16 bits)
#define VCNL4010_HITHRESHOLD 0x8C      ///< High threshold value (16 bits)
#define VCNL4010_INTSTAT 0x8E          ///< Interrupt status
#define VCNL4010_MODTIMING 0x8F ///< Proximity modulator timing adjustment

/** Proximity measurement rate */
typedef enum {
  VCNL4010_1_95 = 0,    // 1.95     measurements/sec (Default)
  VCNL4010_3_90625 = 1, // 3.90625  measurements/sec
  VCNL4010_7_8125 = 2,  // 7.8125   measurements/sec
  VCNL4010_16_625 = 3,  // 16.625   measurements/sec
  VCNL4010_31_25 = 4,   // 31.25    measurements/sec
  VCNL4010_62_5 = 5,    // 62.5     measurements/sec
  VCNL4010_125 = 6,     // 125      measurements/sec
  VCNL4010_250 = 7,     // 250      measurements/sec
} vcnl4010_freq;

/** Values for command register */
#define VCNL4010_MEASUREPROXIMITY                                              \
  0x08 ///< Start a single on-demand proximity measurement
#define VCNL4010_MEASUREAMBIENT                                                \
  0x10 ///< Start a single on-demand ambient light measurement
#define VCNL4010_PROXIMITYREADY                                                \
  0x20 ///< Read-only - Value = 1 when proximity measurement data is available
#define VCNL4010_AMBIENTREADY                                                  \
  0x40 ///< Read-only - Value = 1 when ambient light measurement data is
       ///< available

/** Functions */
void VCNL4010_setLEDcurrent(uint8_t current_10mA);
uint8_t VCNL4010_getLEDcurrent(void);
void VCNL4010_setFrequency(vcnl4010_freq freq);
uint16_t VCNL4010_readProximity(void);
uint16_t VCNL4010_readAmbient(void);

HAL_StatusTypeDef VCNL4010_write8(uint8_t subAddress, uint8_t data);
HAL_StatusTypeDef VCNL4010_write16(uint8_t subAddress, uint16_t data);
uint8_t VCNL4010_read8(uint8_t subAddress);
uint16_t VCNL4010_read16(uint8_t subAddress);

void VCNL4010_enable_Interrupt();
void VCNL4010_ack_ISR();

#endif /* SRC_VCNL4010_H_ */
