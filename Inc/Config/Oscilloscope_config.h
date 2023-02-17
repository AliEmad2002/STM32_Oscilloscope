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
#define LCD_A0_PIN									GPIO_Pin_A11
#define LCD_RST_PIN									GPIO_Pin_A10

#define LCD_SPI_UNIT_NUMBER							SPI_UnitNumber_2
#define LCD_SPI_AFIO_MAP							0

#define LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER	4
#define LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL		TIM_Channel_4
#define LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP		0

#define LCD_STARTUP_SCREEN_DELAY_MS					200

#define INFO_DRAWING_PERIODIC_TIME_MS				500ul

#define LCD_BACKGROUND_COLOR_U8						colorBlackU8Val

#define LCD_BACKGROUND_COLOR_U16					(colorBlack.code565)

#define LCD_MAIN_DRAWING_COLOR_U16					(colorWhite.code565)

#define LCD_SECONDARY_DRAWING_COLOR_U16				(colorGreen.code565)

#define LCD_CURSOR1_DRAWING_COLOR_U16				(colorYellow.code565)

#define LCD_CURSOR2_DRAWING_COLOR_U16				(colorViolet.code565)

#define LCD_AXIS_DRAWING_COLOR_U16					(colorRed.code565)

#define LCD_DIV_DRAWING_COLOR_U16					(colorBlue.code565)

#define LCD_OFFSET_POINTER1_DRAWING_COLOR_U16		LCD_MAIN_DRAWING_COLOR_U16

#define LCD_OFFSET_POINTER2_DRAWING_COLOR_U16		\
	LCD_SECONDARY_DRAWING_COLOR_U16

#define LCD_FPS										20

/*	check list element prefix	*/
#define CHECKED_SYMBOL_ASCII_CODE		253
#define UNCHECKED_SYMBOL_ASCII_CODE		' '

/*******************************************************************************
 * Running indication LED
 ******************************************************************************/
#define LED_INDICATOR_PIN							GPIO_Pin_B12

/*******************************************************************************
 * Control HW
 ******************************************************************************/
/*
 * due to how EXTI in implemented in HW, buttons MUST be connected on different
 * lines. such that no two or more buttons use pins of the same indexing.
 * Examples:
 * 	- one can't set button1 to pin A1 and button2 to pin B1 or C1.
 * 	- one can set button1 to pin A1 and button2 to any pin except B1 and C1.
 * Note: Do not use the line of the frequency measurement pin, as it is used
 * by application!!
 */
#define BUTTON_AUTO_ENTER_MENU_PIN					GPIO_Pin_B11
#define BUTTON_PAUSE_RESUME_PIN						GPIO_Pin_A7

#define BUTTON_DEBOUNCING_TIME_MS					250ul
#define ROTARY_DEBOUNCING_TIME_MS					25ul

#define IR_EXTI_PIN									GPIO_Pin_B4

#define IR_MAX_SMALL_TIME_MS					500
#define IR_MIN_TIME_BETWEEN_PRESSES_MS			250

#define USED_REMOTE						1

#if USED_REMOTE == 0
#define IR_0_BUTTON_VALUE				4278190594
#define IR_1_BUTTON_VALUE				4261478914
#define IR_2_BUTTON_VALUE				4244767234
#define IR_3_BUTTON_VALUE				4228055554
#define IR_4_BUTTON_VALUE				4211343874
#define IR_5_BUTTON_VALUE				4194632194
#define IR_6_BUTTON_VALUE				4177920514
#define IR_7_BUTTON_VALUE				4161208834
#define IR_8_BUTTON_VALUE				4144497154
#define IR_9_BUTTON_VALUE				4127785474
#define IR_BACK_BUTTON_VALUE			3793551874

#elif USED_REMOTE == 1
#define IR_0_BUTTON_VALUE				3994093319
#define IR_1_BUTTON_VALUE				4211345159
#define IR_2_BUTTON_VALUE				4194633479
#define IR_3_BUTTON_VALUE				4177921799
#define IR_4_BUTTON_VALUE				4144498439
#define IR_5_BUTTON_VALUE				4127786759
#define IR_6_BUTTON_VALUE				4111075079
#define IR_7_BUTTON_VALUE				4077651719
#define IR_8_BUTTON_VALUE				4060940039
#define IR_9_BUTTON_VALUE				4044228359
#define IR_PLUS_BUTTON_VALUE			4161210119
#define IR_MINUS_BUTTON_VALUE			4094363399
#define IR_MUL_BUTTON_VALUE				3977381639
#define IR_DIV_BUTTON_VALUE				4010804999
#define IR_X_BUTTON_VALUE				4027516679
#define IR_Y_BUTTON_VALUE				2490042119
#define IR_BACK_BUTTON_VALUE			3960669959
#define IR_ENTER_BUTTON_VALUE			2540177159

#endif

/*******************************************************************************
 * Safe thread:
 * Some routines are very critical and won't work right if they were interrupted
 * while running in main thread. To solve this, put that routine in an EXTI
 * callback and trigger it by SW in main thread.
 ******************************************************************************/
#define SAFE_THREAD_EXTI_LINE						1

/*******************************************************************************
 * Analog channels and ADC configuration
 * Note: 'ADC channel' and 'Oscilloscope channel' are clearly stated down-here.
 * so do not confuse what 'channel' is! it will be the word before it.
 ******************************************************************************/
/*	ADC sample time (an element of the enum 'ADC_SampleTime_t')	*/
#define ADC_SAMPLE_TIME								ADC_SampleTime_1_5

/*	(milli-Hz)	*/
#define INITIAL_SAMPLING_FREQUENCY					1000000

#define INPUT_CH1_ADC_CHANNEL						ADC_ChannelNumber_3

#define INPUT_CH2_ADC_CHANNEL						ADC_ChannelNumber_5

/*******************************************************************************
 * Div. configurations:
 ******************************************************************************/
/*	These two arrays must be written here in an ascending order	*/
#define NUMBER_OF_VOLT_DIVS							19
static const u32 OSC_mVoltsPerDivArr[NUMBER_OF_VOLT_DIVS] = {
	1, 5, 20, 100, 500,
	600, 700, 800, 900,
	1000, 1200, 1400, 1600, 1800,
	2000, 2400, 2800, 3200, 3600
};

#define NUMBER_OF_TIME_DIVS							50
static const u64 OSC_nSecondsPerDivArr[NUMBER_OF_TIME_DIVS] = {
	1e3, 2e3, 3e3, 4e3, 5e3, 6e3, 7e3, 8e3, 9e3,
	1e4, 2e4, 3e4, 4e4, 5e4, 6e4, 7e4, 8e4, 9e4,
	1e5, 2e5, 3e5, 4e5, 5e5, 6e5, 7e5, 8e5, 9e5,
	1e6, 2e6, 3e6, 4e6, 5e6, 6e6, 7e6, 8e6, 9e6,
	1e7, 2e7, 3e7, 4e7, 5e7, 6e7, 7e7, 8e7, 9e7,
	1e8, 2e8, 3e8, 4e8, 5e8
};

/*******************************************************************************
 * frequency measurement
 ******************************************************************************/
#define FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER				2
#define FREQ_MEASURE_CH1_TIMER_UNIT_AFIO_MAP			0
#define FREQ_MEASURE_CH1_MIN_FREQ_MILLI_HZ				(1 * 100 * 1000)

#define FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER				1
#define FREQ_MEASURE_CH2_TIMER_UNIT_AFIO_MAP			0
#define FREQ_MEASURE_CH2_MIN_FREQ_MILLI_HZ				(1 * 40 * 1000)

/*
 * Used in frequency measurement in auto calibrate function. This is the timeout
 * of waiting for signal rising edge to get non-old/parasitic frequency measure.
 */
#define FREQ_MEASURE_TIMEOUT_MS						1000

#endif /* OSCILLOSCOPE_CONFIG_H_ */































