/*
 * Oscilloscope_Private.h
 *
 *  Created on: Jan 18, 2023
 *      Author: Ali Emad Ali
 *
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_
#define INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_


/*	Defines based on configuration file	*/
#define ADC_1_CHANNEL		(ANALOG_INPUT_1_PIN % 16)
#define ADC_2_CHANNEL		(ANALOG_INPUT_2_PIN % 16)

/*	constant values based on experimental tests	*/
static const u64 lineDrawingRatemHzMax = 8000000;


typedef enum{
	OSC_RunningState_Preparing1stQuarter,
	OSC_RunningState_Preparing2ndQuarter,
	OSC_RunningState_Preparing3rdQuarter,
	OSC_RunningState_Preparing4thQuarter, // i.e.: info quarter
}OSC_RunningState_t;

/*
 * This function enables the periodic event (every 10ms) of sending the proper
 * 1/4 of the display.
 */
void OSC_voidStartSignalDrawing(void);

/*	This definition, along with the
 * "Global_NumberOfsentQuartersSinceLastInfoUpdate" defined in private.c,
 * determine how often and when info image is updated
 */
#define NUMBER_OF_SENT_QUARTERS_REQUIERED_FOR_INFO_UPDATE	100

/*	pixels per div	*/
#define PIXELS_PER_VOLTAGE_DIV								16

#define PIXELS_PER_TIME_DIV									15

/*
 * DMA1 channels used in mem to mem operations
 * (for drawing a single line of the display
 */
#define FIRST_LINE_SEGMENT_DMA_CHANNEL						DMA_ChannelNumber_1
#define SECOND_LINE_SEGMENT_DMA_CHANNEL						DMA_ChannelNumber_2
#define THIRD_LINE_SEGMENT_DMA_CHANNEL						DMA_ChannelNumber_4

#endif /* INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_ */

