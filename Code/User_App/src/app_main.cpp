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

const DIO y_step_pin(PinMap::y_mot_step);
const DIO y_dir_pin(PinMap::y_mot_dir);
const DIO y_en_pin(PinMap::y_mot_en);


Hard_PWM led_fade(status_led, false);

Soft_PWM red_pwm(led_red, 0, false);
Soft_PWM green_pwm(led_green, 0.1, false);
Soft_PWM yellow_pwm(led_yellow, 0.2, false);

Timer soft_pwm(Timer_Channels::CHANNEL_0);
Timer stepper(Timer_Channels::CHANNEL_1); //step the motor driven by a timer (takes the spot of the debouncer in these tests
Timer supervisor(Timer_Channels::CHANNEL_2);

float pwm_val = 0;
uint32_t counter = 0;

bool step_high = false;

//toggle the step pin basically
//but do it through the motion_control_core
void stepper_func() {
	Motion_Control_Core::mc_core_interrupt(x_step_pin, y_step_pin, 0.01); //10us interrupt period
}

void run_pwm() {
	red_pwm.update();
	yellow_pwm.update();
	green_pwm.update();
}

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

	x_en_pin.clear();
	x_dir_pin.set();

	y_en_pin.clear();
	y_dir_pin.set();

	soft_pwm.init();
	soft_pwm.set_phase(0);
	soft_pwm.set_freq(Timer::FREQ_10kHz);
	soft_pwm.set_int_priority(Priorities::MED);
	soft_pwm.set_callback_func(&run_pwm);

	stepper.init();
	stepper.set_phase(0.1);
	stepper.set_freq(Timer::FREQ_100kHz);
	stepper.set_int_priority(Priorities::MED_HIGH);
	stepper.set_callback_func(&stepper_func);

	supervisor.init();
	supervisor.set_phase(0.5);
	supervisor.set_freq(Timer::FREQ_1Hz);
	supervisor.set_int_priority(Priorities::LOW);
	supervisor.set_callback_func(&inc_pwm);

	soft_pwm.enable_int();
	soft_pwm.enable_tim();

	stepper.enable_int();
	stepper.enable_tim();

	supervisor.enable_int();
	supervisor.enable_tim();

	Hard_PWM::configure(1000, Priorities::MED_HIGH);
}

void app_loop() {
}


