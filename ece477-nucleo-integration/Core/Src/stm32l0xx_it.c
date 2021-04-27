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

RTC_TimeTypeDef sTime1;
RTC_DateTypeDef sDate1;

uint8_t current_day;
uint8_t days_elapsed = 0;

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
extern TIM_HandleTypeDef htim6;
extern DMA_HandleTypeDef hdma_usart1_rx;
/* USER CODE BEGIN EV */

extern RTC_HandleTypeDef hrtc;
extern uint32_t buffer1_size;
extern uint8_t *buffer1;
extern bool bq_init_ret;
extern enum battery_state batteryState;
extern char  prediction_days_str[2];
extern uint32_t adc_dma_buffer[9];
extern enum fruit_type fruit_selection;
extern bool food_present;

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
  serial_printf("== EXTI2 - PROX INT ==\n");

  if (!food_present) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    return;
  }

  // update days elapsed ...
  HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BCD);
  if (current_day != sDate1.Date){
    serial_print("Updating date... ");
    current_day = sDate1.Date;
    days_elapsed++;
    serial_print("Done! \n\n");
  }

  // Turn on the 5V power to the methane and Wi-Fi peripherals
  batteryState = HIGH;
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_SET);

  int temp = -200;
  int humid = -1;
  temp = hts221_get_temp('F');
  if (temp == TEMP_ERROR) serial_printf("Error reading temperature\r\n");
  else serial_printf("Current temperature is \t\t\t%d\tC\r\n", temp);

  humid = hts221_get_humid();
  if (humid == HUMID_ERROR) serial_printf("Error reading humidity\r\n");
  else serial_printf("Current Relative Humidity is \t\t%d\t%c\r\n", humid, 37);

  uint16_t soc = BQ27441_soc(FILTERED);

  // Send sensor data to cloud
  serial_select(WIFI);
  if (setup_wifi("ASUS", "rickroll362") == AT_FAIL) {
    // TODO: error handling
  }
  if (sent_freshbyte_data(temp, humid, adc_dma_buffer[0]) == AT_FAIL){
    // TODO: error handling
  }

  //get wifi prediction here?

  // Disable the 5V regulator
  serial_select(DEBUG_PRINT);

  uint32_t methane_raw = adc_dma_buffer[0];
  float methane_ppm = ADC_calc_ppm(methane_raw);
  serial_printf("Methane: %d\n", (int) methane_ppm);
  batteryState = LOW;

  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_RESET);
//  HAL_Delay(5000);

  display_readings(soc, temp, humid, methane_ppm, predictive_model(temp, humid, methane_raw));
// Acknowledge the interrupt
  VCNL4010_ack_ISR();
  serial_println("=== Interrupt done! === \n");

  /* USER CODE END EXTI2_3_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_3_IRQn 1 */

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
  * @brief This function handles TIM6 global interrupt and DAC1/DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
  // Timer 6 should gather new sensor readings, upload these data to the cloud via Wi-Fi, pull the updated prediction, and update the display.

  serial_println("=== TIM6 Interrupt ===");

  if (!food_present) {
    HAL_TIM_IRQHandler(&htim6);
    return;
  }

  // update days elapsed ...
  HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BCD);
  if (current_day != sDate1.Date){
    serial_print("Updating date... ");
    current_day = sDate1.Date;
    days_elapsed++;
    serial_print("Done! \n\n");
  }

  // Turn on the 5V power to the methane and Wi-Fi peripherals
  batteryState = HIGH;
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_SET);

  int temp = -200;
  int humid = -1;
  temp = hts221_get_temp('F');
  if (temp == TEMP_ERROR) serial_printf("Error reading temperature\r\n");
  else serial_printf("Current temperature is \t\t\t%d\tC\r\n", temp);

  humid = hts221_get_humid();
  if (humid == HUMID_ERROR) serial_printf("Error reading humidity\r\n");
  else serial_printf("Current Relative Humidity is \t\t%d\t%c\r\n", humid, 37);

  uint16_t soc = BQ27441_soc(FILTERED);
  serial_printf("Current Battery is %d\n", soc);

  // Send sensor data to cloud
  serial_select(WIFI);
  if (setup_wifi("ASUS", "rickroll362") == AT_FAIL) {
    // TODO: error handling
  }
  if (sent_freshbyte_data(temp, humid, adc_dma_buffer[0]) == AT_FAIL){
    // TODO: error handling
  }

  // get wifi prediction here?

  // Disable the 5V regulator
  serial_select(DEBUG_PRINT);

  uint32_t methane_raw = adc_dma_buffer[0];
  float methane_ppm = ADC_calc_ppm(methane_raw);
  serial_printf("Methane: %d\n", (int) methane_ppm);
  batteryState = LOW;

  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_RESET);
//  HAL_Delay(5000);

  display_readings(soc, temp, humid, methane_ppm, predictive_model(temp, humid, methane_raw));
  serial_println("=== Interrupt done! === \n");

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void display_readings(int battery, int temp, int humid, int methane_ppm, int prediction_days) {

  //use string or use

  //turn on red led while updating display
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);

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
  set_x_margin(0);
  set_y_margin(0);
  set_cursor(0,0);

  HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BCD);

  //get datetime
  printFloat( (((sDate1.Month & 0xF0) >> 4) * 10) + (sDate1.Month & 0xF), 0); printString("/");
  printFloat( (((sDate1.Date & 0xF0) >> 4) * 10) + (sDate1.Date & 0xF), 0); printString("/");
//  printString("20");
  printFloat( (((sDate1.Year & 0xF0) >> 4) * 10) + (sDate1.Year & 0xF), 0); printString(", ");
  printFloat( (((sTime1.Hours & 0xF0) >> 4) * 10) + (sTime1.Hours & 0xF), 0); printString(":");
  printFloat( (((sTime1.Minutes & 0xF0) >> 4) * 10) + (sTime1.Minutes & 0xF), 0); printString("\n");

  printString("Battery: ");       printFloat(battery, 0);     printString("%\n");
  printString("Temperature: ");   printFloat(temp, 0);        printString(" F\n");
  printString("Rel. Humidity: "); printFloat(humid, 0);       printString("%\n");
  printString("Methane: ");       printFloat(methane_ppm, 0); printString(" ppm\n\n");

  // Print the current food being stored
  switch (fruit_selection) {
    case NONE:    printString("Food: None\n"); 	  break;
    case APPLE:   printString("Food: Apple\n"); 	break;
    case BANANA:  printString("Food: Banana\n");	break;
    case LIME:    printString("Food: Lemon\n"); 	break;
    case MANGO:   printString("Food: Mango\n"); 	break;
  }

//  printString("Time Elapsed: 0 days\n");
  if(days_elapsed == 1){
    printString("Time Elapsed: ");  printFloat(days_elapsed, 0);    printString(" day\n");
  }
  else{
    printString("Time Elapsed: ");  printFloat(days_elapsed, 0);    printString(" days\n");
  }

  printString("Est. Days Left: ");  printFloat(prediction_days, 0); printString(" days\n");

  display(true);

  //turn off red led
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
}

int predictive_model(int new_temp, int new_rh, int new_methane) {
  int days = 0;

  if(fruit_selection == BANANA) {
    int row[3] = {new_methane, new_temp, new_rh};

    float max_val = 0;
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
  }
  else if (fruit_selection == APPLE){
    days = 14 - days_elapsed;
  }
  else if(fruit_selection == LIME) {
    days = 14 - days_elapsed;
  }
  else if(fruit_selection == MANGO) {
    days = 7 - days_elapsed;
  }
  return days;
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
