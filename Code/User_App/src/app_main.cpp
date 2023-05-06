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

#include "debouncer.h"
#include "soft_pwm.h"

const DIO led_red(PinMap::red_led);
const DIO led_yellow(PinMap::yellow_led);
const DIO led_green(PinMap::green_led);
const DIO status_led(PinMap::status_led);

const DIO step_pin(PinMap::mot_step);
const DIO dir_pin(PinMap::mot_dir);
const DIO en_pin(PinMap::mot_en);

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
void stepper_func() {
	if(step_high) {
		step_pin.clear();
		step_high = false;
	}

	else {
		step_pin.set();
		step_high = true;
	}
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

	en_pin.clear();
	dir_pin.set();

	soft_pwm.init();
	soft_pwm.set_phase(0);
	soft_pwm.set_freq(Timer::FREQ_10kHz);
	soft_pwm.set_int_priority(Priorities::MED);
	soft_pwm.set_callback_func(&run_pwm);

	stepper.init();
	stepper.set_phase(0.1);
	stepper.set_freq(Timer::FREQ_20kHz);
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
	Timer::delay_ms(5000);
	dir_pin.clear();
	Timer::delay_ms(5000);
	dir_pin.set();
}


