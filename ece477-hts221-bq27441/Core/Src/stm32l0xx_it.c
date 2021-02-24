/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "hts221.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim6;
/* USER CODE BEGIN EV */
extern HTS_Cal * hts_cal_data;
extern int bq_init_ret;

// TODO - change from enum to define
typedef enum {
	AVG,  // Average Current (DEFAULT)
	STBY, // Standby Current
	MAX   // Max Current
} current_measure;

// Parameters for the capacity() function, to specify which capacity to read
typedef enum {
	REMAIN,     // Remaining Capacity (DEFAULT)
	FULL,       // Full Capacity
	AVAIL,      // Available Capacity
	AVAIL_FULL, // Full Available Capacity
	REMAIN_F,   // Remaining Capacity Filtered
	REMAIN_UF,  // Remaining Capacity Unfiltered
	FULL_F,     // Full Capacity Filtered
	FULL_UF,    // Full Capacity Unfiltered
	DESIGN      // Design Capacity
} capacity_measure;

// Parameters for the soc() function
typedef enum {
	FILTERED,  // State of Charge Filtered (DEFAULT)
	UNFILTERED // State of Charge Unfiltered
} soc_measure;

// Parameters for the soh() function
typedef enum {
	PERCENT,  // State of Health Percentage (DEFAULT)
	SOH_STAT  // State of Health Status Bits
} soh_measure;

// Parameters for the temperature() function
typedef enum {
	BATTERY,      // Battery Temperature (DEFAULT)
	INTERNAL_TEMP // Internal IC Temperature
} temp_measure;

// Parameters for the setGPOUTFunction() funciton
typedef enum {
	SOC_INT, // Set GPOUT to SOC_INT functionality
	BAT_LOW  // Set GPOUT to BAT_LOW functionality
} gpout_function;


/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM6 global interrupt and DAC1/DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */
  if (hts_cal_data != NULL){
	  int temp = hts221_get_temp('C', hts_cal_data);
	  if (temp == TEMP_ERROR) printf("Error reading temperature\r\n");
	  else printf("Current temperature is %d C\r\n", temp);

	  int humid = hts221_get_humid(hts_cal_data);
	  if (humid == HUMID_ERROR) printf("Error reading humidity\r\n");
	  else printf("Current Relative Humidity is %d%% \r\n", humid);

	  uint16_t battery = BQ27441_voltage();
//	  TODO: Stringify all these :'(
//	  printf("Battery Voltage is ");

	  uint16_t soc = BQ27441_soc(FILTERED);

	  uint16_t current = BQ27441_current(AVG);
	  uint16_t cap_remaining = BQ27441_capacity(REMAIN);
	  uint16_t cap_max = BQ27441_capacity(FULL);
	  int16_t power = BQ27441_power(); //average draw
	  uint16_t soh = BQ27441_soh(PERCENT);
	  uint16_t temp_bat = BQ27441_temperature(BATTERY);
	  uint16_t temp_bq_IC = BQ27441_temperature(INTERNAL_TEMP);

  }

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
