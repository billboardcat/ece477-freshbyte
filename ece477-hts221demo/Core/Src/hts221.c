/*
 * hts221.c
 *
 *  Created on: Feb 14, 2021
 *      Author: pcrain
 */

#include "hts221.h"
#include <stdlib.h>

extern I2C_HandleTypeDef hi2c1; // TODO: How do we handle this if I2C controller is changed?

HTS_Cal * hts221_init () {
	HAL_StatusTypeDef ret;	// I2C return status
	uint8_t buf[7];			// read buffer

	/* === Set HTS221 to wake mode === */
	buf[0] = HTS_CTRL_REG1;
	buf[1] = HTS_CTRL_REG1_PD;
	ret = HAL_I2C_Master_Transmit(&hi2c1, (HTS_ADDR << 1), buf, 2, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
	}
	else  {
		/* === Read in temperature calibration data === */
		// buf[0] = T0_degC_x8
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_T0_degC_x8, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[1] = T1_degC_x8
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_T1_degC_x8, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[2] = T1_T0_msb
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_T1_T0_msb, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[3] = HTS_CAL_T0_OUT_L
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_T0_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 3, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[4] = HTS_CAL_T0_OUT_H
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_T0_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 4, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[5] = HTS_CAL_T1_OUT_L
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_T1_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 5, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[6] = HTS_CAL_T1_OUT_H
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_T1_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 6, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		/* === Process temperature calibration data === */

		uint16_t T0_degC_R32 = buf[0];
		uint16_t T1_degC_R33 = buf[1];
		uint16_t T1_T0_msb 	= buf[2];
		uint16_t T0_OUT = (buf[3] | (buf[4] << 8));
		uint16_t T1_OUT = (buf[5] | (buf[6] << 8));

		// add msb's for 10 bit values

		T0_degC_R32 |= (T1_T0_msb & 0b0011) << 8;
		T1_degC_R33 |= (T1_T0_msb & 0b1100) << 6;

		// divide by 8
		T0_degC_R32 >>= 3;
		T1_degC_R33 >>= 3;

		// init struct to store calibration data
		HTS_Cal * hts_cal_data = malloc(sizeof(HTS_Cal));

		hts_cal_data->T0_OUT = T0_OUT;
		hts_cal_data->correction_factor = (T1_degC_R33 - T0_degC_R32) / (T1_OUT - T0_OUT);
		hts_cal_data->offset = T0_degC_R32;

		//zeroed_temp = T_out - T0_Out
		//temp_adj = (zeroed_temp * correction_factor) + offset

		return hts_cal_data;
	}

	return NULL;

	/* === Malloc struct and variables for struct === */
}

int hts221_get_temp(char unit, HTS_Cal * hts_cal_data){
	//Read values

	HAL_StatusTypeDef ret;	// I2C return status
	uint8_t buf[7];			// read buffer
	uint16_t T_OUT;			// T_OUT raw temperature reading
	int temp_adj;			// calibrated temperature value

	/* === Read in temperature data === */

	//TODO check that this loop is right...
	// Try three times for temp data to be ready
	for (int i = 0; i < 3; ++i) {
		// buf[0] = HTS_STATUS_REG
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_STATUS_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}
		if (buf[0] & 1){
			// new temp. data ready
			break;
		}
		//TODO - HAL_WAIT?
	}

	// buf[1] = HTS_TEMP_OUT_L
	ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_TEMP_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
	}

	// buf[2] = HTS_TEMP_OUT_H
	ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_TEMP_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
	}

	T_OUT = buf[1] | (buf[2] << 8);

	temp_adj = hts221_calc_temp(T_OUT, hts_cal_data);


	// Return in correct units
	if (unit == 'F'){
		//Fahrenheit
		return (temp_adj * 9.0 / 5.0) + 32;

	}
	else {
		//Celsius
		return temp_adj;
	}

}

int hts221_calc_temp(uint16_t T_OUT, HTS_Cal * hts_cal_data){

	int zeroed_temp = T_OUT - hts_cal_data->T0_OUT;
	int temp_adj = (zeroed_temp * hts_cal_data->correction_factor) + hts_cal_data->offset;

	return temp_adj;

}
