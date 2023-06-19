/*
 * app.main.cpp
 *
 *  Created on: Mar 10, 2023
 *      Author: Ishaan
 */

#include "app_main.h"
#include "app_hal_timing.h"
#include "app_hal_dio.h"
#include "app_pin_mapping.h"
#include "app_hal_int_utils.h"
#include "app_hal_pwm.h"

#include "motion_control_core.h"

#include "debouncer.h"
#include "soft_pwm.h"

const DIO led_red(PinMap::red_led);
const DIO led_yellow(PinMap::yellow_led);
const DIO led_green(PinMap::green_led);
const DIO status_led(PinMap::status_led);

const DIO x_step_pin(PinMap::x_mot_step);
const DIO x_dir_pin(PinMap::x_mot_dir);
const DIO x_en_pin(PinMap::x_mot_en);
Axis x_axis(x_step_pin, x_dir_pin, x_en_pin, false, 'X');

const DIO y_step_pin(PinMap::y_mot_step);
const DIO y_dir_pin(PinMap::y_mot_dir);
const DIO y_en_pin(PinMap::y_mot_en);
Axis y_axis(y_step_pin, y_dir_pin, y_en_pin, false, 'Y');

Hard_PWM led_fade(status_led, false);

Soft_PWM red_pwm(led_red, 0, false);
Soft_PWM green_pwm(led_green, 0.1, false);
Soft_PWM yellow_pwm(led_yellow, 0.2, false);
Soft_PWM soft_pwm_group[] = {red_pwm, green_pwm, yellow_pwm};
uint8_t soft_pwm_group_size = sizeof(soft_pwm_group) / sizeof(soft_pwm_group[0]);

const Timer soft_pwm_tim(Timer_Channels::CHANNEL_0);
const Timer mc_core_tim(Timer_Channels::CHANNEL_1); //step the motor driven by a timer (takes the spot of the debouncer in these tests
const Timer supervisor(Timer_Channels::CHANNEL_2);

const Virtual_Int mc_core_calc_vint(Virtual_Int_Channels::VINT_CHANNEL_0);
const Virtual_Int mc_core_stage_step_vint(Virtual_Int_Channels::VINT_CHANNEL_1);

float pwm_val = 0;
uint32_t counter = 0;

void inc_pwm() {
	pwm_val+= 0.25;
	if(pwm_val > 1) pwm_val = 0;

	led_fade.set(pwm_val);
	red_pwm.set(pwm_val);
	yellow_pwm.set(pwm_val);
	green_pwm.set(pwm_val);
}

void app_init() {
	DIO::init();

	supervisor.init();
	supervisor.set_phase(0.5);
	supervisor.set_freq(Timer::FREQ_1Hz);
	supervisor.set_int_priority(Priorities::LOW);
	supervisor.set_callback_func(&inc_pwm);

	supervisor.enable_int();
	supervisor.enable_tim();

	x_axis.init();
	y_axis.init();

	Motion_Control_Core::configure(mc_core_tim, mc_core_calc_vint, mc_core_stage_step_vint);
	Motion_Control_Core::enable_mc_core();

	Soft_PWM::configure(soft_pwm_tim, soft_pwm_group, soft_pwm_group_size);
	Soft_PWM::resynchronize(soft_pwm_group, soft_pwm_group_size);

	Hard_PWM::configure(1000);
}

void app_loop() {
}


