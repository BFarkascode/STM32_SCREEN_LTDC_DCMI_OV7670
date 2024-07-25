/*
 *  Created on: Jul 5, 2024
 *  Author: BalazsFarkas
 *  Project: STM32_SCREEN_LTDC_DCMI
 *  Processor: STM32F429ZI
 *  Program version: 1.1
 *  Header file: Driver_OV7670.h
 *  Change history:
 */

#ifndef INC_CAM_DRIVER_OV7670_H_
#define INC_CAM_DRIVER_OV7670_H_

#include "CAM_I2CDriver_STM32F4xx.h"
#include "ClockDriver_STM32F4xx.h"
#include "stm32f429xx.h"


//LOCAL CONSTANT
static const uint8_t OV7670_QVGA[166][2] =		//init the OV7670 to run at 320x240 QVGA resolution
	{
	{0x3a, 0x04},								//line buffer. set to reserved
    {0x40, 0xd0},								//COM15 - data format - 11010000 (RGB565 picked, full range) - only works in tandem with COM7 to choose the RGB output 565 or 555 (OD 0xd0)
    {0x12, 0x14},								//COM7 - register reset, resolution selection, colour definition - 10100 - QVGA and RGB picked (works with COM15)	(OG 0x14)
												//change register to 0x04 for 640x480 VGA output
    {0x32, 0x80},								//HREF control - HREF control - 10000000 - default value, HREF edge offset
    {0x17, 0x16},								//HSTART - HSTART - 10110 - output format
    {0x18, 0x04},								//HSTOP
    {0x19, 0x02},								//VSTRT
    {0x1a, 0x7b},								//VSTOP
    {0x03, 0x06},								//VREF - vertical frame control - VREF end low bits
    {0x0c, 0x00},								//COM3 - MSB/LSB swap, tristate,DCW, scale - default 00
    {0x3e, 0x00},								//COM14 - PCLK divider, DCW scaling - divided by 1

    {0x70, 0x00},								//scaling registers
    {0x71, 0x00},
    {0x72, 0x11},
    {0x73, 0x00},

    {0xa2, 0x02},								//PCLK delay - default 02

    {0x11, 0x80},								//internal clock - default 80

    {0x7a, 0x20},								//gamma values
    {0x7b, 0x1c},
    {0x7c, 0x28},
    {0x7d, 0x3c},
    {0x7e, 0x55},
    {0x7f, 0x68},
    {0x80, 0x76},
    {0x81, 0x80},
    {0x82, 0x88},
    {0x83, 0x8f},
    {0x84, 0x96},
    {0x85, 0xa3},
    {0x86, 0xaf},
    {0x87, 0xc4},
    {0x88, 0xd7},
    {0x89, 0xe8},

    {0x13, 0xe0},								//COM8 - AGC, AEC - gain control, banding filter control - 11100000 - banding filter off, AGC off

    {0x00, 0x00},								//default gain settings

    {0x10, 0x00},								//exposure value

    {0x0d, 0x00},								//COM4 - windowing - full window

    {0x14, 0x28},								//COM9 - gain ceiling - 101000 - reserved values

    {0xa5, 0x05},								//banding filter control
    {0xab, 0x07},

    {0x24, 0x75},								//AGC/AEC controls
    {0x25, 0x63},
    {0x26, 0xA5},

    {0x9f, 0x78},								//luminescence values
    {0xa0, 0x68},
    {0xa1, 0x03},
    {0xa6, 0xdf},
    {0xa7, 0xdf},
    {0xa8, 0xf0},
    {0xa9, 0x90},

    {0xaa, 0x94},								//AEC alogrythm selection

    {0x13, 0xe5},								//COM8 - AGC, AEC - gain control, banding filter control - 11100101 - enable AGC
    {0x0e, 0x61},
    {0x0f, 0x4b},								//COM6 - optical black control, format counter reset  - 1000011 - format, disable href at optical black (OG 0x4b)

    {0x16, 0x02},								//RSVD

    {0x1e, 0x07},								//MFVP - mirror and flip - 10111 - vertical flip on image (OG 0x17) - 0x7 is no flipping/mirroring

    {0x21, 0x02},								//RSVD
    {0x22, 0x91},
    {0x29, 0x07},
    {0x33, 0x0b},
    {0x35, 0x0b},

    {0x37, 0x1d},								//ADC and current control
    {0x38, 0x71},
    {0x39, 0x2a},

    {0x3c, 0x78},								//COM12 - HREF control - 1111000 - allways has HREF, even when VSYNCH is low (OG 0x78)

    {0x4d, 0x40},								//DMPOS - dummy row - 1000000 - we have one dummy row after active row (OG 0x40)

    {0x4e, 0x20},

    {0x69, 0x00},								//gain control
// OG
//    {0x6b, 0x00},								//PLL control - default 00
//NEW
	//PLL
	{0x6b, 0x40},								//PLL control - x4 multiplier
												//we add PLL to the camera to speed up image capture
												//this is currently 30 fps

    {0x74, 0x19},								//RSVD
    {0x8d, 0x4f},
    {0x8e, 0x00},
    {0x8f, 0x00},
    {0x90, 0x00},
    {0x91, 0x00},
    {0x92, 0x00},
    {0x96, 0x00},
    {0x9a, 0x80},
    {0xb0, 0x84},
    {0xb1, 0x0c},
    {0xb2, 0x0e},
    {0xb3, 0x82},
    {0xb8, 0x0a},

    {0x43, 0x14},								//AWB control
    {0x44, 0xf0},
    {0x45, 0x34},
    {0x46, 0x58},
    {0x47, 0x28},
    {0x48, 0x3a},
    {0x59, 0x88},
    {0x5a, 0x88},
    {0x5b, 0x44},
    {0x5c, 0x67},
    {0x5d, 0x49},
    {0x5e, 0x0e},

    {0x64, 0x04},								//lens correction
    {0x65, 0x20},
    {0x66, 0x05},
    {0x94, 0x04},
    {0x95, 0x08},

    {0x6c, 0x0a},								//AWB
    {0x6d, 0x55},
    {0x6e, 0x11},
    {0x6f, 0x9f},
    {0x6a, 0x40},
    {0x01, 0x40},
    {0x02, 0x40},

    {0x13, 0xe7},								//COM8 - AGC (OG 0xe7)

    {0x15, 0x00},								//COM10 - HREF to HSYNCH, VSYNCH negation, etx. - 10 - vsynch is negative, href is NOT hysnch (OG 0x02)

    {0x4f, 0x80},								//MTX
    {0x50, 0x80},
    {0x51, 0x00},
    {0x52, 0x22},
    {0x53, 0x5e},
    {0x54, 0x80},
    {0x58, 0x9e},

    {0x41, 0x08},								//COM 16 - edge enhancement - works with EDGE and REG75 and REG76 - AWB gain enabled

	{0x3f, 0x00},								//edge enhancement

    {0x75, 0x05},								//REG75	-	edges
    {0x76, 0xe1},								//REG76 -	edges

    {0x4c, 0x00},								//de-noise strenght - default 00
    {0x77, 0x01},

    {0x3d, 0xc2},  								//COM13 - gamma control - 11000010 - gamma enable, UV swap
    {0x4b, 0x09},
    {0xc9, 0x60},								//uv stauration control

    {0x41, 0x38},								//COM16 - edge

    {0x56, 0x40},								//contrast control - default 40

    {0x34, 0x11},								//array current control

    {0x3b, 0x02},								//COM11 - night mode - 10 - no night mode

    {0xa4, 0x89},								//autofram adjustment

    {0x96, 0x00},								//RSVD
    {0x97, 0x30},
    {0x98, 0x20},
    {0x99, 0x30},
    {0x9a, 0x84},
    {0x9b, 0x29},
    {0x9c, 0x03},

    {0x9d, 0x4c},								//banding filter
    {0x9e, 0x3f},

    {0x78, 0x04},   							//RSVD
    {0x79, 0x01},
    {0xc8, 0xf0},
    {0x79, 0x0f},
    {0xc8, 0x00},
    {0x79, 0x10},
    {0xc8, 0x7e},
    {0x79, 0x0a},
    {0xc8, 0x80},
    {0x79, 0x0b},
    {0xc8, 0x01},
    {0x79, 0x0c},
    {0xc8, 0x0f},
    {0x79, 0x0d},
    {0xc8, 0x20},
    {0x79, 0x09},
    {0xc8, 0x80},
    {0x79, 0x02},
    {0xc8, 0xc0},
    {0x79, 0x03},
    {0xc8, 0x40},
    {0x79, 0x05},
    {0xc8, 0x30},
    {0x79, 0x26},

    {0x09, 0x03},								//COM2 - sleep mode - 11 - output drive capacity
    {0x3b, 0x42},								//COM11 - nigth mode - 1000010 - frame rate night mode
	{0xff, 0xff},
};


//LOCAL VARIABLE
static uint32_t DCMI_count = 0;
static uint32_t DMA_count = 0;

//EXTERNAL VARIABLE
extern uint8_t OV7670_address;					//the I2C address of the OV7670 camera
extern uint8_t frame_end_flag;

//FUNCTION PROTOTYPES
void OV7670_Clock_Start(void);
void OV7670_Find(void);
void OV7670_init(void);
void OV7670_DCMI_DMA_init(void);
void OV7670_Capture(uint32_t* frame_buf_location, uint16_t number_of_transfers);
void DMA_DCMI_IRQPriorEnable(void);
void Crop240x240(void);

#endif /* INC_CAM_DRIVER_OV7670_H_ */
