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
 * DMA1 channels used in mem to mem operations
 * (for drawing a single line of the display
 */
#define FIRST_LINE_SEGMENT_DMA_CHANNEL						DMA_ChannelNumber_1
#define SECOND_LINE_SEGMENT_DMA_CHANNEL						DMA_ChannelNumber_2
#define THIRD_LINE_SEGMENT_DMA_CHANNEL						DMA_ChannelNumber_4

/*
 * The current target to be effected by the up and down buttons
 */
typedef enum{
	OSC_Up_Down_Target_ChangeVoltageDiv,
	OSC_Up_Down_Target_ChangeTimeDiv,
	OSC_Up_Down_Target_ChangeVoltageCursor1Position,
	OSC_Up_Down_Target_ChangeVoltageCursor2Position,
	OSC_Up_Down_Target_ChangeTimeCursor1Position,
	OSC_Up_Down_Target_ChangeTimeCursor2Position,
	OSC_Up_Down_Target_ChangeBrightness
}OSC_Up_Down_Target_t;

/*
 * Notice that:
 * ("DASHED_LINE_DRAWN_SEGMENT_LEN" + "DASHED_LINE_BLANK_SEGMENT_LEN") must be
 * dividable by "LINES_PER_IMAGE_BUFFER"
 */

#define LINES_PER_IMAGE_BUFFER								15

/*	By test, this number better be dividable by 3	*/
#define NUMBER_OF_IMAGE_BUFFERS_PER_FRAME					9

#define NUMBER_OF_SAMPLES	\
	(LINES_PER_IMAGE_BUFFER * NUMBER_OF_IMAGE_BUFFERS_PER_FRAME)

/*	length of drawn segment of a dashed line	*/
#define DASHED_LINE_DRAWN_SEGMENT_LEN						3

/*	length of blank segment of a dashed line	*/
#define DASHED_LINE_BLANK_SEGMENT_LEN						2


#endif /* INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_ */


