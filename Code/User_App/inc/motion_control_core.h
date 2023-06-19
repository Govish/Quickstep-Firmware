/*
 * motion_control_core.h
 *
 *  Created on: May 6, 2023
 *      Author: Ishaan
 */

#ifndef INC_MOTION_CONTROL_CORE_H_
#define INC_MOTION_CONTROL_CORE_H_

extern "C" {
	#include "stm32f4xx_hal.h" //for int32_t
}

#include "app_hal_dio.h" //for pin manipulation

#include "app_hal_timing.h" //for mc_core timer control
#include "app_hal_virtual_int.h" //to schedule calculation interrupts and step staging interrupts

#include "profiler.h" //compute the speed of the move given a timestep
#include "pather.h" //compute path of the axes in 3-space
#include "kinematics.h" //inverse kinematics to get to axis step positions
#include "pulser.h" //generate the step pulse for each stepper motor driver

class Motion_Control_Core {
public:

	//pass in instances of scheduling utilities
	//set them up and hook up handles to them with this function
	static void configure(	const Timer &_mc_core_tim,
							const Virtual_Int &_mc_calc_vint, const Virtual_Int &_mc_stage_step_vint);

	//this is where all of the motion control calculation happens
	//runs in an interrupt context, called from the NVIC indirectly from another interrupt
	static void calc_interrupt();

	//this is where step pulses get staged for the next timestep
	//runs in an interrupt context, called from the NVIC indirectly from another interrupt
	static void stage_step_interrupt();

	//this is where the step pulses get executed (though through a helper function in pulser)
	//called from an interrupt context, stages execution of mc_core_calc_interrupt() and mc_core_stage_step_interrupt()
	static void pulse_interrupt();

	//functions that enable and disable the timer corresponding to the motion control core
	static void enable_mc_core();
	static void disable_mc_core();

private:
	Motion_Control_Core(); //shouldn't be able to instantiate these

	//pieces of firmware relevant to task scheduling
	static const Timer *mc_core_tim;
	static const Virtual_Int *mc_calc_vint;
	static const Virtual_Int *mc_stage_step_vint;

	static float motion_tick_ms; //accumulates the time into the particular move being traced
	static float axis_position_f; //floating point axis position
	static int32_t axis_position_steps; //axis position in steps, is an integer
};

#endif /* INC_MOTION_CONTROL_CORE_H_ */
