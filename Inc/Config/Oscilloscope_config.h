/*
 * Oscilloscope_config.h
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
 */

#ifndef OSCILLOSCOPE_CONFIG_H_
#define OSCILLOSCOPE_CONFIG_H_

/*******************************************************************************
 * LCD
 ******************************************************************************/
#define LCD_A0_PIN									GPIO_Pin_B15
#define LCD_RST_PIN									GPIO_Pin_A1

#define LCD_SPI_UNIT_NUMBER							SPI_UnitNumber_1
#define LCD_SPI_AFIO_MAP							0

#define LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER	4
#define LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL		TIM_Channel_4
#define LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP		0
#define LCD_BRIGHTNESS_CONTROL_TIMER_FREQ_HZ		100

#define LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER		1

#define LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER	3
#define LCD_INFO_DRAWING_TRIGGER_FREQUENCY_MILLI_HZ	2000	// once every 0.5 second

#define LCD_STARTUP_SCREEN_DELAY_MS					0

#define LCD_BACKGROUND_COLOR_U8						colorBlackU8Val

#define LCD_BACKGROUND_COLOR_U16					(colorBlack.code565)

#define LCD_MAIN_DRAWING_COLOR_U16					(colorWhite.code565)

//#define LCD_MENU_FONT_SIZE							1

/*******************************************************************************
 * Running indication LED
 ******************************************************************************/
#define LED_INDICATOR_PIN							GPIO_Pin_B11

/*******************************************************************************
 * Control buttons
 ******************************************************************************/
/*
 * due to how EXTI in implemented in HW, buttons MUST be connected on different
 * lines. such that no two or more buttons use pins of the same indexing.
 * Examples:
 * 	- one can't set button1 to pin A1 and button2 to pin B1 or C1.
 * 	- one can set button1 to pin A1 and button2 to any pin except B1 and C1.
 */
#define BUTTON_AUTO_PIN								GPIO_Pin_B13
#define BUTTON_PAUSE_RESUME_PIN						GPIO_Pin_B12
#define BUTTON_MENU_PIN								GPIO_Pin_B10

/*******************************************************************************
 * Analog channels and ADC configuration
 * Note: 'ADC channel' and 'Oscilloscope channel' are clearly stated down-here.
 * so do not confuse what 'channel' is! it will be the word before it.
 ******************************************************************************/
/*	ADC sample time (an element of the enum 'ADC_SampleTime_t')	*/
#define ADC_SAMPLE_TIME								ADC_SampleTime_1_5

/*
 * a structure that defines the ADC channel and its maximum measurable
 * peak-to-peak voltage (that is due to amplifier).
 */
typedef struct{
	ADC_ChannelNumber_t adcChannelNumber;
	u16 maxVPPinMilliVolts;
}OSC_Config_ADC_Channel;

/*******************************************************************************
 * Oscilloscope channel 1 configurations:
 ******************************************************************************/
/*	number of levels for channel 1	*/
#define CHANNEL_1_NUMBER_OF_LEVELS					1 //6

/*
 * array of ADC1 channels connected to these levels.
 * NOTICE: the following array must be ascendingly sorted in terms of
 * maximum measurable peak-to-peak voltage. (as it is useful in the 'auto'
 * funuction)
 */
static const OSC_Config_ADC_Channel
	oscCh1AdcChannels[CHANNEL_1_NUMBER_OF_LEVELS] = {
		//(OSC_Config_ADC_Channel){ADC_ChannelNumber_0, 5},
		//(OSC_Config_ADC_Channel){ADC_ChannelNumber_1, 20},
		//(OSC_Config_ADC_Channel){ADC_ChannelNumber_2, 1000},
		(OSC_Config_ADC_Channel){ADC_ChannelNumber_3, 3300}
		//(OSC_Config_ADC_Channel){ADC_ChannelNumber_4, 5000},
		//(OSC_Config_ADC_Channel){ADC_ChannelNumber_5, 10000},
};

#define ADC_THRESHOLD_MIN							(10)

#define ADC_THRESHOLD_MAX							(4096 - 10)

/*******************************************************************************
 * frequency measurement
 ******************************************************************************/
#define FREQ_MEASURE_TIMER_UNIT_NUMBER				2
#define FREQ_MEASURE_TIMER_UNIT_AFIO_MAP			1
#define FREQ_MEASURE_MIN_FREQ_MILLI_HZ				100



#endif /* OSCILLOSCOPE_CONFIG_H_ */































