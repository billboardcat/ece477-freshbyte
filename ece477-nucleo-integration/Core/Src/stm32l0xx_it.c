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

extern HTS_Cal * hts_cal_data;
extern int bq_init_ret;
extern enum battery_state batteryState;
;
extern char  prediction_days_str[2];

uint8_t button_history[] = {0, 0, 0, 0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;
extern DMA_HandleTypeDef hdma_usart1_rx;
/* USER CODE BEGIN EV */

extern uint32_t adc_readings[2], adc_dma_buffer[2];

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

    //Prox. Sensor Interrupt

    serial_printf("==EXTI2 - PROX INT==\n");
    VCNL4010_ack_ISR();

    serial_printf("Getting readings... ");
    set_cursor(2,2);
    printString("EXTI2_3_IRQHandler: PROX INT TRIG'D\n");
    uint16_t proximity = VCNL4010_readProximity();
    printString("PROX (RAW): "); printUnsigned(proximity, 10); printString("\n");

    int temp = hts221_get_temp('C', hts_cal_data);
    if (temp == TEMP_ERROR) {
        printString("ERR READING TEMP\n");
    }
    else {
        printString("TEMP: ");
        printUnsigned(temp, 10);
        printString(" deg C\n");
    }

    int humid = hts221_get_humid(hts_cal_data);
    if (humid == HUMID_ERROR) {
        printString("ERR READING RH\n");
    }
    else {
        printString("RH: ");
        printUnsigned(humid, 10);
        printString(" \n");
    }

    uint16_t voltage = BQ27441_voltage();
    uint16_t soc = BQ27441_soc(FILTERED);
    uint16_t current = BQ27441_current(AVG);
    uint16_t cap_remaining = BQ27441_capacity(REMAIN);
    uint16_t cap_max = BQ27441_capacity(DESIGN);
    int16_t power = BQ27441_power(); //average draw
    uint16_t soh = BQ27441_soh(PERCENT);
    uint16_t temp_bat = BQ27441_temperature(BATTERY) / 10;
    uint16_t temp_bq_IC = BQ27441_temperature(INTERNAL_TEMP) / 10;
    serial_printf("Done!\n");

    serial_printf("Printing readings... ");
//    printString("CHARGE: "); printUnsigned(soc, 10); printString(" / 100\n");
//    printString("BATT V: "); printUnsigned(voltage, 10); printString(" mV\n");
//    printString("CURR: "); printUnsigned(current, 10); printString(" mA\n");
//    printString("MAX CAP: "); printUnsigned(cap_max, 10); printString(" mAh\n");
//    printString("REM CAP: "); printUnsigned(cap_remaining, 10); printString(" mAh\n");
//    printString("AVG PWR: "); printFloat(power, 0); printString(" mW\n");
//    printString("HEALTH: "); printUnsigned(soh, 10); printString("\n");
//    printString("BATT TEMP: "); printUnsigned(temp_bat, 10); printString(" K \n");
//    printString("IC TEMP: "); printUnsigned(temp_bq_IC, 10); printString(" K \n");
    serial_printf("Done!\n");

    serial_printf("Updating display... ");
//    display(false);
    serial_printf("Done!\n\n");

  /* USER CODE END EXTI2_3_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_3_IRQn 1 */

  /* USER CODE END EXTI2_3_IRQn 1 */
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

    GPIOA->BSRR |= GPIO_BSRR_BS_8;

//    serial_printf("GPIOA_IDR = 0x%x\n", GPIOA->IDR);
//    for (int i = 0; i < 4; i++) {
//        serial_printf("button_history[%d] = 0x%x\n", i, button_history[i]);
//    }

//    serial_println(" ");

//    TODO - need to renable this for buttons
//    if (GPIOA->IDR & GPIO_IDR_ID10_Msk) {
//        button_history[0] = (button_history[0] << 1) | 1;
//    } else {
//        button_history[0] = (button_history[0] >> 1);
//    }

//    if (GPIOA->IDR & GPIO_IDR_ID11_Msk) {
//        button_history[1] = (button_history[1] << 1) | 1;
//    } else {
//        button_history[1] = (button_history[1] >> 1);
//    }
//
//    if (GPIOA->IDR & GPIO_IDR_ID12_Msk) {
//        button_history[2] = (button_history[2] << 1) | 1;
//    } else {
//        button_history[2] = (button_history[2] >> 1);
//    }
//
//    if (GPIOA->IDR & GPIO_IDR_ID13_Msk) {
//        button_history[3] = (button_history[3] << 1) | 1;
//    } else {
//        button_history[3] = (button_history[3] >> 1);
//    }
//
//    GPIOA->BSRR |= GPIO_BSRR_BR_8;
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

//    uint8_t pressed_mask = 0xFF;
//
//    if (button_history[0] == pressed_mask) {
//        serial_println("apple");
//    }
//
//    if (button_history[1] == pressed_mask) {
//        serial_println("banana");
//    }
//
//    if (button_history[2] == pressed_mask) {
//        serial_println("lemon");
//    }
//
//    if (button_history[3] == pressed_mask) {
//        serial_println("mango");
//    }
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
  int methane = -1;
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

//  uint16_t voltage = BQ27441_voltage();
  uint16_t soc = BQ27441_soc(FILTERED);
//  uint16_t current = BQ27441_current(AVG);
//  uint16_t cap_remaining = BQ27441_capacity(REMAIN);
//  uint16_t cap_max = BQ27441_capacity(DESIGN);
//  int16_t power = BQ27441_power(); //average draw
//  uint16_t soh = BQ27441_soh(PERCENT);
//  uint16_t temp_bat = BQ27441_temperature(BATTERY) / 10;
//  uint16_t temp_bq_IC = BQ27441_temperature(INTERNAL_TEMP) / 10;
//
//  serial_printf("State of Charge\t\t\t\t%d\t%%\r\n", soc);
//  serial_printf("Battery Voltage\t\t\t\t%d\tmV\r\n", voltage);
//  serial_printf("Current\t\t\t\t\t%d\tmA\r\n", current);
//  serial_printf("Max Capacity\t\t\t\t%d\tmAh\r\n", cap_max);
//  serial_printf("Remaining Capacity\t\t\t%d\tmAh\r\n", cap_remaining);
//  serial_printf("Ave power consumption\t\t\t%d\tmW\r\n", power);
//  serial_printf("Health\t\t\t\t\t%d\t%%\r\n", soh);
//  serial_printf("Battery Pack Temp\t\t\t%d\tK\r\n", temp_bat);
//  serial_printf("Current Bat IC Temp is\t\t\t%d\tK\r\n\n", temp_bq_IC);

  serial_printf("Methane: %d\n", adc_dma_buffer[0]);

  // Send sensor data to cloud
  serial_select(WIFI);
  if (setup_wifi("ASUS", "rickroll362") == AT_FAIL) {
    // TODO: error handling
  }
  if (sent_freshbyte_data(temp, humid, adc_dma_buffer[0]) == AT_FAIL){
    // TODO: error handling
  }

  // Disable the 5V regulator
  batteryState = LOW;
  HAL_GPIO_WritePin(EN_5V_GPIO_Port, EN_5V_Pin, GPIO_PIN_RESET);
  HAL_Delay(5000);
  serial_select(DEBUG_PRINT);
  serial_println("=== Interrupt done! === \n");

  //get prediction
//  int prediction = get_prediction();

  //TODO: get prediction int
//  display_readings(soc, temp, humid, methane, 115);

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void display_readings(int battery, int temp, int humid, int methane_raw, int prediction_days) {

  //use string or use

  int methane_ppm = methane_raw;
  // TODO: get ppm methane value for display
  //  int methane_ppm = ADC_calc_ppm(methane_raw);

  serial_select(DEBUG_PRINT);
  serial_printf("Clearing display buffers...\n");
  clear_buffer();

  serial_printf("Powering up display...\n");
  epd_powerUp();

  serial_printf("Writing to B/W buffer...\n");
//  write_RAM_to_epd(buffer1, buffer1_size, 0, false);

  serial_printf("Writing to R buffer...\n");
//  write_RAM_to_epd(buffer1, buffer1_size, 1, false);

  serial_println("Printing random information to display\n");
  set_text_scale(2);
  set_x_margin(1);
  set_y_margin(1);
  set_cursor(1,1);

  printString("4/10/2021, 12:00 PM\n");
  printString("Battery: "); printFloat(battery, 0); printString("%\n");
  printString("Temperature: "); printFloat(temp, 0); printString(" F\n");
  printString("Rel. Humidity: "); printFloat(humid, 0); printString("%\n");
  printString("Methane: "); printFloat(methane_ppm, 0); printString(" ppm\n\n");
  // TODO: save food selection
  printString("Food: Banana\n");
  // TODO: get time elapsed from RTC
  printString("Time Elapsed: 2 days\n");
  printString("Est. Days Left: "); printFloat(prediction_days, 0); printString(" days\n");

  display(true);
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
