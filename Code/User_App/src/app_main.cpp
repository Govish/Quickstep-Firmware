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

Debouncer user_button(button_pin, 10, true);
//Soft_PWM led_fade(status_led, 0, false);
Hard_PWM led_fade(status_led, false);

<<<<<<< Updated upstream
=======
Soft_PWM red_pwm(led_red, 0, false);
Soft_PWM green_pwm(led_green, 0.1, false);
Soft_PWM yellow_pwm(led_yellow, 0.2, false);

Timer soft_pwm(Timer_Channels::CHANNEL_0);
Timer debouncer(Timer_Channels::CHANNEL_1);
Timer supervisor(Timer_Channels::CHANNEL_2);

>>>>>>> Stashed changes
float pwm_val = 0;
uint32_t counter = 0;

void update_all_debouncers() {
	user_button.sample_and_update();
}

//void run_pwm() {
//	led_fade.update();
//}

void inc_pwm() {
	pwm_val+= 0.25;
	if(pwm_val > 1) pwm_val = 0;
	led_fade.set(pwm_val);
}

void app_init() {
	DIO::init();

<<<<<<< Updated upstream
	//Timing::init_soft_pwm_timer(10000.0, 0.2, &run_pwm, Priorities::HIGH);
	Timing::init_db_timer(1000.0, 0, &update_all_debouncers, Priorities::HIGH);
	Timing::init_supervisor_timer(1.0, 0.1, &inc_pwm, Priorities::MED);

	Hard_PWM::configure(1000, Priorities::MED_HIGH);
	Timing::start_timers();
=======
	soft_pwm.init();
	soft_pwm.set_phase(0);
	soft_pwm.set_freq(Timer::FREQ_10kHz);
	soft_pwm.set_int_priority(Priorities::MED_HIGH);
	soft_pwm.set_callback_func(&run_pwm);

	debouncer.init();
	debouncer.set_phase(0.1);
	debouncer.set_freq(Timer::FREQ_1kHz);
	debouncer.set_int_priority(Priorities::MED);
	debouncer.set_callback_func(&update_all_debouncers);

	supervisor.init();
	supervisor.set_phase(0.5);
	supervisor.set_freq(Timer::FREQ_1Hz);
	supervisor.set_int_priority(Priorities::LOW);
	supervisor.set_callback_func(&inc_pwm);

	soft_pwm.enable_int();
	soft_pwm.enable_tim();

	debouncer.enable_int();
	debouncer.enable_tim();

	supervisor.enable_int();
	supervisor.enable_tim();

	Hard_PWM::configure(1000, Priorities::MED_HIGH);
//
//	Timing::start_timers();
//
//	en_pin.clear();
//	dir_pin.set();
>>>>>>> Stashed changes
}

void app_loop() {
	if(user_button.get_rising_edge_db()) {
		//reset the soft pwm value if the user button is pressed
		pwm_val = 0;
		led_fade.set(pwm_val); //write to the pwm timer

		counter++;//just for fun
	}
<<<<<<< Updated upstream
=======

//	if(step_counter < 3200)
//		dir_pin.set();
//	else if(step_counter < 6399)
//		dir_pin.clear();
//	else
//		step_counter = 0;
//
//	step_pin.set();
//	Timing::delay_ms(1);
//	step_pin.clear();
//	Timing::delay_ms(1);
//	step_counter++;
>>>>>>> Stashed changes
}


