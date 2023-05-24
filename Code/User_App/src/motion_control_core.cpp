/*
 * motion_control_core.cpp
 *
 *  Created on: May 6, 2023
 *      Author: Ishaan
 */

#include "motion_control_core.h"
#include "trig.h"

volatile static bool step_high = false;

//================= for testing, define these as constants ================
#define STEPS_PER_MM 	160.0f //nominal GT2 with 20 tooth pulley with 16th microstepping
#define AVG_ACCEL 		300.0f //mm/s^2 accel value
#define CRUISE_VELOCITY	500.0f //mm/s cruise speed for the motor

#define MOVE_DISTANCE_X 400.0f //mm move distance for the x axis
#define MOVE_DISTANCE_Y 400.0f //mm move distance for the y axis
#define MOVE_DISTANCE	800.0f //mm move distance (should work out to 10 rotations)

//============== converting these above constants to units of steps and microseconds =================
const float PI = 3.14159;

//##### calculate acceleration related values for the move
static const float accel_time = (CRUISE_VELOCITY / AVG_ACCEL) * 1000.0; //in ms
static const float omega = 2 * PI / (accel_time); //tells us how quickly go accelerate; 2 * pi / accel_time (ms) is omega
static const float omega_scaled = 4096.0 / accel_time; //scaled to work with the cosine look-up table
static const float A = AVG_ACCEL * STEPS_PER_MM / (1e6);//steps per ms^2; used by position equation
static const float A_over_2 = A / 2.0; //need this for position equation
static const float A_over_w2 = A / (omega * omega); //need this for position equation

//##### calculate velocity related values for the move
static const float cruise_vel_steps_ms = CRUISE_VELOCITY * STEPS_PER_MM / 1000.0; //steps per ms

//##### calculate timing related values for the move
static const float accel_distance = A_over_2 * accel_time * accel_time; //in steps
static const float total_distance = MOVE_DISTANCE * STEPS_PER_MM; //in steps
static const float cruise_distance = total_distance - (2 * accel_distance); //in steps
static const float cruise_time = cruise_distance / cruise_vel_steps_ms; //in ms
static const float move_time = 2 * accel_time + cruise_time; //in ms
static const float accel_finshed = accel_time; //ms value at which acceleration phase is completed
static const float decel_start = move_time - accel_time; //ms value at which deceleration phase starts

void Motion_Control_Core::mc_core_interrupt(const DIO &x_step_pin, const DIO &y_step_pin, float tick_inc_ms) {
	motion_tick_ms += tick_inc_ms; //increment the tick by the appropriate amount

	//if we're accelerating
	if(motion_tick_ms < accel_finshed) {
		//absolute position required since integration would result in error over time
		axis_position_f = 	A_over_2 * motion_tick_ms * motion_tick_ms
							 + A_over_w2 * (Trig::cos((uint16_t)(omega_scaled * motion_tick_ms)) - 1.0);
	}
	//if we're decelerating
	else if (motion_tick_ms > (decel_start)) {
		//absolute position required since integration would result in error over time
		float negative_t = move_time - motion_tick_ms; //basically run acceleration backward
		axis_position_f = 	total_distance -
							A_over_2 * negative_t * negative_t
							 - A_over_w2 * (Trig::cos((uint16_t)(omega_scaled * negative_t)) - 1.0);
	}
	//otherwise we're just cruising
	else {
		axis_position_f += tick_inc_ms * cruise_vel_steps_ms; //can just integrate this; shouldn't have any integration error here
	}

	//step the motor if we cross an integer step count
	if((int32_t)axis_position_f > (axis_position_steps + 1)) {
		if(step_high) {
			x_step_pin.clear();
			y_step_pin.clear();
			step_high = false;
		}
		else {
			x_step_pin.set();
			y_step_pin.set();
			step_high = true;
		}
		axis_position_steps++;
	}


	//if we're done with the move, reset all the variables
	if(motion_tick_ms >= move_time) {
		motion_tick_ms = 0;
		axis_position_f = 0;
		axis_position_steps = 0;
	}

}

//initialization of static members
float Motion_Control_Core::motion_tick_ms = 0;
float Motion_Control_Core::axis_position_f = 0;
int32_t Motion_Control_Core::axis_position_steps = 0;
