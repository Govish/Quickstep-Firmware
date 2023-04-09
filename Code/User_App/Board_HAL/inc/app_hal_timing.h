/*
 * app_hal_timing.h
 *
 *  Created on: Mar 10, 2023
 *      Author: Ishaan
 *
 */

#ifndef BOARD_HAL_INC_APP_HAL_TIMING_H_
#define BOARD_HAL_INC_APP_HAL_TIMING_H_

extern "C" {
	#include "stm32f4xx_hal.h"
}
#include "app_hal_int_utils.h"


class Timing {
public:
	static void init_db_timer(const float freq, const float offset, callback_function_t _db_cb, int_priority_t prio);
	static void init_soft_pwm_timer(const float freq, const float offset, callback_function_t _soft_pwm_cb, int_priority_t prio);
	static void init_supervisor_timer(const float freq, const float offset, callback_function_t _supervisor_cb, int_priority_t prio);

	static void start_timers();
	static void delay_ms(uint32_t ms);
	static uint32_t get_ms();

	//heavily optimize these ISRs
#pragma GCC push_options
#pragma GCC optimize ("O3")
	static void db_isr();
	static void soft_pwm_isr();
	static void sup_isr();
#pragma GCC pop_options

private:
	//pointers to callback functions for when these interrupts fire
	static callback_function_t db_cb;
	static callback_function_t soft_pwm_cb;
	static callback_function_t supervisor_cb;

	//so we can't instantiate one of these
	Timing() {}
};

#endif /* BOARD_HAL_INC_APP_HAL_TIMING_H_ */
