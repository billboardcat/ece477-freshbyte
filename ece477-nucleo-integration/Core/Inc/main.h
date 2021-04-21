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
enum system_state {
    WAITING,
    MONITORING_SETUP,
    MONITORING
};

enum battery_state {
    HIGH,
    LOW
};

enum fruit_type {
    NONE,
    APPLE,
    BANANA,
    LIME,
    MANGO
};

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
#define PROX_INT_Pin GPIO_PIN_2
#define PROX_INT_GPIO_Port GPIOB
#define PROX_INT_EXTI_IRQn EXTI2_3_IRQn
#define EPD_DC_Pin GPIO_PIN_7
#define EPD_DC_GPIO_Port GPIOC
#define EN_5V_Pin GPIO_PIN_8
#define EN_5V_GPIO_Port GPIOC
#define SRAM_CS_Pin GPIO_PIN_9
#define SRAM_CS_GPIO_Port GPIOC
#define GREEN_LED_Pin GPIO_PIN_4
#define GREEN_LED_GPIO_Port GPIOB
#define RED_LED_Pin GPIO_PIN_5
#define RED_LED_GPIO_Port GPIOB
#define EPD_CS_Pin GPIO_PIN_6
#define EPD_CS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
