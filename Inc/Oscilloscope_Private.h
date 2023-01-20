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


/*	enum that describes the different states of line drawing	*/
typedef enum{
	OSC_LineDrawingState_1,	// means that the last started operation is:
							// scroll, read ADC, set new drawing boundaries
							// and draw black segment from 0 to
							// 'OSC_smallest' - 1.

	OSC_LineDrawingState_2,	// means that the last started operation is:
							// Drawing red segment from 'OSC_smallest' to
							// 'OSC_largeest'.

	OSC_LineDrawingState_3,	// means that the last started operation is:
							// Drawing red segment from 'OSC_largeest' + 1 to
							// 127.
}OSC_LineDrawingState_t;


typedef enum{
	OSC_RunningState_NormalMode,
	OSC_RunningState_MathMode
}OSC_RunningState_t;

void OSC_voidStartSignalDrawing(void);

void OSC_voidStartInfoDrawing(void);

#endif /* INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_ */
