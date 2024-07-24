/*
 *  Created on: Jul 10, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_SPI_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.0
 *  Header file: SPIDriver_STM32F4xx.h
 *  Change history:
 */

#ifndef INC_SCR_SPIDRIVER_STM32F4XX_H_
#define INC_SCR_SPIDRIVER_STM32F4XX_H_

#include "ClockDriver_STM32F4xx.h"

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE

//FUNCTION PROTOTYPES
void SPI5_w_DMA_Config(void);
void SPI5Master_Addr_Tx (uint8_t reg_addr_write_to);
void SPI5Master_Single_Tx (uint8_t byte_to_send);
void SPI1MasterDMAEnable (uint32_t* frame_buf_location, uint16_t number_of_transfers);
void DMA_SPI5_IRQPriorEnable(void);

#endif /* INC_SCR_SPIDRIVER_STM32F4XX_H_ */
