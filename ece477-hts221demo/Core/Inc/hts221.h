/*
 * hts221.h
 *
 *  Created on: Feb 14, 2021
 *      Author: jimmysung + pcrain
 */

#ifndef INC_HTS221_H_
#define INC_HTS221_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"

/* === I2C ADDRESS == */
#define HTS_ADDR 0x5F

/* === REGISTER ADDRESSES ===  */
#define HTS_WHO_AM_I 0x0F
#define HTS_AV_CONF 0x10
#define HTS_CTRL_REG1 0x20
#define HTS_CTRL_REG2 0x21
#define HTS_CTRL_REG3 0x22
#define HTS_STATUS_REG 0x27
#define HTS_HUMIDITY_OUT_L 0x28
#define HTS_HUMIDITY_OUT_H 0x29
#define HTS_TEMP_OUT_L 0x2A
#define HTS_TEMP_OUT_H 0x2B
#define HTS_CAL_H0_rH_x2 0x30
#define HTS_CAL_H1_rH_x2 0x31
#define HTS_CAL_T0_degC_x8 0x32
#define HTS_CAL_T1_degC_x8 0x33
#define HTS_CAL_T1_T0_msb 0x35

#define HTS_CAL_H0_T0_OUT_L 0x36
#define HTS_CAL_H0_T0_OUT_H 0x37

#define HTS_CAL_H1_T0_OUT_L 0x3A
#define HTS_CAL_H1_T0_OUT_H 0x3B

#define HTS_CAL_T0_OUT_L 0x3C
#define HTS_CAL_T0_OUT_H 0x3D

#define HTS_CAL_T1_OUT_L 0x3E
#define HTS_CAL_T1_OUT_H 0x3F

/* === REGISTER ADDRESS BITS === */
// Let's just add the ones that we'll actually be using to keep it clean
#define HTS_CTRL_REG1_PD 		(0x80)
#define HTS_CTRL_REG1_BUD		(0x1 << 2)
#define HTS_CTRL_REG2_ONE_SHOT	0x1

/* === DATA STRUCTS === */
typedef struct HTS_Cal{
	int T0_OUT;
	float correction_factor;  //slope
	//= (delta of (T1_degC_x8 w/ MSB / 8) - T0 of the same) / (T1_OUT - T0_OUT)

	int offset; //T0_degC_x8 w/ MSB then divided by 8

	//zeroed_temp = T_out - T0_Out
	//temp_adj = (zeroed_temp * correction_factor) + offset
} HTS_Cal;

/* === FUNCTION DECLARATIONS === */
HTS_Cal * hts221_init(void);
int hts221_get_temp(char unit, HTS_Cal * hts_cal_data);
int hts221_calc_temp(int16_t T_OUT, HTS_Cal * hts_cal_data);


#ifdef __cplusplus
}
#endif

#endif /* INC_HTS221_H_ */
