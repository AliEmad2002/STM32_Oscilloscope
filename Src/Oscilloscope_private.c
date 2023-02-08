/*
 * Oscilloscope_private.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"

/*	MCAL	*/
#include "RCC_interface.h"
#include "SCB_interface.h"
#include "NVIC_interface.h"
#include "STK_interface.h"
#include "DMA_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "EXTI_interface.h"
#include "ADC_interface.h"

/*	HAL	*/
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"



/*	TFT LCD object	*/
volatile TFT2_t Global_LCD;

/*
 * This is the image buffer.
 * This buffer is virtually divided into two buffers, one to be sent to TFT,
 * and another one to be processed while the first is sent.
 */
//volatile u16 Global_PixArr[2 * LINES_PER_IMAGE_BUFFER * 128];
// added two more lines to be enough when drawing info.
volatile u16 Global_PixArr[32 * 128];

volatile u16* Global_ImgBufferArr[2] = {
	Global_PixArr, &Global_PixArr[LINES_PER_IMAGE_BUFFER * 128]
};

volatile u16 Global_InfoImg[12 * 5 * 8];

/*
 * Peak to peak value in a single frame.
 * Is calculated as the difference between the largest and smallest values in
 * current frame.
 * "current frame" starts when "tftScrollCounter" equals zero, and ends when it
 * equals "tftScrollCounterMax".
 * Unit is: [TFT screen pixels].
 */
volatile u8 Global_Ch1PeakToPeakValueInCurrentFrame;
volatile u8 Global_Ch1MinValueInCurrentFrame;
volatile u8 Global_Ch1MaxValueInCurrentFrame;

volatile u8 Global_Ch2PeakToPeakValueInCurrentFrame;
volatile u8 Global_Ch2MinValueInCurrentFrame;
volatile u8 Global_Ch2MaxValueInCurrentFrame;

/*	current resolution values	*/
volatile u32 Global_CurrentCh1MicroVoltsPerPix;
volatile u32 Global_CurrentCh2MicroVoltsPerPix;
volatile u64 Global_CurrentNanoSecondsPerPix;

/*	pausing of display	*/
volatile b8 Global_Paused;

volatile u64 Global_LastMeasuredFreq;

volatile OSC_Up_Down_Target_t Global_UpDownTarget;

volatile b8 Global_IsMenuOpen;

volatile u16 Global_SampleBuffer[2 * NUMBER_OF_SAMPLES];

volatile char Global_Str[128];

volatile b8 Global_IsCh1Enabled;
volatile b8 Global_IsCh2Enabled;

/*	voltage in pixels	*/
volatile u8 Global_LastRead1;
volatile u8 Global_LastRead2;

volatile u8 Global_Smaller1;
volatile u8 Global_Larger1;

volatile u8 Global_Smaller2;
volatile u8 Global_Larger2;

volatile s32 Global_Offset1MicroVolts;
volatile s32 Global_Offset2MicroVolts;


/*	current running mode	*/
volatile OSC_RunningMode_t Global_CurrentRunningMode;

