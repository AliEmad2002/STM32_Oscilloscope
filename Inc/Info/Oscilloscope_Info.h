/*
 * Oscilloscope_Info.h
 *
 *  Created on: Feb 8, 2023
 *      Author: Ali Emad Ali
 */

#ifndef APP_INFO_OSCILLOSCOPE_INFO_H_
#define APP_INFO_OSCILLOSCOPE_INFO_H_

#define MAX_NAME_LEN	6
#define MAX_UNIT_LEN	3

#define NUMBER_OF_INFO	19

typedef struct{
	s64 (*getValInNanoCallback)(void);
	char name[MAX_NAME_LEN];
	char unit[MAX_UNIT_LEN];
	b8 enabled;
}OSC_Info_t;

/*	Inits info array	*/
void OSC_voidInitInfo(void);

/*
 * Draws enabled info on their selected places on the display.
 * (Draws first 4 info's only, as there's only 4 places available)
 */
void OSC_voidDrawInfo(void);

//void OSC_voidEnableInfo()





#endif /* APP_INFO_OSCILLOSCOPE_INFO_H_ */
