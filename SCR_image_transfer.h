/*
 *  Created on: Jun 26, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.1
 *  Header file: image_transfer.h
 *  Change history:
 */

#ifndef INC_SCR_IMAGE_TRANSFER_H_
#define INC_SCR_IMAGE_TRANSFER_H_

#include <stdint.h>
#include "SCR_ili9341.h"

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE
extern uint8_t image[153600];
//extern uint8_t image[240][640];
//extern uint8_t image[640][240];

//FUNCTION PROTOTYPES
void Transmit320x240Frame(uint8_t* half_pixels);
void Transmit_RGB_320x240Frame(uint8_t* half_pixels);
uint8_t* GenerateImage(void);
void Transmit_RGB_320x240Frame(uint8_t* half_pixels);

#endif /* INC_SCR_IMAGE_TRANSFER_H_ */
