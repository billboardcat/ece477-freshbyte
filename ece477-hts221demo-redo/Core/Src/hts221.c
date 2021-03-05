/*
 * hts221.c
 *
 *  Created on: Feb 14, 2021
 *      Author: pcrain
 */

#include "hts221.h"
#include <stdlib.h>

// For debug
#include <stdio.h>
#include "stm32l0xx_hal.h"
extern UART_HandleTypeDef huart1;

extern I2C_HandleTypeDef hi2c1; // TODO: How do we handle this if I2C controller is changed?

HTS_Cal * hts221_init () {
	//Returns NULL if not able to init object - remember to check after calling this function

	HAL_StatusTypeDef ret;	// I2C return status
	uint8_t buf[13];			// read buffer

	/* === Set HTS221 to wake mode === */
	buf[0] = HTS_CTRL_REG1_PD | HTS_CTRL_REG1_BUD;

//	ret = HAL_I2C_Master_Transmit(&hi2c1, (HTS_ADDR << 1), buf, 2, HAL_MAX_DELAY);
	ret = HAL_I2C_Mem_Write(&hi2c1, (HTS_ADDR << 1), HTS_CTRL_REG1, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
	}
	else  {
		/* === Read in temperature + humidity calibration data === */
		//Temp

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

		//Humidity

		// buf[7] = HTS_CAL_H0_T0_OUT_L
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_H0_T0_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 7, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[8] = HTS_CAL_H0_T0_OUT_H
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_H0_T0_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 8, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[9] = HTS_CAL_H1_T0_OUT_L
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_H1_T0_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 9, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[10] = HTS_CAL_H1_T0_OUT_H
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_H1_T0_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 10, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[11] = HTS_CAL_H0_rH_x2
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_H0_rH_x2, I2C_MEMADD_SIZE_8BIT, buf + 11, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		// buf[12] = HTS_CAL_H1_rH_x2
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_CAL_H1_rH_x2, I2C_MEMADD_SIZE_8BIT, buf + 12, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}

		/* === Process temperature + humidity calibration data === */

		uint16_t T0_degC_R32 = buf[0] >> 3; //divide x8 value by 8
		uint16_t T1_degC_R33 = buf[1] >> 3; //divide x8 value by 8
		uint8_t T1_T0_msb 	= buf[2];
		int16_t T0_OUT = (buf[3] | (buf[4] << 8)); // This should be signed int
		int16_t T1_OUT = (buf[5] | (buf[6] << 8)); // This should be signed int

		uint8_t H0_Rh_R30 = buf[11] >> 1; //divide HTS_CAL_H0_rH_x2 by 2
		uint8_t H1_Rh_R31 = buf[12] >> 1; //divide HTS_CAL_H0_rH_x2 by 2
		int16_t H0_T0_OUT = (buf[7] | (buf[8] << 8)); // This should be signed int
		int16_t H1_T0_OUT = (buf[9] | (buf[10] << 8)); // This should be signed int

		// add msb's for 10 bit values
		T0_degC_R32 |= (T1_T0_msb & 0b0011) << 8;
		T1_degC_R33 |= (T1_T0_msb & 0b1100) << 6;

		// init struct to store calibration data
		HTS_Cal * hts_cal_data = malloc(sizeof(HTS_Cal));

		// Store Temp
		hts_cal_data->T0_OUT = T0_OUT;
		hts_cal_data->temp_correction_factor = (float) (T1_degC_R33 - T0_degC_R32) / (T1_OUT - T0_OUT);
		hts_cal_data->temp_offset = T0_degC_R32;

		//zeroed_temp = T_out - T0_Out
		//temp_adj = (zeroed_temp * correction_factor) + offset

		//Store Humid.
		hts_cal_data->H0_OUT = H0_T0_OUT;
		hts_cal_data->humid_correction_factor = (float) (H1_Rh_R31 - H0_Rh_R30) / (H1_T0_OUT - H0_T0_OUT);
		hts_cal_data->humid_offset = H0_Rh_R30;

		return hts_cal_data;
	}

	return NULL;
}

int hts221_get_temp(char unit, HTS_Cal * hts_cal_data){
	//Read values

	HAL_StatusTypeDef ret;	// I2C return status
	uint8_t buf[3];			// read buffer
	int16_t T_OUT;			// T_OUT raw temperature reading
	int temp_adj;			// calibrated temperature value

	/* === Start a temperature reading === */
	buf[0] = HTS_CTRL_REG2_ONE_SHOT;
	ret = HAL_I2C_Mem_Write(&hi2c1, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
	}


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
		return TEMP_ERROR;
	}

	// buf[1] = HTS_TEMP_OUT_L
	ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_TEMP_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
		return TEMP_ERROR;
	}

	// buf[2] = HTS_TEMP_OUT_H
	ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_TEMP_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
		return TEMP_ERROR;
	}

	T_OUT = buf[1] | (((uint16_t) buf[2]) << 8);

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

int hts221_calc_temp(int16_t T_OUT, HTS_Cal * hts_cal_data){

	int zeroed_temp = T_OUT - hts_cal_data->T0_OUT;
	int temp_adj = (zeroed_temp * hts_cal_data->temp_correction_factor) + hts_cal_data->temp_offset;

	return temp_adj;

}

int hts221_get_humid(HTS_Cal * hts_cal_data){
	//Read values

	HAL_StatusTypeDef ret;	// I2C return status
	uint8_t buf[3];			// read buffer
	int16_t H_OUT;			// H_OUT raw temperature reading
	int humid_adj;			// calibrated temperature value

	/* === Start a humidity reading === */
	buf[0] = HTS_CTRL_REG2_ONE_SHOT;
	ret = HAL_I2C_Mem_Write(&hi2c1, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
		return HUMID_ERROR;
	}


	/* === Read in humidity data === */

	//TODO check that this loop is right...
	// Try three times for temp data to be ready
	for (int i = 0; i < 3; ++i) {
		// buf[0] = HTS_STATUS_REG
		ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_STATUS_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			// TODO: error handling
		}
		if (buf[0] & 2){
			// new humid. data ready
			break;
		}
		//TODO - HAL_WAIT?
		return HUMID_ERROR;
	}

	// buf[1] = HTS_HUMIDITY_OUT_L
	ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_HUMIDITY_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
		return HUMID_ERROR;
	}

	// buf[2] = HTS_HUMIDITY_OUT_H
	ret = HAL_I2C_Mem_Read(&hi2c1, (HTS_ADDR << 1), HTS_HUMIDITY_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
	if (ret != HAL_OK) {
		// TODO: error handling
		return HUMID_ERROR;
	}

	H_OUT = buf[1] | (((uint16_t) buf[2]) << 8);

	humid_adj = hts221_calc_humid(H_OUT, hts_cal_data);

	return humid_adj;

}

int hts221_calc_humid(int16_t H_OUT, HTS_Cal * hts_cal_data){

	int zeroed_humid = H_OUT - hts_cal_data->H0_OUT;
	int humid_adj = (zeroed_humid * hts_cal_data->humid_correction_factor) + hts_cal_data->humid_offset;

	return humid_adj;

}
