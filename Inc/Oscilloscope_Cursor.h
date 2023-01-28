/*
 * Oscilloscope_Cursor.h
 *
 *  Created on: Jan 28, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_CURSOR_H_
#define INCLUDE_APP_OSCILLOSCOPE_CURSOR_H_


typedef struct{
	u8 pos;
	b8 isEnabled;
}OSC_Cursor_t;

/*	enable / add cursor	*/
void OSC_voidEnableCursorV1(void);
void OSC_voidEnableCursorV2(void);
void OSC_voidEnableCursorT1(void);
void OSC_voidEnableCursorT2(void);

/*	disable / remove cursor	*/
void OSC_voidDisableCursorV1(void);
void OSC_voidDisableCursorV2(void);
void OSC_voidDisableCursorT1(void);
void OSC_voidDisableCursorT2(void);

/*	increment cursor position (if enabled)	*/
void OSC_voidIncrementCursorV1(void);
void OSC_voidIncrementCursorV2(void);
void OSC_voidIncrementCursorT1(void);
void OSC_voidIncrementCursorT2(void);

/*	decrement cursor position (if enabled)	*/
void OSC_voidDecrementCursorV1(void);
void OSC_voidDecrementCursorV2(void);
void OSC_voidDecrementCursorT1(void);
void OSC_voidDecrementCursorT2(void);


#endif /* INCLUDE_APP_OSCILLOSCOPE_CURSOR_H_ */
