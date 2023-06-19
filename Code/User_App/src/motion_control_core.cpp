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


//==================== INITIALIZE STATIC MEMBERS ===================
const Timer *Motion_Control_Core::mc_core_tim = NULL;
const Virtual_Int *Motion_Control_Core::mc_calc_vint = NULL;
const Virtual_Int *Motion_Control_Core::mc_stage_step_vint = NULL;


void Motion_Control_Core::configure(const Timer &_mc_core_tim,
									const Virtual_Int &_mc_calc_vint, const Virtual_Int &_mc_stage_step_vint) {
	//associate the passed scheduling utilities to the class members
	mc_core_tim = &_mc_core_tim;
	mc_calc_vint = &_mc_calc_vint;
	mc_stage_step_vint = &_mc_stage_step_vint;

	//set up the timer
	mc_core_tim->init();
	mc_core_tim->set_phase(0);
	mc_core_tim->set_freq(Timer::FREQ_200kHz); //with 4x oversampling, stepper loop runs at 50kHz
	mc_core_tim->set_int_priority(Priorities::REALTIME);
	mc_core_tim->set_callback_func(Motion_Control_Core::pulse_interrupt);

	//hook up the motion control core calculation interrupt to a virtual interrupt
	//triggered by the pulse interrupt, and designed to be pre-empted by quick IO functions
	mc_calc_vint->set_callback_func(Motion_Control_Core::calc_interrupt);
	mc_calc_vint->set_int_priority(Priorities::MED);

	//hook up the motion control step staging interrupt to a virtual interrupt
	//triggered by the pulse interrupt
	//should run pretty fast, but allowing it to be pre-empted if necessary
	mc_stage_step_vint->set_callback_func(Motion_Control_Core::stage_step_interrupt);
	mc_stage_step_vint->set_int_priority(Priorities::MED);
}

void Motion_Control_Core::pulse_interrupt() {
	//generate any step pulses as necessary
	uint8_t pulse_count = Pulser::service_pulse_timer();

	//at the start of the pulse cycle, run the motion control core calculation
	if(pulse_count == 0)
		mc_calc_vint->trigger();

	//after the last pulse of a calculation cycle, stage any pulses for the next cycle
	else if(pulse_count == Pulser::get_max_pulse_count())
		mc_stage_step_vint->trigger();
}


void Motion_Control_Core::calc_interrupt() {
	//todo: run calculation here
//	float tick_inc_ms = 0.025;
//	motion_tick_ms += tick_inc_ms; //increment the tick by the appropriate amount
//
//	//if we're accelerating
//	if(motion_tick_ms < accel_finshed) {
//		//absolute position required since integration would result in error over time
//		axis_position_f = 	A_over_2 * motion_tick_ms * motion_tick_ms
//							 + A_over_w2 * (Trig::cos((uint16_t)(omega_scaled * motion_tick_ms)) - 1.0);
//	}
//	//if we're decelerating
//	else if (motion_tick_ms > (decel_start)) {
//		//absolute position required since integration would result in error over time
//		float negative_t = move_time - motion_tick_ms; //basically run acceleration backward
//		axis_position_f = 	total_distance -
//							A_over_2 * negative_t * negative_t
//							 - A_over_w2 * (Trig::cos((uint16_t)(omega_scaled * negative_t)) - 1.0);
//	}
//	//otherwise we're just cruising
//	else {
//		axis_position_f += tick_inc_ms * cruise_vel_steps_ms; //can just integrate this; shouldn't have any integration error here
//	}
//
//	//step the motor if we cross an integer step count
//	if((int32_t)axis_position_f > (axis_position_steps + 1)) {
//		if(step_high) {
//			x_step_pin.clear();
//			y_step_pin.clear();
//			step_high = false;
//		}
//		else {
//			x_step_pin.set();
//			y_step_pin.set();
//			step_high = true;
//		}
//		axis_position_steps++;
//	}
//
//
//	//if we're done with the move, reset all the variables
//	if(motion_tick_ms >= move_time) {
//		motion_tick_ms = 0;
//		axis_position_f = 0;
//		axis_position_steps = 0;
//	}

}

void Motion_Control_Core::stage_step_interrupt() {
	//todo: stage pulse here
}

void Motion_Control_Core::enable_mc_core() {
	//enable the timer and interrupt
	mc_core_tim->enable_int();
	mc_core_tim->enable_tim();
}

void Motion_Control_Core::disable_mc_core() {
	//disable the timer and interrupt
	mc_core_tim->disable_tim();
	mc_core_tim->disable_int();
}

//initialization of static members
float Motion_Control_Core::motion_tick_ms = 0;
float Motion_Control_Core::axis_position_f = 0;
int32_t Motion_Control_Core::axis_position_steps = 0;
