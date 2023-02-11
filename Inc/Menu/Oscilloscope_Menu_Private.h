/*
 * Oscilloscope_Menu.h
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_MENU_H_
#define INCLUDE_APP_OSCILLOSCOPE_MENU_H_

/*	sets image boundaries after exiting menu	*/
extern void OSC_voidSetDisplayBoundariesForSignalArea(void);

/*	change selection functions	*/
void OSC_voidSelectChangeCh1VoltageDivAsUpDownTraget(void);
void OSC_voidSelectChangeCh2VoltageDivAsUpDownTraget(void);
void OSC_voidSelectChangeTimeDivAsUpDownTraget(void);
void OSC_voidSelectChangeV1Position(void);
void OSC_voidSelectChangeV2Position(void);
void OSC_voidSelectChangeT1Position(void);
void OSC_voidSelectChangeT2Position(void);
void OSC_voidSelectChangeCh1Offset(void);
void OSC_voidSelectChangeCh2Offset(void);
void OSC_voidSelectChangeBrightness(void);

void OSC_voidUpdateMenuOnDisplay(Menu_t* menu);

/*	opens menu (shows it on screen and watches the up / down buttons 	*/
void OSC_voidOpenMenu(Menu_t* menu);

void OSC_voidUpdateCheckListOnDisplay(Check_List_t* checkListPtr);

void OSC_voidOpenCheckList(Check_List_t* checkListPtr);

#endif /* INCLUDE_APP_OSCILLOSCOPE_MENU_H_ */
