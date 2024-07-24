/*
 *  Created on: Jul 16, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.0
 *  File: SCR_LTDCDriver_STM32F4xx.c
 *  Change history:
 */

/*
 * The LTDC driver for the F429ZI DISCO board.
 */

#include "SCR_LTDCDriver_STM32F4xx.h"


void LTDC_Init(void){

	/*
	 * Configure the peripheral
	 *
	 * 1) Enable peripheries and GPIO clocks
	 * 2) GPIO setting for LTDC - PA4, PA6, PB8, PB9, PC6, PC7, PC8, PC9, PC11, PD3 and PG9
	 * 3) LTDC PLL clocking - PLL SAI, N x192, R /4, CDCLK /4 (output freq is 12 MHz eventually)
	 * 4) LTDC setup
	 *
	 */

	//1)
	RCC->APB2ENR |= (1<<26);													//enable LTDC interface

	RCC->AHB1ENR |=	(1<<0);														//PORTA clocking
	RCC->AHB1ENR |=	(1<<1);														//PORTB clocking
	RCC->AHB1ENR |=	(1<<2);														//PORTC clocking
	RCC->AHB1ENR |=	(1<<3);														//PORTD clocking
	RCC->AHB1ENR |=	(1<<5);														//PORTF clocking
	RCC->AHB1ENR |=	(1<<6);														//PORTG clocking


	//2)
	GPIOA->MODER &= ~(1<<6);													//alternate function for PA3
	GPIOA->MODER |= (1<<7);														//alternate function for PA3
	GPIOA->MODER &= ~(1<<8);													//alternate function for PA4 (DCMI HSYNCH overlap with LTDC_VSYNCH)
	GPIOA->MODER |= (1<<9);														//alternate function for PA4 (DCMI HSYNCH overlap with LTDC_VSYNCH)
	GPIOA->MODER &= ~(1<<12);													//alternate function for PA6 (DCMI PIXCLK overlap with LTDC_G2)
	GPIOA->MODER |= (1<<13);													//alternate function for PA6 (DCMI PIXCLK overlap with LTDC_G2)
	GPIOA->MODER &= ~(1<<22);													//alternate function for PA11
	GPIOA->MODER |= (1<<23);													//alternate function for PA11
	GPIOA->MODER &= ~(1<<24);													//alternate function for PA12
	GPIOA->MODER |= (1<<25);													//alternate function for PA12
	GPIOB->MODER &= ~(1<<0);													//alternate function for PB0
	GPIOB->MODER |= (1<<1);														//alternate function for PB0
	GPIOB->MODER &= ~(1<<2);													//alternate function for PB1
	GPIOB->MODER |= (1<<3);														//alternate function for PB1
	GPIOB->MODER &= ~(1<<16);													//alternate function for PB8 (DCMI D6 overlap with LCD_B6)
	GPIOB->MODER |= (1<<17);													//alternate function for PB8
	GPIOB->MODER &= ~(1<<18);													//alternate function for PB9 (DCMI D7 overlap with LCD_B7)
	GPIOB->MODER |= (1<<19);													//alternate function for PB9
	GPIOB->MODER &= ~(1<<20);													//alternate function for PB10
	GPIOB->MODER |= (1<<21);													//alternate function for PB10
	GPIOB->MODER &= ~(1<<22);													//alternate function for PB11
	GPIOB->MODER |= (1<<23);													//alternate function for PB11
	GPIOC->MODER &= ~(1<<12);													//alternate function for PC6 (DCMI D0 overlap with LCD_HSYNCH)
	GPIOC->MODER |= (1<<13);													//alternate function for PC6
	GPIOC->MODER &= ~(1<<14);													//alternate function for PC7 (DCMI D1 overlap with LCD_G6)
	GPIOC->MODER |= (1<<15);													//alternate function for PC7
	GPIOD->MODER &= ~(1<<6);													//alternate function for PD3 (DCMI D5 overlap with LCD_G7)
	GPIOD->MODER |= (1<<7);														//alternate function for PD3
	GPIOF->MODER &= ~(1<<20);													//alternate function for PF10
	GPIOF->MODER |= (1<<21);													//alternate function for PF10
	GPIOG->MODER &= ~(1<<12);													//alternate function for PG6
	GPIOG->MODER |= (1<<13);													//alternate function for PG6
	GPIOG->MODER &= ~(1<<14);													//alternate function for PG7
	GPIOG->MODER |= (1<<15);													//alternate function for PG7
	GPIOG->MODER &= ~(1<<20);													//alternate function for PG10
	GPIOG->MODER |= (1<<21);													//alternate function for PG10
	GPIOG->MODER &= ~(1<<22);													//alternate function for PG11
	GPIOG->MODER |= (1<<23);													//alternate function for PG11
	GPIOG->MODER &= ~(1<<24);													//alternate function for PG12
	GPIOG->MODER |= (1<<25);													//alternate function for PG12
																				//Note: MODER resets to 0x0 here!

	GPIOA->OSPEEDR |= (3<<6);													//high speed PA3
	GPIOA->OSPEEDR |= (3<<8);													//high speed PA4
	GPIOA->OSPEEDR |= (3<<12);													//high speed PA6
	GPIOA->OSPEEDR |= (3<<22);													//high speed PA11
	GPIOA->OSPEEDR |= (3<<24);													//high speed PA12
	GPIOB->OSPEEDR |= (3<<0);													//high speed PB0
	GPIOB->OSPEEDR |= (3<<2);													//high speed PB1
	GPIOB->OSPEEDR |= (3<<16);													//high speed PB8
	GPIOB->OSPEEDR |= (3<<18);													//high speed PB9
	GPIOB->OSPEEDR |= (3<<20);													//high speed PB10
	GPIOB->OSPEEDR |= (3<<22);													//high speed PB11
	GPIOC->OSPEEDR |= (3<<12);													//high speed PC6
	GPIOC->OSPEEDR |= (3<<14);													//high speed PC7
	GPIOD->OSPEEDR |= (3<<6);													//high speed PD3
	GPIOF->OSPEEDR |= (3<<20);													//high speed PF10
	GPIOG->OSPEEDR |= (3<<12);													//high speed PG6
	GPIOG->OSPEEDR |= (3<<14);													//high speed PG7
	GPIOG->OSPEEDR |= (3<<20);													//high speed PG10
	GPIOG->OSPEEDR |= (3<<22);													//high speed PG11
	GPIOG->OSPEEDR |= (3<<24);													//high speed PG12
																				//no pull-up/pull-down

	//Note: AFR values are in the device datasheet (AF9 or AF14). For DCMI, they all are AF13
	GPIOA->AFR[0] |= (14<<12);													//high speed PA3 - AF14		B5

//Note: WE MUST NOT ACTIVATE THE AF FOR PA4 AND PA6!!!
//	GPIOA->AFR[0] |= (14<<16);													//high speed PA4 - AF14		VS
//	GPIOA->AFR[0] |= (14<<24);													//high speed PA6 - AF14		G2

	GPIOA->AFR[1] |= (14<<12);													//high speed PA11 - AF14	R4
	GPIOA->AFR[1] |= (14<<16);													//high speed PA12 - AF14	R5
	GPIOB->AFR[0] |= (9<<0);													//high speed PB0 - AF9		R3
	GPIOB->AFR[0] |= (9<<4);													//high speed PB1 - AF9		R6
	GPIOB->AFR[1] |= (14<<0);													//high speed PB8 - AF14		B6
	GPIOB->AFR[1] |= (14<<4);													//high speed PB9 - ...		B7
	GPIOB->AFR[1] |= (14<<8);													//high speed PB10 - AF14	G4
	GPIOB->AFR[1] |= (14<<12);													//high speed PB11 - ...		G5
	GPIOC->AFR[0] |= (14<<24);													//high speed PC6			HS
	GPIOC->AFR[0] |= (14<<28);													//high speed PC7			G6
	GPIOD->AFR[0] |= (14<<12);													//high speed PD3			G7
	GPIOF->AFR[1] |= (14<<8);													//high speed PF10 - AF14	DE
	GPIOG->AFR[0] |= (14<<24);													//high speed PG6			R7
	GPIOG->AFR[0] |= (14<<28);													//high speed PG7			CLK
	GPIOG->AFR[1] |= (9<<8);													//high speed PG10 - AF9		G3
	GPIOG->AFR[1] |= (14<<12);													//high speed PG11 - AF14	B3
	GPIOG->AFR[1] |= (9<<16);													//high speed PG12 - AF9		B4

	Delay_us(1);																//this delay is necessary, otherwise GPIO setup may freeze

	//3
	RCC->PLLSAICFGR |= (192<<6);												//PLLSAI N
	RCC->PLLSAICFGR |= (4<<28);													//PLLSAI R
	RCC->DCKCFGR |= (1<<16);													//DIV 4
	RCC->CR |= (1<<28);															//we turn on the PLL SAI for LCD
	while (!(RCC->CR & (1<<29)));												//and wait until it becomes available

	NVIC_SetPriority(LTDC_IRQn, 1);												//IRQ priority for channel 1
	NVIC_EnableIRQ(LTDC_IRQn);													//IRQ enable for channel 1

}


void LTDC_320x240_RGB565_Config(uint32_t frame_buf_address){

	/*
	 * Configure the window (frame plus porches) and the layer we intend to feed into the window
	 */

	//LTDC config
	LTDC->SSCR = 0b10010000000000000001;										//horizontal synch height 9, vertical synch height 1 (these are 1 below the actual value)
	LTDC->BPCR = 0b111010000000000000011;										//horizontal back porch 29, vertical back porch 3
	LTDC->AWCR = 0b1000011010000000101000011;									//accumulated active width 269, active height 323
	LTDC->TWCR = 0b1000101110000000101000111;									//total width in clock periods 279 , total height in clocks 327
	LTDC->IER  = 0b1;															//line trigger
	LTDC->LIPCR = 239;															//total width line trigger
																				//Note: we publish a 320x240 image, but interrupt it at 240x240

	//layer config
	LTDC_Layer1->CR    = 0b1;													//layer enable
	LTDC_Layer1->WHPCR = 0b1000011010000000000011110;							//horizontal start and stop positions
	LTDC_Layer1->WVPCR = 0b1010000110000000000000100;							//vertical start and stop positions
	LTDC_Layer1->PFCR  = 0b10;													//RGB565 format
	LTDC_Layer1->CFBAR = frame_buf_address;										//address of the frame buffer
	LTDC_Layer1->CFBLR = 0b10100000000000001010000011;							//pixel pitch and line length (320*2 + 3)

	LTDC->SRCR = 0x1;															//update LTDC parameters
																				//Note: this bit is cleared once the update is done
	LTDC->GCR  = 0b1;															//LCD enabled
}


void LTDC_IRQHandler(void){

	/*
	 * IRQ handler for the line interrupt
	 *
	 * Note: comment out the two disable lines in case an image pattern is being used as input
	 */

	LTDC->ICR |= (1<<0);														//clear line interrupt flag

	//Note: in order to have a VSYNCH line for the ILI using the DCMI, we need to uncomment the following two lines
	LTDC->GCR  = 0x0;															//LCD disabled
	LTDC_Layer1->CR = 0x0;														//layer disabled

}
