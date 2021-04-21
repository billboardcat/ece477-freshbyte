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
unsigned char UART1_rxBuffer[200] = {0};
HTS_Cal * hts_cal_data;
int bq_init_ret;
//TODO - make this an enum? for battery state?
enum battery_state batteryState = LOW;
enum system_state systemState = WAITING;
uint32_t adc_readings[2], adc_dma_buffer[2];
unsigned char prediction_str[2];
extern DMA_HandleTypeDef hdma_adc;
bool food_present = false;
extern uint32_t buffer1_size;
extern uint8_t *buffer1;

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
  write_RAM_to_epd(buffer1, buffer1_size, 0, false);
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

    // Change the thresholds
    uint32_t curr_upper = (hadc->Instance->TR >> 16) & 0x00000FFF;
    uint32_t curr_lower = (hadc->Instance->TR & 0x00000FFF);

    if (curr_upper != 0x0FFF) { //  we trig'd because something was placed onto the sensor
        serial_println("Something was placed on me!");
        // change upper threshold to max so that it can't be triggered due to something sitting on the pressure sensor
        hadc->Instance->TR = (0x0FFF << 16);
        hadc->Instance->TR |= curr_upper;
    } else {
        serial_println("Something was removed from me!");
        hadc->Instance->TR = (curr_lower << 16);
        hadc->Instance->TR &= ~(0x0000FFFF); // clear the lower threshold
    }

    serial_println("*** END OF ADC WATCHDOG INTERRUPT ***\n");
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

  // Disable prox interrupt while refreshing display
  VCNL4010_disable_Interrupt();

  //Start Receive Buffer From ESP 8266
  HAL_UART_Receive_DMA(&huart1, UART1_rxBuffer, 600);

  //WIFI Test
  serial_select(DEBUG_PRINT);
  serial_clear();
  serial_print("Setting up Wi-Fi... ");
  serial_select(WIFI);
  if (setup_wifi("ASUS", "rickroll362") == AT_FAIL){
    // try again
  }
  serial_select(DEBUG_PRINT);
  serial_println("Done!");
//  if (sent_freshbyte_data(5000, 5000, 50000) == AT_FAIL){
//    // try again
//  }
//  receive_prediction(prediction_str);
//  if (HAL_UART_Transmit(&huart2, "Predicted Days: ", sizeof("Predicted Days: "), 100) != HAL_OK){
//    serial_println("Error! Predicted Days");
//  }
//  if (HAL_UART_Transmit(&huart2, prediction_str, sizeof(prediction_str), 100)!= HAL_OK){
//    serial_println("Error! Printing string");
//  }
//  serial_select(DEBUG_PRINT);
//  serial_clear();
//  serial_printf("Predicted Days: ");
//  serial_printf("%s\n", prediction_str);
//  serial_println("Did you see that? I was chatting with the wi-fi module for a little bit ;)");

  // Test red and green LEDs
  serial_print("Testing LEDs... ");
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
  serial_println("Done!");

  // Setup display and I2C peripherals
  display_setup();

  serial_printf("Initializing I2C peripherals... ");
  hts_cal_data = hts221_init();
  bq_init_ret = bq_init();
//  VCNL4010_setLEDcurrent(20);
//  VCNL4010_enable_Interrupt();
  serial_println("Done!");

  if (HAL_ADC_Start_DMA(&hadc, adc_dma_buffer, 2) != HAL_OK) {
    serial_println("!!! Failed to start ADC DMA");
  } else {
    serial_println("ADC DMA Started");
  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (systemState == WAITING) {
//      HAL_TIM_Base_Start_IT(&htim2); // Don't need this anymore, using physical debouncing
      
    }
    else if (systemState == MONITORING) {
      HAL_TIM_Base_Start_IT(&htim6);
      VCNL4010_setLEDcurrent(20);
      VCNL4010_enable_Interrupt();
    }
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
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 2, 0);
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

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  serial_print("*DMA* Updating ADC values... ");
  for (int i =0; i<2; i++)
  {
    adc_readings[i] = adc_dma_buffer[i];  // store the values in adc[]
  }
  serial_println("Done!");
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
