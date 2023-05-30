/*
 * profiler.cpp
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#include "profiler.h"

#include "trig.h" //for cosine for path computation


//check if a move has been completed based off the move params
bool Profiler::move_completed(move_profile_param_t *move_params) {
	//dependent on the motion_tick_ms being updated, but this should be the case
	return move_params->motion_tick_ms > move_params->total_move_time;
}

//given move parameters and a tick increment value, return the distance along the path
float Profiler::distance_along_path(float tick_inc_ms, move_profile_param_t *move_params) {
	//increment the tick counter based off the tick increment value
	move_params->motion_tick_ms += tick_inc_ms;

	//if we're accelerating
	if(move_params->motion_tick_ms < move_params->accel_finish_t) {
		//absolute position required since integration would result in error over time
		move_params->path_distance = 	move_params->A_over_2 * move_params->motion_tick_ms * move_params->motion_tick_ms
										+ move_params->A_over_w2 * (Trig::cos_minus_1((uint16_t)(move_params->omega_scaled * move_params->motion_tick_ms)));
	}
	//if we're decelerating
	else if (move_params->motion_tick_ms > move_params->decel_start_t) {
		//absolute position required since integration would result in error over time
		float negative_t = move_params->total_move_time - move_params->motion_tick_ms; //basically run acceleration backward
		move_params->path_distance = 	move_params->total_move_dist -
										move_params->A_over_2 * negative_t * negative_t
										- move_params->A_over_w2 * (Trig::cos_minus_1((uint16_t)(move_params->omega_scaled * negative_t)));
	}
	//otherwise we're just cruising
	else {
		//can just integrate this; shouldn't have any integration error here
		//i guess other than quantization error which should be small and recoverable
		move_params->path_distance += tick_inc_ms * move_params->cruise_vel;
	}

	//return the accumulated/calculated distance along the path
	return move_params->path_distance;
}

