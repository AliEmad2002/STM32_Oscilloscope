/*
 * Oscilloscope_IR.h
 *
 *  Created on: Feb 16, 2023
 *      Author: ali20
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_IR_H_
#define INCLUDE_APP_OSCILLOSCOPE_IR_H_

typedef struct{
	/*	32-bit raw value of the button	*/
	u32 rawVal;

	/*
	 * array of all the characters this button can write.
	 * i.e.:
	 * 	- for button of number '1', str = "1".
	 * 	- for button of number '2', str = "2ABC".
	 */
	char* str;
}OSC_IR_Button;



#endif /* INCLUDE_APP_OSCILLOSCOPE_IR_H_ */
