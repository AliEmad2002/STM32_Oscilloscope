/*
 * Oscilloscope_Menu_Init.h
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_MENU_OSCILLOSCOPE_MENU_INIT_H_
#define INCLUDE_APP_MENU_OSCILLOSCOPE_MENU_INIT_H_


Menu_t* OSC_PtrInitChangeVoltageDivMenu(void);

Menu_t* OSC_PtrInitChangeDivMenu(void);

Menu_t* OSC_PtrInitOffsetMenu(void);

Menu_t* OSC_PtrInitCusrorSelectMenu(void);

Check_List_t* OSC_PtrInitCursorEnableDisableCkeckList(char* returnStr);

Menu_t* OSC_PtrInitCursorMenu(char* returnStr);

Check_List_t* OSC_PtrInitChannelEnableDisableCkeckList(char* returnStr);

Check_List_t* OSC_PtrInitInfoEnableDisableCkeckList(char* returnStr);

Menu_t* OSC_PtrInitMathExpressionMenu(void);

Menu_t* OSC_PtrInitChangeModeMenu(void);

void OSC_voidInitMainMenu(void);

#endif /* INCLUDE_APP_MENU_OSCILLOSCOPE_MENU_INIT_H_ */
