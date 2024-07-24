/*
 *  Created on: Jul 4, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.1
 *  File: ClockDriver_STM32F4xx.c
 *  Change history:
 *  		v.1.1: Changed clocking frequency from 60 MHz to 120 MHz
 */

#include "ClockDriver_STM32F4xx.h"
#include "stm32f429xx.h"

//1)We set up the core clock and the peripheral prescalers/dividers
void SysClockConfig(void) {
	/*
	 * Similar to L0xx counterpart. See ClockDriver project.
	 *
	 */

	//1)
	//HSI16 on, wait for ready flag
	RCC->CR |= (1<<0);															//we turn on HSI16
	while (!(RCC->CR & (1<<1)));												//and wait until it becomes stable. Bit 2 should be 1.


	//2)
	//power control enabled, put to reset value
	RCC->APB1RSTR |= (1<<28);													//reset PWR interface
	PWR->CR |= (1<<14);															//we put scale1
	PWR->CR |= (1<<15);
	while ((PWR->CSR & (1<<14)));												//and regulator voltage selection ready

	//3)
	//Flash access control register - 3WS latency
	FLASH->ACR |= (3<<0);														//3 WS

	//4)Setting up the clocks

	//AHB 1
	RCC->CFGR &= ~(1<<4);														//no prescale
	RCC->CFGR &= ~(1<<5);
	RCC->CFGR &= ~(1<<6);

	//APB1 divided by 4
	RCC->CFGR |= (5<<10);														//we put 101 to bits [10:8]. This should be a DIV4.

	//APB2 divided by 4
	RCC->CFGR |= (5<<13);

	//PLL source HSI16
	RCC->PLLCFGR &= ~(1<<22);

	//PLL "M" division is 16
	RCC->PLLCFGR |= (16<<0);

	//PLL "N" multiplier is 240
	RCC->PLLCFGR |= (240<<6);

	//PLL "P" division is 2
	RCC->PLLCFGR &= ~(1<<16);
	RCC->PLLCFGR &= ~(1<<17);

	//PLL "MCO1" division is 2 - MCU output clock on PA8 - not used here
//	RCC->PLLCFGR |= (1<<26);

	//5)Enable PLL
	//enable and ready flag for PLL
	RCC->CR |= (1<<24);															//we turn on the PLL
	while (!(RCC->CR & (1<<25)));												//and wait until it becomes available

	//6)Set PLL as system source
	RCC->CFGR |= (2<<0);														//PLL as source
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);						//system clock status (set by hardware in bits [3:2]) should be matching the PLL source status set in bits [1:0]

	SystemCoreClockUpdate();													//This CMSIS function must be called to update the system clock! If not done, we will remain in the original clocking (likely MSI).
}

//2) TIM6 setup for precise delay generation
void TIM6Config (void) {
	/*
	 * Same as L0xx counterpart (except TIM6 clocking enable register name). See ClockDriver project.
	 */

	//1)
	RCC->APB1ENR |= (1<<4);														//enable TIM6 clocking

	//2)

	TIM6->PSC = 60 - 1;															// 120 MHz/60 =  MHz -- 500 us delay

	TIM6->ARR = 0xFFFF;															//Maximum ARR value - how far can the timer count?

	//3)
	TIM6->CR1 |= (1<<0);														//timer counter enable bit
	while(!(TIM6->SR & (1<<0)));												//wait for the register update flag - UIF update interrupt flag
																				//update the timer if we are overflown/underflow with the counter was reinitialised.
																				//This part is necessary since we can update on the fly. We just need to wait until we are done with a counting cycle and thus an update event has been generated.
																				//also, almost everything is preloaded before it takes effect
																				//update events can be disabled by writing to the UDIS bits in CR1. UDIS as LOW is UDIS ENABLED!!!s
}


//3) Delay function for microseconds
void Delay_us(int micro_sec) {
	/*
	 * Same as L0xx counterpart. See ClockDriver project.
	 */

	TIM6->CNT = 0;
	while(TIM6->CNT < micro_sec);												//Note: this is a blocking timer counter!
}


//4) Delay function for milliseconds
void Delay_ms(int milli_sec) {
	/*
	 * Same as L0xx counterpart. See ClockDriver project.
	 */

	for (uint32_t i = 0; i < milli_sec; i++){
		Delay_us(1000);															//we call the custom microsecond delay for 1000 to generate a delay of 1 millisecond
	}
}


//5) Setup for PWM using TIM2 and channel 1.
void TIM3_CH2_PWM_Config (uint16_t PWM_resolution, uint16_t PWM_pulse) {
	/*
	 * This is very much identical to the L0x2 solution except for the RCC register naming. Pleas check the ClockDriver project for details.
	 */

	//1)Enable PORT clocking in the RCC, set MODER and OSPEEDR - PA5 LED2

	RCC->AHB1ENR |=	(1<<0);														//PORTA clocking - PA7 is TIM3_CH2
	GPIOA->MODER &= ~(1<<14);													//alternate function for PA7
	GPIOA->MODER |= (1<<15);													//alternate function for PA7

	//OSPEEDR, OTYPER and PUPDR remain as-is since we don't need more than low speed, no open drain and no pull resistors

	//Note: AFR values are in the device datasheet for L4. For TIM3 CH2, it will be AF2 for PC7 as seen on page 78 of the datasheet.
	GPIOA->AFR[0] |= (1<<29);													//PA7 AF2 setup

	//2)Set TIM2_CH1
	RCC->APB1ENR |= (1<<1);														//enable TIM3 clocking
	TIM3->PSC = 1 - 1;															// We should clock TIM3 at 80 MHz. We don't need prescaler.
	TIM3->ARR = PWM_resolution;													//we set the resolution of the PWM - how far shall we count before we go back to 0
																				//here we should go until 4 to have 20 MHz

	//3)Adjust PWM
	TIM3->CCR2 = PWM_pulse;														//we set the duty cycle of the PWM - after counting at which point shall we pull the output in the other direction?
																				//Note: each channel has its own CCR register!
	TIM3->CCMR1 |= (1<<11);														//we activate the PWM functionality by output compare preload enabled
	TIM3->CCMR1 |= (6<<12);														//PWM mode 1 selected. CH2 will be "on" as long as CNT is lower than CCR1
	//NOTE: CCMR1 CC1S is reset to 2'b0, which means CH1 is an output.
	//NOTE: we keep the original polarity (active HIGH) of the output by keeping CCER CC1P as LOW.

	//4)Enable everything
	TIM3->CCER |= (1<<4);														//we enable CH2
	TIM3->CR1 |= (1<<7);														//ARP enabled (ARR will be buffered)
	TIM3->CR1 |= (1<<0);														//timer counter enable bit
	//NOTE: we are edge aligned so no modifications on CR1 CMS

	//5)Update timer registers
	TIM3->EGR |= (1<<0);														//we update TIM3 with the new parameters. This generates an update event.

	while(!(TIM3->SR & (1<<0)));												//wait for the register update flag - UIF update interrupt flag
																				//it will go LOW - and thus, allow the code to pass - when the EGR-controlled TIM2 update has occurred successfully and the registers are updated
																				//we have an update event at the end of each overflow - count up to - or underflow - count down to - moment. Count limit is defined by the ARR.
}
