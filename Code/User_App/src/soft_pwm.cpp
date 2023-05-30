/*
 * soft_pwm.cpp
 *
 *  Created on: Mar 22, 2023
 *      Author: Ishaan
 */

#include "soft_pwm.h"

//======================= DEFINING CLASS VARIABLES ====================
bool Soft_PWM::__allow_updates__ = true; //run the ISR normally
const Timer *Soft_PWM::soft_pwm_timer = NULL;
uint32_t Soft_PWM::num_channels_to_update = 0;
Soft_PWM *Soft_PWM::channels_to_update = NULL;

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

//Timer ISR function just runs through an array of all the channels and updates them all
void __attribute__((optimize("O3"))) Soft_PWM::update_all() {
	for(uint32_t i = 0; i < num_channels_to_update; i++)
		channels_to_update[i].update();
}

//========================================= CLASS METHODS =======================================

//have the Soft_PWM class configure its own timer
//need to pass in a callback function externally that updates all the Soft_PWM instances
void Soft_PWM::configure(	const Timer &pwm_timer, int_priority_t priority, timer_freq_t freq,
							float timer_phase, Soft_PWM chans[], const uint32_t num_chans) {
	//save a pointer to the Timer referenced (not used as of now, future-proofing)
	soft_pwm_timer = &pwm_timer;

	//also save the address of the array of all the instantiated Soft_PWM channels
	//and also the number of channels to update
	channels_to_update = chans;
	num_channels_to_update = num_chans;

	//run the appropriate initialization functions
	soft_pwm_timer->init();
	soft_pwm_timer->set_phase(timer_phase);
	soft_pwm_timer->set_freq(freq);
	soft_pwm_timer->set_int_priority(priority);
	soft_pwm_timer->set_callback_func(Soft_PWM::update_all);

	//enable the timer and interrupt
	soft_pwm_timer->enable_int();
	soft_pwm_timer->enable_tim();
}

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
