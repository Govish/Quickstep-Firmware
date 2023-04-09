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

const DIO status_led(PinMap::status_led);
const DIO button_pin(PinMap::user_button);
const DIO led_red(PinMap::led_red);
const DIO led_green(PinMap::led_green);
const DIO led_yellow(PinMap::led_yellow);
const DIO step_pin(PinMap::mot_step);
const DIO dir_pin(PinMap::mot_dir);
const DIO en_pin(PinMap::mot_en);

Debouncer user_button(button_pin, 10, true);

Hard_PWM led_fade(status_led, false);

Soft_PWM red_pwm(led_red, 0, false);
Soft_PWM green_pwm(led_green, 0.1, false);
Soft_PWM yellow_pwm(led_yellow, 0.2, false);

float pwm_val = 0;
uint32_t counter = 0;
uint32_t step_counter = 0;

void update_all_debouncers() {
	user_button.sample_and_update();
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

	Timing::init_soft_pwm_timer(10000.0, 0.2, &run_pwm, Priorities::HIGH);
	Timing::init_db_timer(1000.0, 0, &update_all_debouncers, Priorities::HIGH);
	Timing::init_supervisor_timer(1.0, 0.1, &inc_pwm, Priorities::MED);

	Hard_PWM::configure(1000, Priorities::MED_HIGH);

	Timing::start_timers();

	en_pin.clear();
	dir_pin.set();
}

void app_loop() {
	if(user_button.get_rising_edge_db()) {
		//reset the soft pwm value if the user button is pressed
		pwm_val = 0;

		led_fade.set(pwm_val); //write to the pwm timer
		red_pwm.set(pwm_val);
		yellow_pwm.set(pwm_val);
		green_pwm.set(pwm_val);

		counter++;//just for fun
	}

	if(step_counter < 3200)
		dir_pin.set();
	else if(step_counter < 6399)
		dir_pin.clear();
	else
		step_counter = 0;

	step_pin.set();
	Timing::delay_ms(1);
	step_pin.clear();
	Timing::delay_ms(1);
	step_counter++;
}


