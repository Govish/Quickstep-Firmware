/*
 * soft_pwm.cpp
 *
 *  Created on: Mar 22, 2023
 *      Author: Ishaan
 */

#include "soft_pwm.h"

//======================= DEFINING CLASS VARIABLES ====================
bool Soft_PWM::__allow_updates__ = true; //run the ISR normally

//call resynchronize() after instantiating a new object to ensure Soft_PWM groups are phased correctly
Soft_PWM::Soft_PWM(const DIO &_pin, const float _offset, const bool _inverted):
		PIN(_pin), INVERTED(_inverted), OFFSET(_offset)
{}

void Soft_PWM::set(float _pwm_val) {
	//input sanity checks
	if(_pwm_val < 0) return;
	if(_pwm_val > 1) return;

	//operate the PWM normally
	blank_pwm_output = false;

	//load the buffer with the new PWM value,
	duty_cycle = _pwm_val; //store the duty cycle if we resynchronize
	//and apply the calculated counts value to the buffer
	pwm_val_buffer = (uint32_t)(Soft_PWM::resolution * duty_cycle);
}

//go back to normal PWM operation after being forced high or low
void Soft_PWM::operate_normally() {
	//go back to servicing the ISR normally
	//pin will be updated accordingly after next call to `update()`
	blank_pwm_output = false;
}

//force the PWM output asserted (LOW if channel inverted, HIGH if not)
void Soft_PWM::force_asserted() {
	blank_pwm_output = true;
	if(INVERTED)
		PIN.clear();
	else
		PIN.set();
}

//force the PWM output deasserted (HIGH if channel inverted, LOW if not)
void Soft_PWM::force_deasserted() {
	blank_pwm_output = true;
	if(INVERTED)
		PIN.set();
	else
		PIN.clear();
}

//aggressively optimize here since this will likely be called from ISR
//soft PWM frequency is frequency this function is called at divided by soft pwm resolution
void __attribute__((optimize("O3"))) Soft_PWM::update() {
	if(!Soft_PWM::__allow_updates__) return; //if our semaphore is set, don't run any PWM functions

	//====================== manage the counter increment =======================
	//should count from <0> to <resolution - 1>, inclusive
	if(counter >= (resolution - 1)) {
		counter = 0;
		//load the buffered value into the actual COUNT on timer rollover
		pwm_val = pwm_val_buffer;
	}
	else counter++;

	//======================= manage the digital output ==========================
	if(blank_pwm_output) return;

	else if(counter < pwm_val) { //pin should be asserted anytime before the counter value
		if(INVERTED) PIN.clear();
		else PIN.set();
	}

	else { //pin should be deasserted anytime after the counter value
		if(INVERTED) PIN.set();
		else PIN.clear();
	}
}

//========================================= CLASS METHODS =======================================
/*
 * An aside:
 * I'm reeeeeaaallllly not a fan of how this is implemented, as it seems to break abstraction rules
 * I'm implementing it like this in order to keep things as modularized as possible, but the fact that
 *  we need to pass it a list of all soft PWM objects externally rather than maintain a list internally
 *  feels kinda cumbersome
 *
 * This is somewhat necessary, as I can't find a way to maintain an expandable container of pointers
 *  to Soft_PWM instances without getting dangerous with the heap and compromise on ISR performance
 *
 * So as of now, we're just going to use arrays with some additional params to pass
 */

//updates the passed array of channels with the new resolution value
//resynchronizes all Soft_PWM channels passed to this function
//make sure to pass the number of instances, and not just the size_of() array!
void Soft_PWM::set_resolution(uint32_t _resolution, Soft_PWM chans[], const uint32_t num_chans) {
	Soft_PWM::__allow_updates__ = false; //block the ISR

	//update the resolution for all soft PWM channels
	for(uint32_t i = 0; i < num_chans ; i++) {
		chans[i].resolution = _resolution;
	}

	Soft_PWM::resynchronize(chans, num_chans); //then synchronize all the PWM channels

	Soft_PWM::__allow_updates__ = true; //re-allow the ISR
}

//ensures that the phasing and offsets of the passed Soft_PWM channels are correct
//make sure to pass the number of instances, and not just the size_of() array!
void Soft_PWM::resynchronize(Soft_PWM chans[], const uint32_t num_chans) {
	Soft_PWM::__allow_updates__ = false; //block the ISR

	for(uint32_t i = 0; i < num_chans ; i++) {
		chans[i].counter = (uint32_t)(chans[i].resolution * chans[i].OFFSET); //reset the counter with the appropriate offset
		chans[i].pwm_val = (uint32_t)(chans[i].resolution * chans[i].duty_cycle); //set the pwm value according to the stored duty cycle
		chans[i].pwm_val_buffer = chans[i].pwm_val; //set the buffered PWM value up accordingly too
	}

	Soft_PWM::__allow_updates__ = true; //re-allow the ISR
}
