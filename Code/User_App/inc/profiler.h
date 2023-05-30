/*
 * profiler.h
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#ifndef INC_PROFILER_H_
#define INC_PROFILER_H_

//all the necessary elements to describe a move speed profile
typedef struct {
	//step and time constants related to the move
	float total_move_dist;
	float total_move_time;
	float accel_finish_t;
	float decel_start_t;

	//parameters related to the
	float cruise_vel;
	float A_over_2;
	float A_over_w2;
	float omega_scaled;

	//variables updated during the execution of the move
	float motion_tick_ms;
	float path_distance;
} move_profile_param_t;

class Profiler {
public:
	//given move parameters and a tick increment value, return the distance along the path
	static float distance_along_path(float tick_inc_ms, move_profile_param_t *move_params);

	//check if a move has been completed based off the move params
	static bool move_completed(move_profile_param_t *move_params);

private:
	Profiler(); //prevent instantiation of one of these, just use static methods
};



#endif /* INC_PROFILER_H_ */
