/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hts221.h"
#include "bq27441.h"
#include "vcnl4010.h"
#include "serial_print.h"
#include "epd.h"
#include "epd_gfx.h"
#include "at_commands.h"
#include "main_gui.c"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
unsigned char UART1_rxBuffer[600] = {0};

HTS_Cal * hts_cal_data;
int bq_init_ret;

//TODO - make this an enum? for battery state?
int state = 0;
uint32_t adc_readings[] = {0, 0};

extern DMA_HandleTypeDef hdma_adc;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void display_setup(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void display_setup() {
  serial_printf("Initializing display... ");
  epd_init(false);
  serial_println("Done!");

  serial_printf("Setting display rotation... ");
  set_rotation(2);
  serial_println("Done!");

  serial_printf("Clearing display buffers... ");
  clear_buffer();
  epd_powerUp();
  write_RAM_to_epd(buffer1, buffer1_size, 1, false);
  display(false);
  serial_println("Done!");

  serial_printf("Drawing bitmap to buffer... ");
  draw_bitmap(0, 0, main_select, EPD_WIDTH, EPD_HEIGHT, EPD_BLACK);
  display(false);
  serial_println("Done!\n");

}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc) {
    serial_println("*** ADC WATCHDOG INTERRUPT ***");
    HAL_Delay(500);
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    uint32_t adc_value = HAL_ADC_GetValue(hadc);
    serial_printf("ADC reading: %d\n", adc_value);
    HAL_ADC_Stop(hadc);

    // Change the thresholds
    uint32_t curr_upper = (hadc->Instance->TR >> 16) & 0x00000FFF;
    uint32_t curr_lower = (hadc->Instance->TR & 0x00000FFF);

    if (curr_upper != 0x0FFF) { //  we trig'd because something was placed onto the sensor
        serial_println("Something was placed on me!");
        // change upper threshold to max so that it can't be triggered due to something sitting on the pressure sensor
        hadc->Instance->TR = (0x0FFF << 16);
        hadc->Instance->TR |= curr_upper;
        HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
    } else {
        serial_println("Something was removed from me!");
        hadc->Instance->TR = (curr_lower << 16);
        hadc->Instance->TR &= ~(0x0000FFFF); // clear the lower threshold
        HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
    }

    serial_print("\n");

    HAL_ADC_Start(hadc);

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_SPI1_Init();
  MX_ADC_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  //Start Receive Buffer From ESP 8266
  /*
  HAL_UART_Receive_DMA(&huart1, UART1_rxBuffer, 600);

  serial_clear();
  serial_select(DEBUG_PRINT);
  serial_println("Hello world\n");

  serial_select(DEBUG_PRINT);
  serial_clear();
  serial_println("Hello world, this is a test of the new serial print functions!");
  serial_select(WIFI);

//    serial_printf("AT\n");
  if (setup_wifi("ASUS", "rickroll362") == AT_FAIL){
    // try again
  }
  if (sent_freshbyte_data(5000, 5000, 50000) == AT_FAIL){
    // try again
  }


  unsigned char * prediction_days = malloc(sizeof(char) * 15);
  prediction_days = receive_prediction(prediction_days);

  if (HAL_UART_Transmit(&huart2, "Predicted Days: ", sizeof("Predicted Days: "), 100) != HAL_OK){
    serial_println("Error! Predicted Days");
  }

  if (HAL_UART_Transmit(&huart2, prediction_days, sizeof(prediction_days), 100)!= HAL_OK){
    serial_println("Error! Printing string");
  }

  serial_select(DEBUG_PRINT);
  serial_clear();
  serial_printf("Predicted Days: ");
  serial_printf("%s\n", prediction_days);
  serial_println("Did you see that? I was chatting with the wi-fi module for a little bit ;)");
*/

  // Clear the serial debug terminal
  serial_select(DEBUG_PRINT);
  serial_clear();
  epd_init(false);

  // Disable prox interrupt while refreshing display
  VCNL4010_disable_Interrupt();

//  display_setup();

  serial_printf("Initializing I2C peripherals... ");
  hts_cal_data = hts221_init();
//  bq_init_ret = bq_init();
  VCNL4010_setLEDcurrent(20);
  VCNL4010_enable_Interrupt();
  serial_println("Done!");

  /*
  uint16_t voltage = BQ27441_voltage();
  uint16_t soc = BQ27441_soc(FILTERED);
  uint16_t current = BQ27441_current(AVG);
  uint16_t cap_remaining = BQ27441_capacity(REMAIN);
  uint16_t cap_max = BQ27441_capacity(DESIGN);
  int16_t power = BQ27441_power(); //average draw
  uint16_t soh = BQ27441_soh(PERCENT);
  uint16_t temp_bat = BQ27441_temperature(BATTERY) / 10;
  uint16_t temp_bq_IC = BQ27441_temperature(INTERNAL_TEMP) / 10;

  serial_printf("State of Charge\t\t\t\t%d\t%%\r\n", soc);
  serial_printf("Battery Voltage\t\t\t\t%d\tmV\r\n", voltage);
  serial_printf("Current\t\t\t\t\t%d\tmA\r\n", current);
  serial_printf("Max Capacity\t\t\t\t%d\tmAh\r\n", cap_max);
  serial_printf("Remaining Capacity\t\t\t%d\tmAh\r\n", cap_remaining);
  serial_printf("Ave power consumption\t\t\t%d\tmW\r\n", power);
  serial_printf("Health\t\t\t\t\t%d\t%%\r\n", soh);
  serial_printf("Battery Pack Temp\t\t\t%d\tK\r\n", temp_bat);
  serial_printf("Current Bat IC Temp is\t\t\t%d\tK\r\n", temp_bq_IC);
   */

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim2);

//  HAL_ADC_Start(&hadc);
//  HAL_ADC_Start_DMA(&hadc, adc_readings, 2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* TIM6_DAC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  //once buffer is full - restart
  HAL_UART_Receive_DMA(&huart1, UART1_rxBuffer, 600);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    serial_printf("Buffer is half full!\n");
//    HAL_DMA_IRQHandler(&hdma_adc);
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    serial_printf("Buffer is completely full!\n");
//    HAL_DMA_IRQHandler(&hdma_adc);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
