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

#include "serial_print.h"
#include "bq27441.h"
#include "hts221.h"
#include "vcnl4010.h"
#include "epd_gfx.h"
#include "epd.h"
#include "adc.h"
#include "at_commands.h"

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
const float intercepts[11] = {0.16073036, -0.04111263, -0.23397941, -0.17531694, -0.11094095, -0.06462243, 0.00729972,  0.10545316,  0.18820124,  0.16793229, -0.0036444};
const float coefficients[11][3] = {
        {0.04302877, -0.11790926, -0.07499422},
        {0.02560501, -0.80263872, 0.843488},
        {0.02066652, -0.40335158, 0.42342182},
        {0.02795597, -0.29232132, 0.2462591},
        {0.03853047, 0.0698773,  -0.26072735},
        {0.03113318, 0.08293416, -0.23121061},
        {0.02430052, 0.22897817, -0.37987175},
        {0.01500706 , 0.2655911,  -0.38267549},
        {0.01167814 , 0.25759487, -0.3564595},
        {-0.00155557, 0.28978902, -0.33514264},
        {-0.23635007 , 0.42145625,  0.50791264}
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc;
extern ADC_HandleTypeDef hadc;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;
extern DMA_HandleTypeDef hdma_usart1_rx;
/* USER CODE BEGIN EV */

extern uint32_t buffer1_size;
extern uint8_t *buffer1;
extern HTS_Cal * hts_cal_data;
extern int bq_init_ret;
extern enum battery_state batteryState;
extern char  prediction_days_str[2];
extern uint32_t adc_dma_buffer[9];
extern enum fruit_type fruit_selection;

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
    serial_println("NMI_Handler: something's not being handled right!");
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
    serial_println("HardFault_Handler: hard fault occurred!!!");
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
  * @brief This function handles EXTI line 2 and line 3 interrupts.
  */
void EXTI2_3_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_3_IRQn 0 */

  // Prox. Sensor Interrupt
  serial_printf("==EXTI2 - PROX INT==\n");

  // Acknowledge the interrupt
  VCNL4010_ack_ISR();

  // Turn on the 5V power to the methane and Wi-Fi peripherals
  batteryState = HIGH;
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_SET);
  HAL_Delay(5000);

  /* USER CODE END EXTI2_3_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_3_IRQn 1 */

  // Gather sensor data
  uint16_t proximity = VCNL4010_readProximity();
  serial_printf("Proximity Reading is \t\t\t%d (0x%x)\r\n", proximity, proximity);

  int temp = -200;
  int humid = -1;
  if (hts_cal_data != NULL) {
    temp = hts221_get_temp('F', hts_cal_data);
    if (temp == TEMP_ERROR) serial_printf("Error reading temperature\r\n");
    else serial_printf("Current temperature is \t\t\t%d\tC\r\n", temp);

    humid = hts221_get_humid(hts_cal_data);
    if (humid == HUMID_ERROR) serial_printf("Error reading humidity\r\n");
    else serial_printf("Current Relative Humidity is \t\t%d\t%c\r\n", humid, 37);
  } else {
    serial_printf("Temp/RH sensor initialization has failed.\n Please power cycle system to attemp reinitialization.\n");
  }

  uint16_t soc = BQ27441_soc(FILTERED);

  serial_printf("Methane: %d\n", adc_dma_buffer[0]);

  /* USER CODE END EXTI2_3_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel 1 interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel 2 and channel 3 interrupts.
  */
void DMA1_Channel2_3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_3_IRQn 0 */

  /* USER CODE END DMA1_Channel2_3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA1_Channel2_3_IRQn 1 */

  /* USER CODE END DMA1_Channel2_3_IRQn 1 */
}

/**
  * @brief This function handles ADC, COMP1 and COMP2 interrupts (COMP interrupts through EXTI lines 21 and 22).
  */
void ADC1_COMP_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_COMP_IRQn 0 */

  /* USER CODE END ADC1_COMP_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc);
  /* USER CODE BEGIN ADC1_COMP_IRQn 1 */

  /* USER CODE END ADC1_COMP_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt and DAC1/DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
  // Timer 6 should gather new sensor readings, upload these data to the cloud via Wi-Fi, pull the updated prediction, and update the display.

  serial_println("=== TIM6 Interrupt ===");

  // Turn on the 5V power to the methane and Wi-Fi peripherals
  batteryState = HIGH;
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_SET);
  HAL_Delay(5000);

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  // Gather sensor data
  uint16_t proximity = VCNL4010_readProximity();
  serial_printf("Proximity Reading is \t\t\t%d (0x%x)\r\n", proximity, proximity);

  int temp = -200;
  int humid = -1;
  if (hts_cal_data != NULL) {
    temp = hts221_get_temp('F', hts_cal_data);
    if (temp == TEMP_ERROR) serial_printf("Error reading temperature\r\n");
    else serial_printf("Current temperature is \t\t\t%d\tC\r\n", temp);

    humid = hts221_get_humid(hts_cal_data);
    if (humid == HUMID_ERROR) serial_printf("Error reading humidity\r\n");
    else serial_printf("Current Relative Humidity is \t\t%d\t%c\r\n", humid, 37);
  } else {
    serial_printf("!!! Temp/RH sensor initialization has failed.\n!!! Please power cycle system to attempt reinitialization.\n");
  }

  uint16_t soc = BQ27441_soc(FILTERED);

  serial_printf("Methane: %d\n", adc_dma_buffer[0]);

  // Send sensor data to cloud
  //TODO uncomment this after SMAT
  serial_select(WIFI);
  if (setup_wifi("ASUS", "rickroll362") == AT_FAIL) {
    // TODO: error handling
  }
  if (sent_freshbyte_data(temp, humid, adc_dma_buffer[0]) == AT_FAIL){
    // TODO: error handling
  }

  //get wifi prediction here?

  // Disable the 5V regulator
  batteryState = LOW;
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_RESET);
  HAL_Delay(5000);
  serial_select(DEBUG_PRINT);
  serial_println("=== Interrupt done! === \n");

  //get prediction from prediction function
  //  int prediction = get_prediction();

  //TODO: get prediction int

  display_readings(soc, temp, humid, adc_dma_buffer[0], 5);

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void display_readings(int battery, int temp, int humid, int methane_raw, int prediction_days) {

  //use string or use

  //turn on red led while updating display
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);

  int methane_ppm = methane_raw;
  // TODO: get ppm methane value for display
  // needs to init first
  //  int methane_ppm = ADC_calc_ppm(methane_raw);

  serial_select(DEBUG_PRINT);
  serial_printf("Clearing display buffers...\n");
  clear_buffer();

  serial_printf("Powering up display...\n");
  epd_powerUp();

  serial_printf("Writing to B/W buffer...\n");
  write_RAM_to_epd(buffer1, buffer1_size, 0, false);

  serial_printf("Writing to R buffer...\n");
  write_RAM_to_epd(buffer1, buffer1_size, 1, false);

  serial_println("Printing random information to display\n");
  set_text_scale(2);
  set_x_margin(1);
  set_y_margin(1);
  set_cursor(1,1);

  printString("4/10/2021, 12:00 PM\n");
  printString("Battery: "); printFloat(battery, 0); printString("%\n");
  printString("Temperature: "); printFloat(temp, 0); printString(" F\n");
  printString("Rel. Humidity: "); printFloat(humid, 0); printString("%\n");
  printString("Methane: "); printFloat(methane_ppm, 0); printString(" \n\n");
  //  TODO: enable this when ppm
  //  printString(" ppm\n\n");

  // Print the current food being stored
  switch (fruit_selection) {
    case NONE: 		printString("Food: None\n"); 	break;
    case APPLE: 	printString("Food: Apple\n"); 	break;
    case BANANA: 	printString("Food: Banana\n");	break;
    case LIME: 		printString("Food: Lemon\n"); 	break;
    case MANGO: 	printString("Food: Mango\n"); 	break;
  }
  // TODO: get time elapsed from RTC
  printString("Time Elapsed: 0 days\n");
  printString("Est. Days Left: "); printFloat(prediction_days, 0); printString(" days\n");

  display(true);

  //turn off red led
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
}

int predictive_model(int new_temp, int new_rh, int new_methane) {
  // for testing purposes only, take these three lines out for real thing
  //int new_temp = 70;
  //int new_rh = 50;
  //int new_methane = 550;

  int row[3] = {new_methane, new_temp, new_rh};

  float max_val = 0;
  int days = 0;
  float dot_product = 0;
  float x = 0;

  for(int i=0; i < 11; i++) {
    dot_product = intercepts[i] + (coefficients[i][0] * row[0] + coefficients[i][1] * row[1] + coefficients[i][2] * row[2]);
    x = 1 / (1 + pow(2.71828, -dot_product));
    if (x > max_val){
      max_val = x;
      days = i;
    }
  }
  //printf("\n%d Days Remaining\n", days);
  return days;
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
