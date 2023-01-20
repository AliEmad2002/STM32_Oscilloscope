/*
 * Oscilloscope_Menu.h
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_MENU_H_
#define INCLUDE_APP_OSCILLOSCOPE_MENU_H_

extern void OSC_voidEnterMathMode(void);

extern void OSC_voidEnterNormalMode(void);

#define NUMBER_OF_OSC_MENU_ELEMENTS			2

static Menu_t mainMenu = {
	.currentSelected = 0,
	.numberOfElements = NUMBER_OF_OSC_MENU_ELEMENTS,
	.elementArr[0] = {" Math (X-Y) mode",	OSC_voidEnterMathMode},
	.elementArr[1] = {" Normal (Y-t) mode", 	OSC_voidEnterNormalMode}
};

void OSC_voidRefreshMenu(void);

/*	opens menu (shows it on screen and watches the up / down buttons 	*/
void OSC_voidOpenMenu(void);

#endif /* INCLUDE_APP_OSCILLOSCOPE_MENU_H_ */
