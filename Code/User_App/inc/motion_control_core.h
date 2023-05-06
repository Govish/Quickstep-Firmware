/*
 * motion_control_core.h
 *
 *  Created on: May 6, 2023
 *      Author: Ishaan
 */

#ifndef INC_MOTION_CONTROL_CORE_H_
#define INC_MOTION_CONTROL_CORE_H_

class Motion_Control_Core {
public:

	//this is the function where the heart of the motion control happens
	//runs in an interrupt context, called from a timer interrupt
	//pass in the how many microseconds since the last interrupt
	static void mc_core_interrupt(float tick_inc_us);

private:
	Motion_Control_Core(); //shouldn't be able to instantiate these
};

#endif /* INC_MOTION_CONTROL_CORE_H_ */
