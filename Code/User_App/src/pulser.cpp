/*
 * pulser.cpp
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#include "pulser.h"

#include "arm_math.h" //for DSP library

//======================== STATIC MEMBER VARIABLE INITIALIZATION ============================
uint8_t Pulser::pulse_counter = 0;

float Pulser::steps_minus_half[16];
float Pulser::steps_plus_half[16];
float Pulser::step_delta[16];

pulse_queue_t Pulser::pulse_rising_edge[8];
pulse_queue_t Pulser::pulse_falling_edge[8];

//============================== FUNCTION DEFINITIONS ============================

void Pulser::compute_step(float* axis_steps, pulse_param_t *pulse_params) {
	//basically want the actual step position to be rounded to the closest value of the theoretical step position
	//we require certain computations in the event of a step event, so let's just do that up front
	//leverage DSP library for computation speed

	//compute the "lower bound exceed threshold" (each step position + 0.5)
	arm_offset_f32(axis_steps, 0.5, steps_plus_half, pulse_params->num_axes);
	//and also compute the "upper bound exceed threshold" (each step position -0.5)
	arm_offset_f32(axis_steps, -0.5, steps_minus_half, pulse_params->num_axes);
	//compute the difference between previous position and current position
	arm_sub_f32(axis_steps, pulse_params->last_axis_position, step_delta, pulse_params->num_axes);
	//and update last_axis_position now that we've done our calculations
	//but make sure to copy the vector as axis_steps will be updated
	arm_copy_f32(axis_steps, pulse_params->last_axis_position, pulse_params->num_axes);
}

void Pulser::compute_pulse(pulse_param_t *pulse_params) {
	for(uint8_t i = 0; i < pulse_params->num_axes; i++) {
		//set up the axis direction based off the position delta from previous steps
		//can only do this after the step has completed in the case the direction changes between timesteps
		if(step_delta[i] > 0)
			pulse_params->axes_to_move[i].dir_forward();
		else
			pulse_params->axes_to_move[i].dir_reverse();

		//now do the actual check if we need to perform a step
		if(steps_minus_half[i] > pulse_params->axes_to_move[i].get_pos()) {
			//stage a step pulse that is delayed by 1 - ratio_from_end
			float ratio_from_end = (steps_minus_half[i] - pulse_params->axes_to_move[i].get_pos()) / step_delta[i];
			uint8_t pulse_offset = (uint8_t)( ((uint8_t)SMOOTHING_RATIO) * (1 - ratio_from_end) );
			queue_step_pulse(&pulse_params->axes_to_move[i], pulse_offset);
		}
		else if(steps_plus_half[i] < pulse_params->axes_to_move[i].get_pos()) {
			//ratio_from_end will come out positive since step_delta is also negative
			float ratio_from_end = (steps_plus_half[i] - pulse_params->axes_to_move[i].get_pos()) / step_delta[i];
			uint8_t pulse_offset = (uint8_t)( ((uint8_t)SMOOTHING_RATIO) * (1 - ratio_from_end) );
			queue_step_pulse(&pulse_params->axes_to_move[i], pulse_offset);
		}
	}
}

uint8_t Pulser::get_max_pulse_count() {
	//will almost certainly be optimized and inlined by the compiler
	return (uint8_t)(SMOOTHING_RATIO - 1);
}

uint8_t Pulser::service_pulse_timer() {
	//count up/rollover the counter as necessary
	//pulse counter runs for twice as high as to create "epochs" for step pin drive instructions
	pulse_counter = (pulse_counter + 1) & pulse_counter_rollover_mask;

	//drive all the step pins high as necessary
	//effectively mark that they've been driven high
	for(int i = 0; i < pulse_rising_edge[pulse_counter].num_axes_queued; i++) {
		pulse_rising_edge[pulse_counter].axes_to_pulse[i]->step_high();
	}
	pulse_rising_edge[pulse_counter].num_axes_queued = 0;

	//drive all the step pins low as necessary
	//effectively mark that they've been driven low
	for(int i = 0; i < pulse_falling_edge[pulse_counter].num_axes_queued; i++) {
		pulse_falling_edge[pulse_counter].axes_to_pulse[i]->step_low();
	}
	pulse_falling_edge[pulse_counter].num_axes_queued = 0;

	//return the effective count of the pulse counter
	return pulse_counter & pulse_counter_count_mask;
}

//============================ PRIVATE MEMBER FUNCTIONS =============================

void Pulser::queue_step_pulse(Axis *axis_to_step, uint8_t pulse_to_go_high) {
	//will normally queue into epoch 0
	//if we're currently in epoch 0, queue everything into epoch 1
	if(!(pulse_counter & pulse_counter_epoch_mask))
		pulse_to_go_high += pulse_counter_epoch_mask;

	//add one more axis to the queue of axes to step
	pulse_rising_edge[pulse_to_go_high].num_axes_queued++;
	//and add that axis to the array
	pulse_rising_edge[pulse_to_go_high].axes_to_pulse[pulse_rising_edge[pulse_to_go_high].num_axes_queued]
													  = axis_to_step;

	//for the falling edge, do the same, just delayed by half a cycle
	uint8_t pulse_to_go_low = (pulse_to_go_high + ((uint8_t)SMOOTHING_RATIO/2)) & pulse_counter_rollover_mask;
	//add one more axis to the queue of axes to step
	pulse_falling_edge[pulse_to_go_low].num_axes_queued++;
	//and add that axis to the array
	pulse_falling_edge[pulse_to_go_low].axes_to_pulse[pulse_falling_edge[pulse_to_go_low].num_axes_queued]
													  = axis_to_step;
}
