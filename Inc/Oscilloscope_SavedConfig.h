/*
 * Oscilloscope_SavedConfig.h
 *
 *  Created on: Feb 23, 2023
 *      Author: ali20
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_SAVEDCONFIG_H_
#define INCLUDE_APP_OSCILLOSCOPE_SAVEDCONFIG_H_

/*
 * Updates necessary variables with their saved values on flash..
 */
void OSC_voidGetSavedConfigFromFlash(void);

/*
 * Saves current configuration on flash.
 */
void OSC_voidWriteCurrentConfigOnFlash(void);


#endif /* INCLUDE_APP_OSCILLOSCOPE_SAVEDCONFIG_H_ */
