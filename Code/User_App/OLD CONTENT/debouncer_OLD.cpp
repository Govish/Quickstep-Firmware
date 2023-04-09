/*
 * debouncer.cpp
 *
 *  Created on: Mar 15, 2023
 *      Author: Ishaan
 */

#include "debouncer.h"
#include "math.h"

#define EPSILON 0.0000001 //add this to bounce_time_ms so we don't divide by zero
#define THRESHOLD_UP	(1 - exp(-1)) 	//0.632, about 1 time constant
#define THRESHOLD_DOWN	(exp(-1)) 		//0.368, about 1 time constant

Debouncer::Debouncer(const DIO _pin, const uint32_t _bounce_time_ms, const bool _inverted):
	PIN(_pin), BOUNCE_TIME(_bounce_time_ms), INVERTED(_inverted),
	INPUT_WEIGHT(1-exp(-1.0/(_bounce_time_ms+EPSILON))), PREV_SAMPLE_WEIGHT(1-INPUT_WEIGHT){
	//bounce_counter = 0;
}

uint8_t Debouncer::get_rising_edge_db(bool clear_flag) {
	uint8_t retval = status_flags.flags.rising_db;
	if(clear_flag && retval) //ANDing with `retval` prevents us from unnecessarily doing a register write
		status_flags.flags.rising_db = 0;
	return retval;
}

uint8_t Debouncer::get_falling_edge_db(bool clear_flag) {
	uint8_t retval = status_flags.flags.falling_db;
	if(clear_flag && retval) //ANDing with `retval` prevents us from unnecessarily doing a register write
		status_flags.flags.falling_db = 0;
	return retval;
}

uint8_t Debouncer::get_change_db(bool clear_flag) {
	uint8_t retval = status_flags.flags.change_db;
	if(clear_flag && retval) //ANDing with `retval` prevents us from unnecessarily doing a register write
		status_flags.flags.change_db = 0;
	return retval;
}

uint8_t Debouncer::read_db() {
	return status_flags.flags.state_db;
}


void Debouncer::sample_and_update() {
	//read the input pin, invert if necessary
	float input_val = (float)(INVERTED ? !(PIN.read() > 0) : (PIN.read() > 0));

	//running a first-order IIR filter on the input value
	filter_val = PREV_SAMPLE_WEIGHT * filter_val + INPUT_WEIGHT * input_val;

	//adding hysteresis to our state toggling
	if(filter_val > THRESHOLD_UP) { //if the input is HIGH + hysteresis
		//if we went high
		if(status_flags.flags.state_db == 0) {
			status_flags.flags.rising_db = 1;
			status_flags.flags.change_db = 1;
		}
		status_flags.flags.state_db = 1; //say that we're still high
	}

	else if(filter_val < THRESHOLD_DOWN) { //if the input is LOW + hysteresis
		//if we went low
		if(status_flags.flags.state_db == 1) {
			status_flags.flags.falling_db = 1;
			status_flags.flags.change_db = 1;
		}
		status_flags.flags.state_db = 0; //say that we're still low
	}


}


/*
 * BELOW IS OLD DEBOUNCER CODE--I'm checking if a different method is more reliable
 */
////read the state of the input; invert if necessary
//status_flags.flags.state = INVERTED ? !(PIN.read() > 0) : (PIN.read() > 0);
//
////if we're bouncing, just chill for a little; don't update any internal state vars
//if(bouncing)
//	bounce_counter--;
//
////if we're not bouncing, process the pin reading
//else {
//	//if the pin changed state (with respect do debounced state)
//	if(status_flags.flags.state != status_flags.flags.state_db) {
//
//		//if we transitioned to a high state (from low, implied)
//		if(status_flags.flags.state) {
//			status_flags.flags.change = 1; //set the state change flag
//			status_flags.flags.rising_edge = 1;//set the rising edge flag
//		}
//
//		//otherwise if we transitioned to a low state (from high, implied)
//		else {
//			status_flags.flags.change = 1; //set the state change flag
//			status_flags.flags.falling_edge = 1;//set the falling edge flag
//		}
//		/* NOTE: doing the change flag update in each conditional in case
//		 * the compiler wants to optimize the two lines into a single register write.
//		 * I separated them for code readability reasons
//		 */
//
//		bounce_counter = BOUNCE_TIME; //set the bounce_counter to DEBOUNCE_TIME
//		bouncing = true; //set the flag to show that we're bouncing
//	}
//}
//
////if we're done bouncing
////move this outside of the bounce_counter conditional in case BOUNCE_TIME is 0
////also check if our state changed even after the bouncing
//if((bounce_counter == 0) && bouncing) {
//
//	if(status_flags.flags.state) {
//		status_flags.flags.change_db = 1; //set the state change flag
//		status_flags.flags.rising_db = 1; //set the rising edge flag
//		status_flags.flags.state_db = 1; //set the current state to high
//	}
//
//	else {
//		status_flags.flags.change_db = 1; //set the state change flag
//		status_flags.flags.falling_db = 1; //set the rising edge flag
//		status_flags.flags.state_db = 0; //set the current state to high
//	}
//
//	bouncing = false; //clear the internal flag to show that we're now stable
//	/* NOTE: doing the change flag update in each conditional in case
//	 * the compiler wants to optimize the three lines into a single register write.
//	 * I separated them for code readability reasons
//	 */
//}
