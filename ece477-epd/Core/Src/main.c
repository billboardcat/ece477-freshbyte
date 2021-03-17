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
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include "serial_print.h"
#include "sram.h"
#include "epd.h"
#include "epd_gfx.h"
#include "apple.c"
#include "vcnl4010.h"
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
extern uint8_t *buffer1;
extern uint32_t buffer1_size;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

    serial_clear();
    serial_println("NOTE: Not all GFX and SRAM functions are tested here\n");

    /***** SRAM TESTING CODE *****/

    serial_printf("Initializing display SRAM... ");
    sram_init();
    serial_println("Done!");

    uint8_t write_value = 0x42;
    serial_printf("Testing SRAM write... ");
    sram_erase(0x0008, 2, write_value);
    serial_println("Done!");
    serial_printf("[0x0008] <== 0x%x\n", write_value);

    serial_printf("Testing SRAM read... ");
    uint16_t read_val = sram_read8(0x0008, MCPSRAM_READ);
    serial_println("Done!");
    serial_printf("[0x0008] ==> 0x%x\n\n", read_val);


    /***** DISPLAY TESTING CODE *****/
    /*** INITIALIZATION ***/
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
    serial_println("Done!\n");

    /*** BITMAP ***/
    serial_printf("Drawing bitmap to buffer... ");
    draw_bitmap(0, 0, apple, EPD_WIDTH, EPD_HEIGHT, EPD_BLACK);
    serial_println("Done!");

    serial_printf("Manually writing buffer to the display (for color)... ");
    epd_powerUp();
    write_RAM_to_epd(buffer1, buffer1_size, 1, false);
    serial_println("Done!");

    serial_printf("Clearing display buffers... ");
    clear_buffer();
    serial_println("Done!\n");

    /*** TEXT ***/
    serial_printf("Setting text scaling... ");
    set_text_scale(2);
    serial_println("Done!");

    serial_printf("Setting x-margin... ");
    set_x_margin(2);
    serial_println("Done!");

    serial_printf("Setting cursor... ");
    set_cursor(2,2);
    serial_println("Done!");

	serial_printf("Printing to buffer... ");
	char hello[] = "Hello world,\n\n      I'm FreshByte!";
	printString(hello);
    serial_println("Done!");

	serial_printf("Writing buffer to the display... ");
	display(false);
    serial_println("Done!\n");

    serial_printf("Initial testing sequence done!\n\n");

    serial_printf("Clearing display buffers... ");
    clear_buffer();
    epd_powerUp();
    write_RAM_to_epd(buffer1, buffer1_size, 0, false);
    write_RAM_to_epd(buffer1, buffer1_size, 1, false);
    serial_println("Done!\n");

    /*** MINI INTEGRATION ***/
    serial_println("Starting Proximity Demo...\n");
    set_text_scale(0);
    set_x_margin(2);
    set_y_margin(2);
    set_cursor(2,2);

//    serial_printf("cursor: (%d, %d)\nmargin: %d\n\n", get_x_cursor(), get_y_cursor(), get_x_margin());

    printString("Initializing proximity sensor...\n");
    VCNL4010_setLEDcurrent(2);
//    display(false);

//    serial_printf("cursor: (%d, %d)\nmargin: %d\n\n", get_x_cursor(), get_y_cursor(), get_x_margin());

    printString("-- IR LED Current: 20 mA\n");
//    display(false);

//    serial_printf("cursor: (%d, %d)\nmargin: %d\n\n", get_x_cursor(), get_y_cursor(), get_x_margin());

    VCNL4010_setFrequency(VCNL4010_3_90625);
    printString("-- PROX Frequency: ~4 meas/sec\n\n");
    display(false);

//    serial_printf("cursor: (%d, %d)\nmargin: %d\n\n", get_x_cursor(), get_y_cursor(), get_x_margin());

    uint16_t temp;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    temp = VCNL4010_readAmbient();
    serial_printf("AMBIENT (RAW): %d\n", temp);
    printString("AMBIENT (RAW): ");
    printUnsigned(temp, 10);

//    serial_printf("Cursor: (%d, %d), Margin: %d\n\n", get_x_cursor(), get_y_cursor(), get_x_margin());

    temp = VCNL4010_readProximity();
    serial_printf("PROX (RAW): %d\n", temp);
    printString("\nPROX (RAW): ");
    printUnsigned(temp, 10);
    printChar('\n');
    display(false);
//    serial_printf("Cursor: (%d, %d), Margin: %d\n\n", get_x_cursor(), get_y_cursor(), get_x_margin());
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
    while (1) {
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
