/*
 * ili9341.h
 *
 *  Created on: 2019/12/26
 *      Author: Kotetsu Yamamoto
 *      Copyright [Kotetsu Yamamoto]

MIT License

Copyright (c) 2020 Kotestu Yamamoto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */

/*
 *  Modified on: Jul 11, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.1
 *  Header file: ili9341.h
 *  Change history:
 */

#ifndef INC_SCR_ILI9341_H_
#define INC_SCR_ILI9341_H_

#include "SCR_SPIDriver_STM32F4xx.h"
#include "ClockDriver_STM32F4xx.h"

 //LOCAL CONSTANT

 //LOCAL VARIABLE
typedef enum {
	ROTATE_0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270
} LCD_Horizontal_t;

 //EXTERNAL VARIABLE

 //FUNCTION PROTOTYPES
void ILI9341_Init(void);
void ILI9341_SetWindow(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y);
void ILI9341_DrawBitmap(uint16_t w, uint16_t h, uint8_t *s);
void LCD_WR_REG(uint8_t data);
void ILI9341_SoftReset(void);
void LCD_WR_REG(uint8_t data);
void LCD_WR_DATA(uint8_t data);
void LCD_direction(LCD_Horizontal_t direction);
void CS_L(void);
void DC_L(void);
void DC_H(void);
void ILI9341_RGB_Init(void);


#endif /* INC_SCR_ILI9341_H_ */
