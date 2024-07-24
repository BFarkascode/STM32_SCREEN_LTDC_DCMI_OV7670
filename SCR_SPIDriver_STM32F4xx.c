/*
 *  Created on: Jul 10, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.0
 *  File: SPIDriver_STM32F4xx.c
 *  Change history:
 */

/*
 * The SPI driver is identical to the L0xx. Nevertheless, I changed it a bit - put the addressing into a separate function and removed multi publishing - to allow compatibility with the existing ILI driver.
 */

#include "SCR_SPIDriver_STM32F4xx.h"
#include "stm32f429xx.h"


void SPI5_w_DMA_Config(void){

	/*
	 * SPI5 TX is on DMA2 Stream 4 Channel 2
	 *
	 * 1) Enable peripheries and GPIO clocks
	 * 2) GPIO setting
	 * 3) SPI5 setup
	 * 4) DMA setup
	 *
	 * SPI is on PF7,PF8 and PF9 on the DISCO board.
	 * DC pin for the ILI is on PD13.
	 * CS pin for the ILI is on PC2.
	 *
	 * Note: SSM needs to be set in order to allow internal interfacing with the ILI chip through SPI
	 * Note: no SPI5 enable and no DMA enable, both will be done locally in a function to allow non-DMA communication when needed
	 *
	 */

	//1)
	RCC->APB2ENR |= (1<<20);													//enable SPI5 interface
	RCC->AHB1ENR |= (1<<22);													//enable DMA2 clocking
																				//SPI5 is on DMA2

	RCC->AHB1ENR |=	(1<<2);														//PORTC clocking
	RCC->AHB1ENR |=	(1<<3);														//PORTD clocking
	RCC->AHB1ENR |=	(1<<5);														//PORTF clocking

	//2)
	GPIOF->MODER &= ~(1<<14);													//alternate function for PF7
	GPIOF->MODER |= (1<<15);													//alternate function for PF7
	GPIOF->MODER &= ~(1<<16);													//alternate function for PF8
	GPIOF->MODER |= (1<<17);													//alternate function for PF8
	GPIOF->MODER &= ~(1<<18);													//alternate function for PF9
	GPIOF->MODER |= (1<<19);													//alternate function for PF9


	GPIOF->AFR[0] |= (5<<28);													//high speed PF7 - AF5
	GPIOF->AFR[1] |= (5<<0);													//high speed PF8 - AF5
	GPIOF->AFR[1] |= (5<<4);													//high speed PF9 - AF5

	Delay_us(1);																//this delay is necessary, otherwise GPIO setup may freeze

	//CS pin
	GPIOC->MODER |= (1<<4);														//GPIO output for PC2
	GPIOC->MODER &= ~(1<<5);													//GPIO output for PC2

	//DC pin
	GPIOD->MODER |= (1<<26);													//GPIO output for PD13
	GPIOD->MODER &= ~(1<<27);													//GPIO output for PD13

	Delay_us(1);																//this delay is necessary, otherwise GPIO setup may freeze


	//3
	SPI5->CR1 |= (1<<2);														//Master mode
	SPI5->CR1 |= (1<<9);														//SSM mode is software
	SPI5->CR1 |= (1<<8);														//we put a HIGH value on the SSI
	SPI5->CR1 &= ~(1<<3);														//no prescale
	SPI5->CR2 &= ~(1<<4);														//Motorola mode


	//4)
	DMA2_Stream4->CR &= ~(1<<0);												//disable stream (just in case...)

	DMA2_Stream4->CR |= (1<<4);													//transfer complete IRQ activated
																				//Note: we will need the TC IRQ so as to reset the DMA after transfer since we are not using circular mode
	DMA2_Stream4->CR |= (1<<26);												//select channel 2
																				//SPI5 TX is on CH2 Stream4
	DMA2_Stream4->CR |= (1<<6);													//publishing towards periphery from memory
	DMA2_Stream4->CR &= ~(1<<13);												//memory data size is 8 bits
	DMA2_Stream4->CR &= ~(1<<14);												//memory data size is 8 bits
	DMA2_Stream4->CR &= ~(1<<11);												//peri data size is 8 bits
	DMA2_Stream4->CR &= ~(1<<12);												//peri data size is 8 bits
	DMA2_Stream4->CR |= (1<<10);												//memory increment active

}

//2) Master write address
void SPI5Master_Addr_Tx (uint8_t reg_addr_write_to) {
	/*
	 * Same as L0xx counterpart just picked in two parts to comply ILI driver setup. No DMA used here.
	 */

	while(((SPI5->CR2 & (1<<1) == (1<<1))));									//we wait until DMA is finished the transfer

	SPI5->CR1 |= (1<<6);														//SPI enabled. SCK is enabled
	SPI5->DR = reg_addr_write_to;												//we write a value into the DR register
	while(!((SPI5->SR & (1<<1)) == (1<<1)));									//wait for the TXE flag to go HIGH and indicate that the TX buffer has been transferred to the shift register completely
	while(!((SPI5->SR & (1<<0)) == (1<<0)));									//we wait for the RXNE flag to go HIGH, indicating that the command has been transferred successfully
	uint32_t buf_junk = SPI5->DR;												//we reset the RX flag

}


//2) Master write one byte to register
void SPI5Master_Single_Tx (uint8_t byte_to_send) {
	/*
	 * Same as L0xx counterpart but without the addressing and the multi publishing. No DMA used.
	 */

	while(((SPI5->CR2 & (1<<1) == (1<<1))));									//we wait until DMA is finished the transfer

	SPI5->DR = byte_to_send;													//we load the byte into the Tx buffer
	while(!((SPI5->SR & (1<<1)) == (1<<1)));									//wait for the TXE flag to go HIGH and indicate that the TX buffer has been transferred to the shift register completely
	while(!((SPI5->SR & (1<<0)) == (1<<0)));									//we wait for the RXNE flag to go HIGH, indicating that the data has been transferred successfully
	uint32_t buf_junk = SPI5->DR;												//we reset the RX flag

//	while(((SPI5->SR & (1<<0)) == (1<<0)));										//we check that the Rx buffer is indeed empty
	while((SPI5->SR & (1<<7)) == (1<<7));										//we wait until BSY is LOW

}


void SPI1MasterDMAEnable (uint32_t* frame_buf_location, uint16_t number_of_transfers) {
	/*
	 * Below we enable the DMA communication and start the SPI peripheral
	 * Mind, this will need to be separately compared to the manual mode since the DMA is an automatic system that needs to be started.
	 *
	 * Note: while the DMA for the DCMI runs 32-bit wide, the SPI5 DMA runs 8-bit wide. This limits data transfer.
	 * */

	while(((SPI5->CR2 & (1<<1) == (1<<1))));									//we wait until the previous DMA transfer is finished

	//1)
	DMA2_Stream4->CR &= ~(1<<0);												//disable stream

	//2)
	DMA2_Stream4->PAR = (uint32_t)(&(SPI5->DR));								//we connect the DMA to SPI5								//we connect the DMA to the DCMI
	DMA2_Stream4->NDTR = number_of_transfers;
	DMA2_Stream4->M0AR = (uint32_t)frame_buf_location;

	//3)
	DMA2_Stream4->CR |= (1<<0);													//enable stream
	SPI5->CR2 |= (1<<1);														//TX DMA set. We generate a DMA request every time the transmit buffer is empty (TXE goes HIGH)

}


void DMA2_Stream4_IRQHandler(void)
{
	/*
	 * We need to reset the DMA when transfer is done
	 *
	 */

	DMA2->HIFCR |= (1<<5);														//clear transfer complete flag for stream 4
	   	   	   	   	   	   	   	   												//Note: the transfer complete flag goes HIGH and then blocks the DMA from re-engaging until cleared

	SPI5->CR2 &= ~(1<<1);														//SPI5 TX DMA disabled

}

void DMA_SPI5_IRQPriorEnable(void) {
	/*
	 * We call the two special CMSIS functions to set up/enable the IRQ.
	 *
	 */

	NVIC_SetPriority(DMA2_Stream4_IRQn, 1);										//IRQ priority for channel 1
	NVIC_EnableIRQ(DMA2_Stream4_IRQn);											//IRQ enable for channel 1
}

