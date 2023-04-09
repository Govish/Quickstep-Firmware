/*
 * debouncer.h
 *
 *  Created on: Mar 15, 2023
 *      Author: Ishaan
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
	Debouncer(const DIO _pin, const uint32_t _bounce_time_ms, const bool _inverted);

	//quick functions to read (and clear) flag
	uint8_t get_rising_edge_db(bool clear_flag = true);
	uint8_t get_falling_edge_db(bool clear_flag = true);
	uint8_t get_change_db(bool clear_flag = true);
	uint8_t read_db();


	//aggressively optimize here since this will likely be called from ISR
	//call this function at 1kHz
	void __attribute__((optimize("O3"))) sample_and_update();

private:
	//have a member that stores all relevant data into a single 8-bit value
	volatile union {
	    struct {
	        uint8_t rising_db: 1;
	        uint8_t falling_db : 1;
	        uint8_t change_db : 1;
	        uint8_t state_db : 1;
	    } flags;
	    uint8_t reg;
	} status_flags;

	const DIO PIN; //pin that we're reading/debouncing
	const uint32_t BOUNCE_TIME; //time for which to debounce in ms
	const bool INVERTED; //if we want to invert the logic of everything
	const float INPUT_WEIGHT;
	const float PREV_SAMPLE_WEIGHT;

	volatile float filter_val = 0;
	//	volatile bool bouncing; //flag to show that the switch is bouncing
	//	volatile uint32_t bounce_counter; //counter that gets decremented when we bounce
};


#endif /* INC_DEBOUNCER_H_ */
