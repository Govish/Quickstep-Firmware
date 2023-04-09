/*
 * debouncer.cpp
 *
 *  Created on: Mar 15, 2023
 *      Author: Ishaan
 */

#include "debouncer.h"

Debouncer::Debouncer(const DIO &_pin, const uint32_t _bounce_time_ms, const bool _inverted):
	PIN(_pin), BOUNCE_TIME(_bounce_time_ms), INVERTED(_inverted)
{
	bounce_counter = 0;
}

/*
 * NOTE: FOR ALL THESE GETTER METHODS, THERE IS TECHNICALLY POSSIBILITIES OF RACE CONDITIONS
 * specifically, when we are clearing the flags;
 * Severity is very low, as race condition will only exist when flag is asserted and a corresponding event happens
 * in between reading the flag and clearing the flag
 *
 * HOWEVER, this can be considered acceptable behavior, as the function returns the status of the flags
 * as of the exit cycle of the function (i.e. even if we see a rising edge between reading the flag and clearing it,
 * we return that we saw a rising edge as the function exits)
 */
uint8_t Debouncer::get_rising_edge_db(bool clear_flag) {
	bool retval = rising_db;
	if(clear_flag && retval) //ANDing with `retval` prevents us from unnecessarily doing a register write
		rising_db = false;
	return retval;
}

uint8_t Debouncer::get_falling_edge_db(bool clear_flag) {
	bool retval = falling_db;
	if(clear_flag && retval) //ANDing with `retval` prevents us from unnecessarily doing a register write
		falling_db = false;
	return retval;
}

uint8_t Debouncer::get_change_db(bool clear_flag) {
	bool retval = change_db;
	if(clear_flag && retval) //ANDing with `retval` prevents us from unnecessarily doing a register write
		change_db = false;
	return retval;
}

uint8_t Debouncer::read_db() {
	return state_db;
}

//call this function from ISR context
void Debouncer::sample_and_update() {
	//read the input pin, invert if necessary
	bool input = INVERTED ? !(PIN.read() > 0) : (PIN.read() > 0);

	//if we're bouncing, just chill for a little; don't update any internal state vars
	if(bounce_counter > 0)
		bounce_counter--;

	//if we aren't waiting for a debounce read
	//check to see if the state changed
	//then start a debounce cycle
	else if(input != state_db)
		bounce_counter = BOUNCE_TIME;

	//check the bounce counter now
	//checking it outside of the first 'bounce_counter > 0' conditional in case
	//BOUNCE_TIME is set to 0 (we'd want to check it in the same cycle
	//also check for state change so we don't constantly run this section when just sampling normally
	if((bounce_counter == 0) && (input != state_db)) {
		if(input) //input went high, rising edge
			rising_db = true;
		else  //input went low, falling edge
			falling_db = true;

		change_db = true; //record a debounce state change
		state_db = input;
	}
}
