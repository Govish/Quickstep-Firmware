/*
 * soft_pwm.h
 *
 *  Created on: Mar 22, 2023
 *      Author: Ishaan
 *
 *
 *  This thread was very useful:
 *  https://stackoverflow.com/questions/69811934/would-it-be-possible-to-call-a-function-in-every-instance-of-a-class-in-c
 */

#ifndef INC_SOFT_PWM_H_
#define INC_SOFT_PWM_H_

extern "C" {
	#include "stm32f4xx_hal.h"
}
#include "stdbool.h"
#include "app_hal_dio.h"

class Soft_PWM {
public:
	static void set_resolution(uint32_t _resolution, Soft_PWM chans[], const uint32_t num_chans); // applies to all soft PWM objects
	static void resynchronize(Soft_PWM chans[], const uint32_t num_chans); //restart all counters and pwm objects

	Soft_PWM(const DIO &_pin, const float _offset, const bool _inverted); //normal constructor

	//float from 0 to 1 inclusive
	void set(float _pwm_val);
	void operate_normally();
	void force_asserted();
	void force_deasserted();

	//aggressively optimize here since this will likely be called from ISR
	//soft PWM frequency is frequency this function is called at divided by soft pwm resolution
	void __attribute__((optimize("O3"))) update();

private:
	static bool __allow_updates__; //a little semaphore type thing to ensure atomic writes across all of our PWM channels

	const DIO PIN;
	const bool INVERTED;
	const float OFFSET;

	//place to keep the true desired duty cycle
	//useful for resynchronizing the timers after adjusting timer resolution
	float duty_cycle = 0;
	uint32_t pwm_val_buffer = 0; //buffered PWM value loaded after timer overflow
	uint32_t pwm_val = 0; //pwm value for the current counter cycle
	uint32_t counter = 0; //counter that gets incremented every interrupt call
	uint32_t resolution = 100; //how many discrete pwm values we can take
	bool blank_pwm_output; //flag that prevents the output from changing

};

#endif /* INC_SOFT_PWM_H_ */
