/* USER CODE BEGIN Header */
/**
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  File: main.c
 *  Hardware description/pin distribution:
 *  					LTDC			- PA3, (PA4), (PA6), PA11, PA12, PB0, PB1, PB8, PB10, PB11, PC6, PC7, PD3, PF10, PG6, Pg7, PG10, PG11, PG12
 *  					DCMI 			- PA4, PA6, PA9, PA10, PB6, PC8, PC9, PC11, PE5, PE6, PG9
 *  					SPI				- PF7, PF8, PF9
 *  					ILI CS and DC	- PC2, PD13
 *  					I2C				- PF0, PF1
 *  					Master clock 	- PA7
 *  					CAM PWDN		- PF4
 *  Change history: N/A
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ClockDriver_STM32F4xx.h"
#include "CAM_Driver_OV7670.h"
#include "CAM_I2CDriver_STM32F4xx.h"
#include "SCR_ili9341.h"
#include "SCR_image_transfer.h"
#include "SCR_LTDCDriver_STM32F4xx.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t image[153600];																//this image will be stored in RAM!!!
uint8_t* image_read_ptr = &image[0];
uint32_t* image_write_ptr = &image[0];
uint8_t OV7670_address = 0x21;

uint8_t image_captured_flag = 0;
uint8_t frame_end_flag = 0;

uint8_t layer_published = 0;
uint8_t first_execution = 1;

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

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  SysClockConfig();

  TIM6Config();																		//custom delay function
  I2C2Config();																		//I2C for camera communications

  //---------Set up screen interface----------//

  SPI5_w_DMA_Config();
  DMA_SPI5_IRQPriorEnable();

  //---------Set up screen interface----------//

  //---------Set up screen----------//

#ifdef SPI_10_fps
  ILI9341_Init();
#endif

//#ifdef LTDC_30_fps
  ILI9341_RGB_Init();
  LTDC_Init();
//#endif

  //---------Set up screen----------//

  //---------Generate an image----------//
#ifdef generated_image_input

  GenerateImage();																	//this fills the frame buffer with a constant image

  //Note: we need to activate PA4 and PA6 for the pattern
  GPIOA->AFR[0] |= (14<<16);														//PA4 - AF14		LTDC_VS
  GPIOA->AFR[0] |= (14<<24);														//PA6 - AF14		LTDC_G2

  //Note: LTDC_IRQ should be deactivated wither by commenting out the enable line in the layer config or by commenting out the lines in the handler function itself

#endif
  //---------Generate an image----------//

//#ifdef cam_input

  //---------Set up camera----------//

  OV7670_Clock_Start();

  TIM3->CR1 |= (1<<0);																//clock start

  OV7670_Find();

  OV7670_init();

  //---------Set up camera----------//

  //---------Set up camera interface----------//

  OV7670_DCMI_DMA_init();

  Crop240x240();

  DMA_DCMI_IRQPriorEnable();

  //---------Set up camera interface----------//

//#endif


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  //---------Capture camera image----------//
//#ifdef cam_input

	  OV7670_Capture(image_write_ptr, 28800);
	  while(!frame_end_flag);

	  frame_end_flag = 0;

//#endif
	  //---------Capture camera image----------//

//#ifdef first_part_endian_swap
// this is the "lead" we will give the LTDC
	  for(int i = 0; i < 34800; i+=2){

		  uint8_t pix_buf;
		  pix_buf = image[i];
		  image[i] = image[i+1];
		  image[i+1] = pix_buf;

	  }
//#endif
	  //---------Publish image----------//

#ifdef SPI_10_fps
	  Transmit320x240Frame(image_read_ptr);											//here we read out what we have captured

	  Delay_ms(25);
#endif

//#ifdef LTDC_30_fps

	  Transmit_RGB_320x240Frame(image_read_ptr);
	  
//#ifdef second_part_endian_swap
	  for(int i = 34800; i < 115200; i+=2){						
		  
		  uint8_t pix_buf;
		  pix_buf = image[i];
		  image[i] = image[i+1];
		  image[i+1] = pix_buf;

	  }
//#endif

	  while(!layer_published);														//we don't move forward until the layer is published (see line trigger in the LTDC)

	  layer_published = 0;

//#endif

	  //---------Publish image----------//

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 240;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
