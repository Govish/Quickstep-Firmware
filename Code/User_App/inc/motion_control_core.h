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

class Motion_Control_Core {
public:

	//this is the function where the heart of the motion control happens
	//runs in an interrupt context, called from a timer interrupt
	//pass in the how many microseconds since the last interrupt
	static void mc_core_interrupt(const DIO &step_pin, float tick_inc_ms);

private:
	Motion_Control_Core(); //shouldn't be able to instantiate these

	static float motion_tick_ms; //accumulates the time into the particular move being traced
	static float axis_position_f; //floating point axis position
	static int32_t axis_position_steps; //axis position in steps, is an integer
};

#endif /* INC_MOTION_CONTROL_CORE_H_ */
