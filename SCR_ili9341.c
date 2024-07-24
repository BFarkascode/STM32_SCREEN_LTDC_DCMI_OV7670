/*
 * REWORK of the code by Kotetsu Yamamoto
 * Bare-bones version of the original driver with everything that is not crucial being dropped
 * Also, HAL SPI and DMA driving is replaced with custom one.
 * Removed HAL.
 *
 */

/*
 *  Modified on: Jul 11, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.1
 *  File: ili9341.c
 *  Change history:
 *  		v.1.1: Added fucntion to calibrate ILI for RGB interface
 */

#include "SCR_ili9341.h"
#include "stm32f429xx.h"


void ILI9341_Init(void)
{
	//Initialization
	ILI9341_SoftReset();

	/* Pixel Format Set */
	LCD_WR_REG(0x3A);
	LCD_WR_DATA(0x05);

	LCD_WR_REG(0xB1);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x18);

	LCD_WR_REG(0x11);

	Delay_ms(60);

	//TURN ON DISPLAY
	LCD_WR_REG(0x29);

	LCD_direction(ROTATE_0);
}


void ILI9341_SetWindow(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
	//Set the draw window for the screen
	//Note: the window must be the same size as the data we intend to publish
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(start_x >> 8);
	LCD_WR_DATA(0xFF & start_x);
	LCD_WR_DATA(end_x >> 8);
	LCD_WR_DATA(0xFF & end_x);

	LCD_WR_REG(0x2b);
	LCD_WR_DATA(start_y >> 8);
	LCD_WR_DATA(0xFF & start_y);
	LCD_WR_DATA(end_y >> 8);
	LCD_WR_DATA(0xFF & end_y);

}

static void ConvHL(uint8_t *s, int32_t l)
{
	//Calculations
	uint8_t v;
	while (l > 0) {
		v = *(s+1);
		*(s+1) = *s;
		*s = v;
		s += 2;
		l -= 2;
	}
}

void ILI9341_DrawBitmap(uint16_t w, uint16_t h, uint8_t *s)
{
	// Enable to access GRAM
	LCD_WR_REG(0x2c);
	DC_H();
	ConvHL(s, (int32_t)w*h*2);
	SPI1MasterDMAEnable ((uint32_t*)s, w * h *2);
}

void ILI9341_SoftReset(void)
{
	//Reset screen
	uint8_t cmd;
	cmd = 0x01; //Software reset
	DC_L();
	SPI5Master_Addr_Tx(cmd);
}

void LCD_WR_REG(uint8_t data)
{
	//This is to send the address of a register to the screen
	DC_L();
	SPI5Master_Addr_Tx(data);

}

void LCD_WR_DATA(uint8_t data)
{
	//This is to send data to the screen, byte-by-byte
	DC_H();
	SPI5Master_Single_Tx(data);

}

void LCD_direction(LCD_Horizontal_t direction)
/*
 * Define the publishing direction from the frame buffer to the screen
 * This is necessary to align the data readout to the image stored.
 * Note: the image readout MUST align with the image information
 *
 * */
{
	switch (direction) {
	case ROTATE_0:
		LCD_WR_REG(0x36);
		LCD_WR_DATA(0x48);
		break;
	case ROTATE_90:
		LCD_WR_REG(0x36);
		LCD_WR_DATA(0x28);
		break;
	case ROTATE_180:
		LCD_WR_REG(0x36);
		LCD_WR_DATA(0x88);
		break;
	case ROTATE_270:
		LCD_WR_REG(0x36);
		LCD_WR_DATA(0xE8);
		break;
	}
}

void CS_L(void)
{
	//GPIO output for the hardware SPI CS pin
	GPIOC->BSRR |= (1<<18);

}

void DC_L(void)
{
	//ILI specificic GPIO output pin to select between data and command on the ILI
	GPIOD->BSRR |= (1<<29);
}

void DC_H(void)
{

	GPIOD->BSRR |= (1<<13);

}


void ILI9341_RGB_Init(void)
{
	//Initialization
	ILI9341_SoftReset();

	LCD_WR_REG(0xB1);							//frame rate control in normal mode
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x1f);							//61 Hz refresh rate


	LCD_WR_REG(0xF6);							//reset value is 0x01, 0x00, 0x00 so RIM 0, DM 0
	LCD_WR_DATA(0x01);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x06); 							//RIM 0b1
												//RM 0b1
												//MDT bits are "don't care" for RGB565

	LCD_WR_REG(0xB0);	  						//select RGB interface control
	LCD_WR_DATA(0xC2);    						//RCM 0b10 for DE mode, 0b11 for synch mode
												//polarity:
														//DE active HIGH
														//CLK rising edge
														//HYSNCH active LOW
														//VSYNCH active LOW
														//GRAM not bypassed

	LCD_WR_REG(0x3A);	  						//select colour resolution
	LCD_WR_DATA(0x55);    						//16bit - for RGB interface and MCU interface


	LCD_WR_REG(0x11);

	Delay_ms(60);

	//TURN ON DISPLAY
	LCD_WR_REG(0x29);

	LCD_direction(ROTATE_0);

}

