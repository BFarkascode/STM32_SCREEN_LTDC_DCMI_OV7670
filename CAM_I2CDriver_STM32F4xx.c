/*
 *  Created on: Jul 5, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.1
 *  File: I2CDriver_STM32F4xx.c
 *  Change history:
 *  	v.1.1: Changed from I2C1 to I2C2 to avoid overlapping pins
 */

/* Custom library to run I2C on F4xx
 * Be aware that the I2C peripheral of the F4xx is completely different than the L0xx
 * Unlike in the L0xx, we will have separate functions for generate Start/Stop conditions, send the Address and to Tx/Rx
 * Certain automatic capabilities like AUTOEND are not available in the F4xx
 * There are not separate registers for Tx/Rx/Address. All is sent from the same DR register*/

#include "CAM_I2CDriver_STM32F4xx.h"
#include "stm32f429xx.h"


void I2C2Config(void) {

	/*
	 * 1) Enable clocking
	 * 2) GPIO setup
	 * 3) Set clock source - this is not possible on the F4xx. Unlike the L0xx, here all I2Care clocked from APB only!
	 * 4) Set timing
	 * 5) Set own address, if any
	 * 6)Set filtering and clock stretch
	 * 7)Enable I2C
	 *
	 */

	//1)Enable clocking in the RCC, set I2CCLK source clock, enable GPIO clocking - PF1 SCL, PF0 SDA

	RCC->APB1ENR |= (1<<22);							//enable I2C2 clock
	RCC->AHB1ENR |=	(1<<5);								//PORTF clocking

	//2)Set GPIO parameters (mode, speed, pullup) - PB6 SCL, PB7 SDA
	GPIOF->MODER &= ~(1<<0);							//alternate function for PF0
	GPIOF->MODER |= (1<<1);								//alternate function for PF0
	GPIOF->MODER &= ~(1<<2);							//alternate function for PF1
	GPIOF->MODER |= (1<<3);								//alternate function for PF1
														//Note: MODER resets to 0x0 here!
	GPIOF->OTYPER |= (1<<0);							//open drain for PF0
	GPIOF->OTYPER |= (1<<1);							//open drain for PF1
	GPIOF->OSPEEDR |= (3<<0);							//high speed PF0
	GPIOF->OSPEEDR |= (3<<2);							//high speed PF1
	GPIOF->PUPDR |= (1<<0);								//pullup PF0
	GPIOF->PUPDR |= (1<<2);								//pullup PF1

	//Note: AFR values are in the device datasheet
	GPIOF->AFR[0] |= (4<<0);							//PB6 AF4 setup
	GPIOF->AFR[0] |= (4<<4);							//PB7 AF4 setup

	Delay_us(1);										//this delay is necessary, otherwise GPIO setup may freeze

	//reset I2C
	I2C2->CR1 |= (1<<15);
	I2C2->CR1 &= ~(1<<15);

	//3)Set a clock source for the internal clock of the I2C

	//N/A

	//4)Set timing - standard mode selected with 8 MHz I2CCLK

	//Standard timing
	I2C2->CR2 = 0x1e;									//frequency of the APB1 clock.
	I2C2->CCR = 0x96;									//value taken from CubeMX - CCR is clock control register
	I2C2->TRISE = 0x1f;									//value taken from CubeMX
														//Note: the I2C bus is rather flexible regarding clocking, though it is always best to be precise

	//5) Set own address

	//N/A

	//6)Set filtering and clock stretch

	I2C2->FLTR &= ~(1<<4);								//analog filter enabled
	I2C2->CR1 &= ~(1<<7);								//clock stretch enabled
														//this must be kept as such for MASTER mode
	//7)Enable I2C

	I2C2->CR1 |= (1<<0);								//enable I2C
	Delay_us(1);										//We wait for the setup to take effect
}


int I2C2SCANNER (uint8_t slave_addr) {
	/*
	 * This function is similar to the L0xx scan function.
	 * Note that we are not using the TX address specific function here. That is because the ADDR bit does not go HIGH during a NACK so that function gets into an infinite loop.
	 * Also, we have a flag called AF, not NACK.
	 * We reset the SR registers by reading them.
	 * Addressing is shifted by 1 where LSB becomes the indication of TX or RX (unlike in L0xx where we did this definition by setting bits in the CR2 register)
	 *
	 * Note: this function only sends the address of the slave to the bus, then resets.
	 */

	uint8_t reply = 0;

	I2C2->CR1 |= (1<<0);								//we turn on PE
	Delay_us(1);

	I2C2_Master_Start();								//we generate a start bit

	I2C2->DR = slave_addr << 1;							//we put the address into DR

	Delay_ms(1);										//we wait for the transfer to occur

	if ((I2C2->SR1 & (1<<10)) == (1<<10)) {				//if the AF flag is 1, we had a NO ACK!

		reply = 0;										//the init function returns "0" while the AF bit is 1
														//NACKF is 1 if there is no acknowledge for the slave address sent over
	} else {

		reply = 1;

	}

	uint32_t temp = I2C2->SR1 | I2C2->SR2;				//we reset SR registers

	I2C2_Master_Stop();

	I2C2->CR1 &= ~(1<<0);								//we turn off PE and reset the I2C bus
	Delay_ms(1);										//we need to wait 3 APB cycles after PE is cleared. We wait for 1 us instead.

	return reply;
}


void I2C2TX (uint8_t number_of_bytes, uint8_t *bytes_to_send) {
	/*
	 * A set of while conditions to progress through a simple TX.
	 * ONly the TXE and the BTF flags need to be checked
	 * Difference between TX and RX is done on the addressing level, not here
	 *
	 */

	while(!(I2C2->SR1 & (1<<7)));						//TXE flag
	while(number_of_bytes) {

		while(!(I2C2->SR1 & (1<<7)));
		I2C2->DR = (volatile uint8_t) *bytes_to_send++;
		number_of_bytes--;

	}

	while(!(I2C2->SR1 & (1<<2)));						//BTF flag

}


void I2C2_Master_Start(void){
	/*
	 * 1) We generate a start bit
	 * 2) We enable the ACK
	 * 3)We wait for the SB bit to go HIGH indicating the the start condition is generated
	 *
	 */

	//1)
	I2C2->CR1 |= (1<<8);								//we start

	//2)
	I2C2->CR1 |= (1<<10);								//ACK enabled

	//3)
	while(!(I2C2->SR1 & (1<<0)));						//we wait for the SB bit to be set

}


void I2C2_Address_TX(uint8_t slave_addr){

	/*
	 * 1) We load the slave address into the Tx/Rx/DR register
	 * 2) We wait until the ADDR bit is set, indicating that the address has been sent
	 * 3) We reset the SR registers
	 *
	 * Note: the ADDR bit ONLY goes HIGH, if there is a match in the address. As such, this function can only be used with known addresses
	 *
	 */

	//1)
	I2C2->DR = slave_addr << 1;

	//2)
	while(!(I2C2->SR1 & (1<<1)));						//wait for the ADDR bit to set

	//3)
	uint32_t temp = I2C2->SR1 | I2C2->SR2;				//reset SR registers

}


void I2C2_Master_Stop(void){

	/*
	 * Simple stop command to generate a stop condition on hte bus
	 *
	 */

	I2C2->CR1 |= (1<<9);

}
