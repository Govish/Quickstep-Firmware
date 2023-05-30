/*
 * axis.h
 *
 * A class that holds all functionality related to a single axis control
 *
 *  Created on: May 25, 2023
 *      Author: Ishaan
 */

#ifndef INC_AXIS_H_
#define INC_AXIS_H_

#include "app_hal_dio.h"

class Axis {
public:
	Axis(const DIO &_step, const DIO &_dir, const DIO &_en, const bool reversed, const char _alias);


	void set_pos(int32_t position); //we'll cast to a float when we save thee variable
	char get_axis_alias(); //return the gcode letter that corresponds with this axis

//low level GPIO functions with the stepper driver, optimize for high performance
//not explicitly inlining the function to let compiler do what it feels is best
#pragma GCC push_options
#pragma GCC optimize ("O3")
	float get_pos(); //will have to do this frequently, make sure this operation is optimized
	void enable();
	void disable();
	void step_high();
	void step_low();
	void dir_forward();
	void dir_reverse() ;
#pragma GCC pop_options

private:
	const DIO *step;
	const DIO *dir;
	const DIO *en;

	const char axis_alias; //what gcode letter corresponds to this axis

	const bool DIRECTION_REVERSED;

	float pos_min, pos_max;
	float vel_max;
	float accel_max;

	float position_f;
	float pos_increment_val; //either 1 or -1 depending on the direction of the motor
	int32_t position_steps;
};



#endif /* INC_AXIS_H_ */
