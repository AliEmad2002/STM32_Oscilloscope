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

/*	pixels per div	*/
#define PIXELS_PER_VOLTAGE_DIV								16
#define PIXELS_PER_TIME_DIV									15

/*
 * DMA1 channel used in mem to mem operations
 * (for drawing a single line segment of the display)
 * Notice that: lines 3, 5 are taken by SPI1, 2. Line 1 is taken by ADC1.
 */
#define LINE_SEGMENT_DMA_CHANNEL						DMA_ChannelNumber_2

/*
 * The current target to be effected by the up and down buttons
 */
typedef enum{
	OSC_Up_Down_Target_ChangeCh1VoltageDiv,
	OSC_Up_Down_Target_ChangeCh2VoltageDiv,
	OSC_Up_Down_Target_ChangeTimeDiv,
	OSC_Up_Down_Target_ChangeVoltageCursor1Position,
	OSC_Up_Down_Target_ChangeVoltageCursor2Position,
	OSC_Up_Down_Target_ChangeTimeCursor1Position,
	OSC_Up_Down_Target_ChangeTimeCursor2Position,
	OSC_Up_Down_Target_ChangeCh1Offset,
	OSC_Up_Down_Target_ChangeCh2Offset,
	OSC_Up_Down_Target_ChangeBrightness
}OSC_Up_Down_Target_t;

/*
 * Notice that:
 * ("DASHED_LINE_DRAWN_SEGMENT_LEN" + "DASHED_LINE_BLANK_SEGMENT_LEN") must be
 * dividable by "LINES_PER_IMAGE_BUFFER"
 */
#define LINES_PER_IMAGE_BUFFER								16

#define NUMBER_OF_IMAGE_BUFFERS_PER_FRAME					10

#define NUMBER_OF_SAMPLES	\
	(LINES_PER_IMAGE_BUFFER * NUMBER_OF_IMAGE_BUFFERS_PER_FRAME)

/*	length of drawn segment of a dashed line	*/
#define DASHED_LINE_DRAWN_SEGMENT_LEN						3

/*	length of blank segment of a dashed line	*/
#define DASHED_LINE_BLANK_SEGMENT_LEN						2

/*	sum of the two previous values	*/
#define SUM_OF_DASH_LEN		\
	(DASHED_LINE_DRAWN_SEGMENT_LEN + DASHED_LINE_BLANK_SEGMENT_LEN)

#define ADC_DMA_CHANNEL								DMA_ChannelNumber_1

#define ADC_MIN_CONV_TIME_NANO_SECOND				1000		// 1uS

#define ADC_MAX_SAMPLING_FREQUENCY_MILLI_HZ	\
	(1e12 / ADC_MIN_CONV_TIME_NANO_SECOND)

/*
 * the more samples per period, the more accurate the display is at higher
 * frequency periodic signals. (Thanks to interleaved sampling mode)
 */
#define MIN_NUMBER_OF_REAL_SAMPLES_PER_SIGNAL_PERIOD	10

/*
 * this is the frequency after which the device enters the interleaved sampling
 * mode.
 */
#define INTERLEAVED_SAMPLING_THRESHOLD_FREQUENCY_MILLI_HZ	\
	(                                                       \
		ADC_MAX_SAMPLING_FREQUENCY_MILLI_HZ /               \
		MIN_NUMBER_OF_REAL_SAMPLES_PER_SIGNAL_PERIOD        \
	)

typedef enum{
	OSC_RunningMode_Normal
}OSC_RunningMode_t;

#define OFFSET_POINTER_LEN	7

#define SIGNAL_IMG_X_MIN	8

#define SIGNAL_IMG_X_MAX	(128 - 8 - 1)

#define SIGNAL_LINE_LENGTH (SIGNAL_IMG_X_MAX - SIGNAL_IMG_X_MIN + 1)

#define IMG_BUFFER_SIZE	(SIGNAL_LINE_LENGTH * LINES_PER_IMAGE_BUFFER)

#define RISING_EDGE_WAIT_TIMEOUT_MS			100

#endif /* INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_ */


