/*
 * pulser.h
 *
 *	Pulser ISR runs at 2^n frequency over the main step timer frequency
 *	Steps will be fired off right at timer rollover, or at equal intervals after that
 *	i.e. if we have a smoothing ratio of 4, here's what the timing could look like:
 *
 *				   Pulse Count Firing: === [3] ======= [0] ======= [1] ======= [2] ======= [3] ======== [0] ===
 *	Step Computation Interrupt Firing: =================|================================================| ====
 *
 *	The step computation will calculate when to fire a pulse during the *next* timestep, i.e.
 *
 *
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#ifndef INC_PULSER_H_
#define INC_PULSER_H_

#include "app_hal_dio.h" //for pin control and stuff

extern "C" {
	#include "stm32f4xx_hal.h" //for uint8_t
}

#include "axis.h"

//to tell the pulser what
typedef enum {
	SMOOTHING_RATIO_2X = 2,
	SMOOTHING_RATIO_4X = 4,
	SMOOTHING_RATIO_8X = 8
} pulse_smoothing_ratio_t;

//informs the pulser about how exactly to move axes
typedef struct {
	uint8_t num_axes; //how many axes involved in move
	float *last_axis_position; //store the floating point axis positions from the previous loop
	Axis *axes_to_move; //pointer to first element of array of axes to move
} pulse_param_t;

typedef struct {
	uint8_t num_axes_queued;
	Axis *axes_to_pulse[16];
} pulse_queue_t;

class Pulser {
public:

	//splitting the pulser computation into two parts
	//the first part computes the step delta and the thresholds required to actually move a step
	//the second calculates if we need to move a step and stages a step if necessary
	//data shared between the two processes are stored in private class variables between these function calls
	static void compute_step(float* axis_steps, pulse_param_t *pulse_params);
	static void compute_pulse(pulse_param_t *pulse_params);

	//interrupt service routine for the pulser timer
	//return the pulse_counter value at the end of the loop
	//useful for synchronizing other motion calculations
	static uint8_t service_pulse_timer();
	static uint8_t get_max_pulse_count(); //utility function for scheduling pulse computation

private:
	Pulser(); //don't allow instantiation

	//when we want to actually step an axis, call this function with the axis and the proper pulse time
	static void queue_step_pulse(Axis *axis_to_step, uint8_t pulse_to_go_high);

	//some statically allocated placeholder values for axis positions
	//might not be used depending on compiler optimizations we'll see
	//will just need to have as many elements as axes in move, so just making it kinda big for future-proofing
	//I don't think this will bite me in the future, but just gonna flag with a TODO: increase size as necessary
	static float steps_minus_half[16];
	static float steps_plus_half[16];
	static float step_delta[16];

	//========================= stuff related to running the actual pulse generation =============================

	static const pulse_smoothing_ratio_t SMOOTHING_RATIO = SMOOTHING_RATIO_4X;

	static uint8_t pulse_counter; //particular sample we're at
	static const uint8_t pulse_counter_epoch_mask = (uint8_t)SMOOTHING_RATIO;
	static const uint8_t pulse_counter_rollover_mask = ((uint8_t)SMOOTHING_RATIO * 2) - 1;
	static const uint8_t pulse_counter_count_mask = ((uint8_t)SMOOTHING_RATIO) - 1; ;

	static pulse_queue_t pulse_rising_edge[8];
	static pulse_queue_t pulse_falling_edge[8];
};




#endif /* INC_PULSER_H_ */
