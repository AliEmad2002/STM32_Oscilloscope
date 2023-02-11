/*
 * Oscilloscope_AutoCalib.h
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_AUTOCALIB_H_
#define INCLUDE_APP_OSCILLOSCOPE_AUTOCALIB_H_

typedef enum{
	OSC_Channel_1,
	OSC_Channel_2
}OSC_Channel_t;

void OSC_voidFindMaxAndMinOfChxInTwoSeconds(
	OSC_Channel_t ch, u16* maxPtr, u16* minPtr);

void OSC_voidFindMaxAndMinOfBothChannelsInTwoSeconds(
	u16* max1Ptr, u16* min1Ptr, u16* max2Ptr, u16* min2Ptr);

void OSC_voidAutoCalibrate(void);



#endif /* INCLUDE_APP_OSCILLOSCOPE_AUTOCALIB_H_ */
