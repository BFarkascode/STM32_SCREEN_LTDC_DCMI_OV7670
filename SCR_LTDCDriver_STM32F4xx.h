/*
 *  Created on: Jul 16, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.0
 *  Header file: SCR_LTDCDriver_STM32F4xx.h
 *  Change history:
 */

#ifndef INC_SCR_LTDCDRIVER_STM32F4XX_H_
#define INC_SCR_LTDCDRIVER_STM32F4XX_H_

#include "stm32f429xx.h"

//LOCAL CONSTANT

//LOCAL VARIABLE
static uint8_t enable_layer_false_trigger = 1;

//EXTERNAL VARIABLE
extern uint8_t layer_published;

//FUNCTION PROTOTYPES
void LTDC_Init(void);
void LTDC_320x240_RGB565_Config(uint32_t frame_buf_address);

#endif /* INC_SCR_LTDCDRIVER_STM32F4XX_H_ */
