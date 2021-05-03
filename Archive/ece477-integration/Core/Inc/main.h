/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_PRESSURE_Pin GPIO_PIN_0
#define ADC_PRESSURE_GPIO_Port GPIOA
#define ADC_GAS_Pin GPIO_PIN_1
#define ADC_GAS_GPIO_Port GPIOA
#define ADC_TOUCH_X_Pin GPIO_PIN_2
#define ADC_TOUCH_X_GPIO_Port GPIOA
#define ADC_TOUCH_Y_Pin GPIO_PIN_3
#define ADC_TOUCH_Y_GPIO_Port GPIOA
#define DISP_NSS_Pin GPIO_PIN_4
#define DISP_NSS_GPIO_Port GPIOA
#define DISP_CLK_Pin GPIO_PIN_5
#define DISP_CLK_GPIO_Port GPIOA
#define DISP_MISO_Pin GPIO_PIN_6
#define DISP_MISO_GPIO_Port GPIOA
#define DISP_MOSI_Pin GPIO_PIN_7
#define DISP_MOSI_GPIO_Port GPIOA
#define DISP_DC_Pin GPIO_PIN_0
#define DISP_DC_GPIO_Port GPIOB
#define DISP_NRST_Pin GPIO_PIN_1
#define DISP_NRST_GPIO_Port GPIOB
#define WIFI_NSS_Pin GPIO_PIN_12
#define WIFI_NSS_GPIO_Port GPIOB
#define WIFI_CLK_Pin GPIO_PIN_13
#define WIFI_CLK_GPIO_Port GPIOB
#define WIFI_MISO_Pin GPIO_PIN_14
#define WIFI_MISO_GPIO_Port GPIOB
#define WIFI_MOSI_Pin GPIO_PIN_15
#define WIFI_MOSI_GPIO_Port GPIOB
#define BTN_1_IN_Pin GPIO_PIN_10
#define BTN_1_IN_GPIO_Port GPIOA
#define BTN_2_IN_Pin GPIO_PIN_11
#define BTN_2_IN_GPIO_Port GPIOA
#define BTN_3_IN_Pin GPIO_PIN_12
#define BTN_3_IN_GPIO_Port GPIOA
#define BTN_4_IN_Pin GPIO_PIN_13
#define BTN_4_IN_GPIO_Port GPIOA
#define WIFI_TX_Pin GPIO_PIN_14
#define WIFI_TX_GPIO_Port GPIOA
#define WIFI_RX_Pin GPIO_PIN_15
#define WIFI_RX_GPIO_Port GPIOA
#define TX_Pin GPIO_PIN_6
#define TX_GPIO_Port GPIOB
#define RX_Pin GPIO_PIN_7
#define RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
