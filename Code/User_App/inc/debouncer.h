/*
 * debouncer.h
 *
 *  Created on: Mar 15, 2023
 *      Author: Ishaan
 *
 *  This thread was very useful:
 *  https://stackoverflow.com/questions/69811934/would-it-be-possible-to-call-a-function-in-every-instance-of-a-class-in-c
 */

#ifndef INC_DEBOUNCER_H_
#define INC_DEBOUNCER_H_

extern "C" {
	#include "stm32f4xx_hal.h"
}
#include "app_hal_dio.h"
#include "stdbool.h"

class Debouncer {
public:
	Debouncer(const DIO &_pin, const uint32_t _bounce_time_ms, const bool _inverted);

	//quick functions to read (and clear) flag
	uint8_t get_rising_edge_db(bool clear_flag = true);
	uint8_t get_falling_edge_db(bool clear_flag = true);
	uint8_t get_change_db(bool clear_flag = true);
	uint8_t read_db();


	//aggressively optimize here since this will likely be called from ISR
	//call this function at 1kHz
	void __attribute__((optimize("O3"))) sample_and_update();

private:
	//storing all these flags as their own bools, rather than a single structure
	//allows for greater read/write atomicity (i.e. can clear rising flag while asserting falling flag)
	volatile bool rising_db;
	volatile bool falling_db;
	volatile bool change_db;
	volatile bool state_db;
	volatile uint32_t bounce_counter; //counter that gets decremented when we bounce

	const DIO PIN; //pin that we're reading/debouncing
	const uint32_t BOUNCE_TIME; //time for which to debounce in ms
	const bool INVERTED; //if input HIGH means it's deasserted
};


#endif /* INC_DEBOUNCER_H_ */
