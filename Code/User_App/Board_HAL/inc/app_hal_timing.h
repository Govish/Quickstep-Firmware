/*
 * app_hal_timing.h
 *
 *  Created on: Mar 10, 2023
 *      Author: Ishaan
 *
 *  TO ADD ANOTHER TIMER CHANNEL:
 *
 *
 */

#ifndef BOARD_HAL_INC_APP_HAL_TIMING_H_
#define BOARD_HAL_INC_APP_HAL_TIMING_H_

extern "C" {
	#include "stm32f4xx_hal.h"
	#include "stm32f446xx.h" //need this for the IRQn_type
}
#include "app_hal_int_utils.h"

typedef struct {
	callback_function_t init_func; //not a callback function, but has the same signature so just gonna use this typedef
	TIM_HandleTypeDef &htim; //what memory addresses corresond to timer control registers
	IRQn_Type irq_type; //for NVIC--which interrupt type corresponds to the particular timer instance we're mapped to
} timer_config_struct_t;

//have a very explicit enum type to map timer firmware instance to hardware
typedef enum Timer_Channels {
	CHANNEL_0 = 0,
	CHANNEL_1 = 1,
	CHANNEL_2 = 2
} timer_channel_t;

//have a very explicit struct named by tick frequency to
typedef struct {
	uint16_t prescaler;
	uint16_t auto_reload;
} timer_freq_t;

class Timer {
public:
	Timer(timer_channel_t _channel);
	void init(); //call this first before doing anything else really!

	void set_freq(timer_freq_t freq);
	void set_phase(float phase);
	void set_callback_func(callback_function_t cb);
	void set_int_priority(int_priority_t prio);
	void enable_int();
	void disable_int();
	void enable_tim();
	void disable_tim();

	float get_freq();
	float get_tim_fclk();

	static void delay_ms(uint32_t ms);
	static uint32_t get_ms();

	//just have a single ISR function that resets the appropriate timer flags
	//and calls the corresponding callback functions
	//if this is optimized hard enough, the array indexing should hopefully unroll and be
	//as fast as if we had individual ISRs for each timer channel
	//NOTE FOR PORTING: APP WILL NEVER CALL THIS FUNCTION, SO IMPLEMENT HOW YOU'D LIKE
	static void __attribute__((optimize("O3"))) ISR_func(int channel);

	//=================== section here just to declare frequency presets ======================
	static const timer_freq_t FREQ_200kHz;
	static const timer_freq_t FREQ_100kHz;
	static const timer_freq_t FREQ_50kHz;
	static const timer_freq_t FREQ_20kHz;
	static const timer_freq_t FREQ_10kHz;
	static const timer_freq_t FREQ_5kHz;
	static const timer_freq_t FREQ_2kHz;
	static const timer_freq_t FREQ_1kHz;
	static const timer_freq_t FREQ_500Hz;
	static const timer_freq_t FREQ_200Hz;
	static const timer_freq_t FREQ_100Hz;
	static const timer_freq_t FREQ_50Hz;
	static const timer_freq_t FREQ_20Hz;
	static const timer_freq_t FREQ_10Hz;
	static const timer_freq_t FREQ_5Hz;
	static const timer_freq_t FREQ_2Hz;
	static const timer_freq_t FREQ_1Hz;
	static const timer_freq_t FREQ_0p5Hz;
	static const timer_freq_t FREQ_0p2Hz;
	static const timer_freq_t FREQ_0p1Hz;
	//=================== END section declaring frequency presets ===================


private:
	static const timer_config_struct_t timer_chan_configs[];
	static callback_function_t callbacks[];

	int channel; //which channel the particular instance is mapped to
};

#endif /* BOARD_HAL_INC_APP_HAL_TIMING_H_ */
