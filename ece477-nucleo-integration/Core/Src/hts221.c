/*
 * hts221.c
 *
 *  Created on: Feb 14, 2021
 *      Author: pcrain
 */

#include "hts221.h"
#include "i2c.h"
#include "stm32l0xx_hal.h"
#include <stdlib.h>
#include "serial_print.h"

#define I2C_CONTROLLER hi2c1

// Internal Functions Declarations
int hts221_calc_temp(int16_t T_OUT, HTS_Cal * hts_cal_data);
int hts221_calc_humid(int16_t H_OUT, HTS_Cal * hts_cal_data);
int hts221_reboot(void);

// Function code
int hts221_reboot() {
    HAL_StatusTypeDef ret;	// I2C return status
    uint8_t buf[1];			// read buffer

    /* === Reboot HTS221 === */

    //read register
    ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return HTS_REBOOT_FAIL;
    }

    //write boot bit
    buf[0] |= HTS_CTRL_REG2_BOOT;
    ret = HAL_I2C_Mem_Write(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return HTS_REBOOT_FAIL;
    }

    //wait for device to restart + clear boot bit
    do{
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return HTS_REBOOT_FAIL;
        }
    } while (buf[0] & HTS_CTRL_REG2_BOOT);

	serial_println("Rebooted\n");

    return HTS_REBOOT_SUCCESS;

}

HTS_Cal * hts221_init () {
    HAL_StatusTypeDef ret;	// I2C return status
    uint8_t buf[7];			// read buffer

    if (hts221_reboot() == -1) {
        serial_println("Reboot FAIL\n");
        return NULL;
    }

    /* === Set HTS221 to wake mode === */
    buf[0] = HTS_CTRL_REG1_PD | HTS_CTRL_REG1_BUD;

    ret = HAL_I2C_Mem_Write(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG1, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return NULL;
    }
    else  {
        /* === Read in temperature calibration data === */
        // buf[0] = T0_degC_x8
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_T0_degC_x8, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[1] = T1_degC_x8
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_T1_degC_x8, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[2] = T1_T0_msb
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_T1_T0_msb, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[3] = HTS_CAL_T0_OUT_L
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_T0_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 3, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[4] = HTS_CAL_T0_OUT_H
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_T0_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 4, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[5] = HTS_CAL_T1_OUT_L
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_T1_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 5, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[6] = HTS_CAL_T1_OUT_H
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_T1_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 6, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        /* === Process temperature calibration data === */

        uint16_t T0_degC_R32 = buf[0];
        uint16_t T1_degC_R33 = buf[1];
        uint16_t T1_T0_msb 	= buf[2];
        int16_t T0_OUT = (buf[3] | (buf[4] << 8)); // This should be signed int
        int16_t T1_OUT = (buf[5] | (buf[6] << 8)); // This should be signed int

        // add msb's for 10 bit values
        T0_degC_R32 |= (T1_T0_msb & 0b0011) << 8;
        T1_degC_R33 |= (T1_T0_msb & 0b1100) << 6;

        // divide by 8
        T0_degC_R32 >>= 3;
        T1_degC_R33 >>= 3;

        // init struct to store calibration data
        HTS_Cal * hts_cal_data = malloc(sizeof(HTS_Cal));

        hts_cal_data->T0_OUT = T0_OUT;
        hts_cal_data->correction_factor = (float) (T1_degC_R33 - T0_degC_R32) / (T1_OUT - T0_OUT);
        hts_cal_data->offset = T0_degC_R32;

        /*=== Read in humidity calibration data ===*/
        // buf[0] = HTS_CAL_H0_T0_OUT_L
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_H0_T0_OUT_L, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[1] = HTS_CAL_H0_T0_OUT_H
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_H0_T0_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[2] = HTS_CAL_H1_T0_OUT_L
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_H1_T0_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[3] = HTS_CAL_H1_T0_OUT_H
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_H1_T0_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 3, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[4] = HTS_CAL_H0_rH_x2
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_H0_rH_x2, I2C_MEMADD_SIZE_8BIT, buf + 4, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        // buf[5] = HTS_CAL_H1_rH_x2
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CAL_H1_rH_x2, I2C_MEMADD_SIZE_8BIT, buf + 5, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return NULL;
        }

        /*=== Process humidity calibration data ===*/
        uint8_t H0_Rh_R30 = buf[4] >> 1; //divide HTS_CAL_H0_rH_x2 by 2
        uint8_t H1_Rh_R31 = buf[5] >> 1; //divide HTS_CAL_H0_rH_x2 by 2
        int16_t H0_T0_OUT = (buf[0] | (buf[1] << 8)); // This should be signed int
        int16_t H1_T0_OUT = (buf[2] | (buf[3] << 8)); // This should be signed int

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
    uint8_t buf[7];			// read buffer
    int16_t T_OUT;			// T_OUT raw temperature reading
    int temp_adj;			// calibrated temperature value

    /* === Start a temperature reading === */
    buf[0] = HTS_CTRL_REG2_ONE_SHOT;
    ret = HAL_I2C_Mem_Write(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        // TODO: error handling
    }


    /* === Read in temperature data === */

    //TODO check that this loop is right...
    // Try three times for temp data to be ready
    for (int i = 0; i < 3; ++i) {
        // buf[0] = HTS_STATUS_REG
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_STATUS_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
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
    ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_TEMP_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        // TODO: error handling
    }

    // buf[2] = HTS_TEMP_OUT_H
    ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_TEMP_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        // TODO: error handling
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

int hts221_calc_temp(int16_t T_OUT, HTS_Cal * hts_cal_data) {

    int zeroed_temp = T_OUT - hts_cal_data->T0_OUT;
    int temp_adj = (zeroed_temp * hts_cal_data->correction_factor) + hts_cal_data->offset;

    return temp_adj;
}

int hts221_get_humid(HTS_Cal * hts_cal_data) {
    //Read values

    HAL_StatusTypeDef ret;	// I2C return status
    uint8_t buf[3];			// read buffer
    int16_t H_OUT;			// H_OUT raw temperature reading
    int humid_adj;			// calibrated temperature value

    /* === Start a humidity reading === */
    ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return HUMID_ERROR;
    }

    buf[0] |= HTS_CTRL_REG2_ONE_SHOT;
    ret = HAL_I2C_Mem_Write(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return HUMID_ERROR;
    }

    // wait for one shot bit to clear by the hts
    do{
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_CTRL_REG2, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return HUMID_ERROR;
        }
    } while (buf[0] & HTS_CTRL_REG2_ONE_SHOT);


    /* === Read in humidity data === */

    //TODO check that this loop is right...
    // Try three times for temp data to be ready
    for (int i = 0; i < 3; ++i) {
        // buf[0] = HTS_STATUS_REG
        ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_STATUS_REG, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            return HUMID_ERROR;
        }
        if (buf[0] & 2){
            // new humid. data ready
            break;
        }
        return HUMID_ERROR;
    }

    // buf[1] = HTS_HUMIDITY_OUT_L
    ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_HUMIDITY_OUT_L, I2C_MEMADD_SIZE_8BIT, buf + 1, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return HUMID_ERROR;
    }

    // buf[2] = HTS_HUMIDITY_OUT_H
    ret = HAL_I2C_Mem_Read(&I2C_CONTROLLER, (HTS_ADDR << 1), HTS_HUMIDITY_OUT_H, I2C_MEMADD_SIZE_8BIT, buf + 2, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
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