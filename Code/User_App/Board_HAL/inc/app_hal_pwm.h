/*
 * app_hal_pwm.h
 *
 *  Created on: Mar 26, 2023
 *      Author: Ishaan
 *
 *
 *  NOTE: this isn't "Hard" PWM per se, but it's not "Soft" PWM
 *  It exists in a middle ground where it relies on MCU timer peripherals
 *  basically it relies on MCU timer interrupts. However, instead of being bound to the particular PWM pin,
 *  It leverages the compare and overflow interrupts to set and reset whatever Output pin the user wants
 *  As a result, it's has higher performance/lower overhead than pure soft PWM, but performs worse than true hardware PWM
 *
 */

#ifndef BOARD_HAL_INC_APP_HAL_PWM_H_
#define BOARD_HAL_INC_APP_HAL_PWM_H_

#define NUM_PWM_CHANNELS 8 //maximum number of PWM channels we can instantiate

extern "C" {
	#include "stm32f4xx_hal.h"
}
#include "app_hal_int_utils.h"
#include "app_hal_dio.h"

class Hard_PWM {
public:
	//trying to keep the interface as similar to Soft_PWM as possible
	//could theoretically set up a phase offset, but that would require twice as many timer channels to be efficient
	//as such, going to just have all PWM channels reset when counter rolls over
	Hard_PWM(const DIO &_pin, const bool _inverted);
	~Hard_PWM(); //should never be called, but writing this just in case

	//float from 0 to 1 inclusive
	void set(float _pwm_val);
	void operate_normally();
	void force_asserted();
	void force_deasserted();

	static void configure(const float _freq, int_priority_t _priority); //have this apply to all PWM pins

	//aggressively optimize here since this will be called from timer ISRs
	//splitting into two ISRs due to two separate channel groups of four
	//THESE FUNCTIONS WILL NEVER BE CALLED FROM THE APP, SO CAN IMPLEMENT THE ISR
	//HOWEVER YOU WANT BASED OFF OF WHAT'S BEST FOR YOUR HARDWARE
	static void __attribute__((optimize("O3"))) isr_groupA();
	static void __attribute__((optimize("O3"))) isr_groupB();

private:
	//don't allow one of these to be copied, since conflicts could arise writing to the same output pin
	Hard_PWM(Hard_PWM &other){}
	static void enable_chan_interrupt(uint8_t pwm_channel);
	static void disable_chan_interrupt(uint8_t pwm_channel);

	uint8_t channel_mapping; //which pwm channel the particular instance corresponds to

	static bool channel_inverted[8];
	static const DIO *pwm_pins[8];
	static bool blank_channel[8]; //true indicates that the ISR shouldn't affect the pin
	static bool channel_in_use[8]; //true indicates that the channel is active
};

#endif /* BOARD_HAL_INC_APP_HAL_PWM_H_ */
