/*
 *  Created on: Jul 5, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.1
 *  File: Driver_OV7670.c
 *  Change history:
 *  	v.1.1:  Changed camera pin distro to be more harmonised with parallel LTDC
 *  			The changes only impact data pins, so the old code (v.1.0) should still result in some image
 *  			Changed camera clocking PLL to x4
 *  			Added image crop function
 *
 */

/*Below are the basic functions to run the OV7670 camera attached to the F429 DISCO board
  Camera needs an external "master" clock signal of 12 MHz (or more). Clock signal is generated using PWM from the DISCO.
  Camera is driven using standard I2C.
  Camera output frequency is 24 MHz (pixel clock). Output is 8 bit parallel data, refresh with the pixel clock.
  Other outputs are vysnch and hsynch (frame and line end indicator flags). Their activity direction can be picked.
  DCMI runs on DMA.

  Note: an adapter board for the camera is HIGHLY recommended to limit noise */

#include "CAM_Driver_OV7670.h"

void OV7670_Clock_Start(void){
	/*
	 * We start the clock signal for the camera
	 *
	 */

	TIM3_CH2_PWM_Config(4,2);													//we start the 12 MHz clock signal on PA7
																				//60 MHz divided by 5 with a non-important duty cycle


}


void OV7670_Find(void){
	/*
	 * We search for the camera on the bus
	 *
	 */

	uint8_t ret;
	uint8_t device_addr;

	for(uint8_t i = 1; i<128; i++) {
		ret = I2C2SCANNER(i);

		if (ret == 0) {

				//do nothing

		}
		else{

			device_addr = i;													//we define the address of the slave device
																				//Note: this works only if we have just one slave on the bus!
		}
	}

	if(device_addr != OV7670_address) {

		printf("Error...OV7670 not found... \r\n");
		while(1);

	} else {

		  //do nothing

	}

}

void OV7670_init (void){

	/*
	 * We initialise the camera using the init matrix
	 *
	 */

	//we set the PWDN pin GPIO
	RCC->AHB1ENR |=	(1<<5);														//PORTF clocking
	GPIOF->MODER |= (1<<8);														//GPIO output for PF4
	GPIOF->MODER &= ~(1<<9);													//GPIO output for PF4

	int array_rows = sizeof(OV7670_QVGA)/sizeof(OV7670_QVGA[0]);				//this is going to be 166

	I2C2->CR1 |= (1<<0);														//we turn on PE
	Delay_us(1);

	for(int i = 0; i < array_rows; i++){

		I2C2_Master_Start();
		I2C2_Address_TX(OV7670_address);
		I2C2TX (2, &OV7670_QVGA[i][0]);
		I2C2_Master_Stop();

	}

	I2C2->CR1 &= ~(1<<0);														//we turn off PE and reset the I2C bus
	Delay_ms(1);																//we need to wait 3 APB cycles after PE is cleared. We wait for 1 us instead.

}


void OV7670_DCMI_DMA_init (void){

	/*
	 * We enable DCMI and the connected DMA.
	 *
	 * 1) Enable peripheries and GPIO clocks
	 * 2) GPIO setting for DCMI - PA4, PA6, PB8, PB9, PC6, PC7, PC8, PC9, PC11, PD3 and PG9
	 * 3) DCMI setup
	 * 4) DMA setup
	 * 5) DCMI enable - no DMA enable, that will be done locally in a function
	 *
	 * */

	//1)
	RCC->AHB2ENR |= (1<<0);														//enable DCMI interface
	RCC->AHB1ENR |= (1<<22);													//enable DMA2 clocking
																				//DCMI is on DMA2

	RCC->AHB1ENR |=	(1<<0);														//PORTA clocking
	RCC->AHB1ENR |=	(1<<1);														//PORTB clocking
	RCC->AHB1ENR |=	(1<<2);														//PORTC clocking
	RCC->AHB1ENR |=	(1<<3);														//PORTD clocking
	RCC->AHB1ENR |=	(1<<4);														//PORTE clocking
	RCC->AHB1ENR |=	(1<<6);														//PORTG clocking

	//2)Set GPIO parameters
	GPIOA->MODER &= ~(1<<8);													//alternate function for PA4 (overlap!)
	GPIOA->MODER |= (1<<9);														//alternate function for PA4
	GPIOA->MODER &= ~(1<<12);													//alternate function for PA6 (overlap!)
	GPIOA->MODER |= (1<<13);													//alternate function for PA6
	GPIOA->MODER &= ~(1<<18);													//alternate function for PA9 (moved from PC6)
	GPIOA->MODER |= (1<<19);													//alternate function for PA9
	GPIOA->MODER &= ~(1<<20);													//alternate function for PA10 (moved from PC7)
	GPIOA->MODER |= (1<<21);													//alternate function for PA10
	GPIOB->MODER &= ~(1<<12);													//alternate function for PB6 (moved from PD3)
	GPIOB->MODER |= (1<<13);													//alternate function for PB6
	GPIOC->MODER &= ~(1<<16);													//alternate function for PC8
	GPIOC->MODER |= (1<<17);													//alternate function for PC8
	GPIOC->MODER &= ~(1<<18);													//alternate function for PC9
	GPIOC->MODER |= (1<<19);													//alternate function for PC9
	GPIOC->MODER &= ~(1<<22);													//alternate function for PC11
	GPIOC->MODER |= (1<<23);													//alternate function for PC11
	GPIOE->MODER &= ~(1<<10);													//alternate function for PE5 (moved from PB8)
	GPIOE->MODER |= (1<<11);													//alternate function for PE5
	GPIOE->MODER &= ~(1<<12);													//alternate function for PE6 (moved from PB9)
	GPIOE->MODER |= (1<<13);													//alternate function for PE6
	GPIOG->MODER &= ~(1<<18);													//alternate function for PG9
	GPIOG->MODER |= (1<<19);													//alternate function for PG9
																				//Note: MODER resets to 0x0 here!

	GPIOA->OSPEEDR |= (3<<8);													//high speed PA4
	GPIOA->OSPEEDR |= (3<<12);													//high speed PA6
	GPIOA->OSPEEDR |= (3<<18);													//high speed PA9
	GPIOA->OSPEEDR |= (3<<20);													//high speed PA10
	GPIOB->OSPEEDR |= (3<<12);													//high speed PB6
	GPIOC->OSPEEDR |= (3<<16);													//high speed PC8
	GPIOC->OSPEEDR |= (3<<18);													//high speed PC9
	GPIOC->OSPEEDR |= (3<<22);													//high speed PC11
	GPIOE->OSPEEDR |= (3<<10);													//high speed PE5
	GPIOE->OSPEEDR |= (3<<12);													//high speed PE6
	GPIOG->OSPEEDR |= (3<<18);													//high speed PG9
																				//no pull-up/pull-down

	//Note: AFR values are in the device datasheet. For DCMI, they all are AF13
	GPIOA->AFR[0] |= (13<<16);													//high speed PA4 - AF13		HS
	GPIOA->AFR[0] |= (13<<24);													//high speed PA6 - AF13		PIXCLK
	GPIOA->AFR[1] |= (13<<4);													//high speed PA9			D0
	GPIOA->AFR[1] |= (13<<8);													//high speed PA10			D1
	GPIOB->AFR[0] |= (13<<24);													//high speed PB6 - AF13		D5
	GPIOC->AFR[1] |= (13<<0);													//high speed PC8			D2
	GPIOC->AFR[1] |= (13<<4);													//high speed PC9			D3
	GPIOC->AFR[1] |= (13<<12);													//high speed PC11			D4
	GPIOA->AFR[0] |= (13<<20);													//high speed PE5 - AF13		D6
	GPIOE->AFR[0] |= (13<<24);													//high speed PE6 - AF13		D7
	GPIOG->AFR[1] |= (13<<4);													//high speed PG9 - AF13		VS

	Delay_us(1);																//this delay is necessary, otherwise GPIO setup may freeze

	//3)
//	DCMI->CR |= (1<<1);															//snapshot mode
																				//Note: snapshot mode does not activate reliably. Continous mode is used instead.
	DCMI->CR |= (1<<7);															//VSYNCH active HIGH
																				//EDM and FCRC are 0x00 which indicates 8-bit width per pixel clock and all frames captured

	DCMI->IER |= (1<<0);														//frame captured IRQ activated

	//4)
	DMA2_Stream1->CR &= ~(1<<0);												//disable stream (just in case...)

	DMA2_Stream1->CR |= (1<<4);													//transfer complete IRQ activated
																				//Note: we will need the TC IRQ so as to reset the DMA after transfer since we are not using circular mode
	DMA2_Stream1->CR |= (1<<25);												//select channel 1
																				//DCMI is on CH1 Stream1 or Stream 7
	DMA2_Stream1->CR &= ~(1<<13);												//memory data size is 32 bits
	DMA2_Stream1->CR |= (1<<14);												//memory data size is 32 bits
	DMA2_Stream1->CR &= ~(1<<11);												//peri data size is also 32 bits
	DMA2_Stream1->CR |= (1<<12);												//peri data size is also 32 bits
	DMA2_Stream1->CR |= (1<<10);												//memory increment active
	DMA2_Stream1->PAR = (uint32_t)(&(DCMI->DR));								//we connect the DMA to the DCMI

	//5)
	DCMI->CR |= (1<<14);														//DCMI enabled
}


void OV7670_Capture(uint32_t* frame_buf_location, uint16_t number_of_transfers){
	/*
	 * Function to capture an image from the camera
	 *
	 * 1) We disable the DMA
	 * 2) We set up the DMA for the transfer
	 * 3) We enable the DMA and start the capture
	 *
	 */

	//1)
	DMA2_Stream1->CR &= ~(1<<0);												//disable stream

	//2)
	DMA2_Stream1->PAR = (uint32_t)(&(DCMI->DR));								//we connect the DMA to the DCMI
	DMA2_Stream1->NDTR = number_of_transfers;
	DMA2_Stream1->M0AR = (uint32_t)frame_buf_location;

	//3)
	DMA2_Stream1->CR |= (1<<0);													//enable stream
	DCMI->CR |= (1<<0);															//DCMI capture
																				//Note: snapshot mode automatically clears this bit so we don't need to reset it
	GPIOF->BSRR |= (1<<20);														//PWD goes LOW and the camera exits idle state
}


void DCMI_IRQHandler(void){

	//when the DCMI has captured the frame, we stop the camera's clocking
	DCMI->ICR |= (1<<0);
	DCMI->CR &= ~(1<<0);														//stop capture
	GPIOF->BSRR |= (1<<4);														//PWD goes HIGH, we put the camera to IDLE
	frame_end_flag = 1;
}


void DMA2_Stream1_IRQHandler(void)
{
	/*
	 * We need to reset the DMA when transfer is done
	 *
	 */

   DMA2->LIFCR |= (1<<11);														//clear transfer complete flag for stream 1
   	   	   	   	   	   	   	   	   												//Note: the transfer complete flag goes HIGH and then blocks the DMA from re-engaging until cleared
}

void DMA_DCMI_IRQPriorEnable(void) {
	/*
	 * We call the two special CMSIS functions to set up/enable the IRQ for the DCMI and the attached DMA.
	 *
	 */

	NVIC_SetPriority(DMA2_Stream1_IRQn, 1);
	NVIC_EnableIRQ(DMA2_Stream1_IRQn);
	NVIC_SetPriority(DCMI_IRQn, 1);
	NVIC_EnableIRQ(DCMI_IRQn);
}

void Crop240x240(void){

	/*
	 * We use the DCMI to crop the incoming 320x240 image to 240x240
	 *
	 */

	//DCMI_CSTRT HOFFCNT bit defines first column value to crop from (should be (320 - 240) / 2 = 40)
	//DCMI_CSTRT VST bit defines first row value to crop from (should be 0)
	//DCMI_SIZE CAPCNT bit defines row length (should be 240)
	//DCMI_SIZE VLINE bit defines column length	(should be 240 pixels, which are 480 PIXCLK)

	//Note: we are working with PIXCLKs, so everything should be multiplied by 2
	//also, 0 in the register is 1 PIXCLK already, so 240 pixels should be value 479 in the register

	DCMI->CR &= ~(1<<14);														//DCMI disabled
	DCMI->CR |= (1<<2);															//crop on
	DCMI->CWSTRTR = 80;															//HOFFCNT
	DCMI->CWSIZER |= (479<<0);													//CAPCNT
	DCMI->CWSIZER |= (479<<16);													//VLINE
	DCMI->CR |= (1<<14);														//DCMI enabled

}
