/*
 * axis.cpp
 *
 *  Created on: May 26, 2023
 *      Author: Ishaan
 */

#include "axis.h"

//============================= heavily optimized public functions ============================
float Axis::get_pos() {
	return position_f;
}

void Axis::enable() {
	en->clear();
}

void Axis::disable() {
	en->set();
}

void Axis::step_high() {
	step->set();
	//only increment on the rising edge of STEP
	position_f += pos_increment_val;
}

void Axis::step_low() {
	step->clear();
}

void Axis::dir_forward() {
	//I'm hoping these conditionals get evaluated at compile time, not sure if possible or not
	if (DIRECTION_REVERSED) dir->clear();
	else dir->set();
	pos_increment_val = 1.0; //increment steps when we're moving forward
}

void Axis::dir_reverse() {
	//I'm hoping these conditionals get evaluated at compile time, not sure if possible or not
	if(DIRECTION_REVERSED) dir->set();
	else dir->clear();
	pos_increment_val = -1.0; //decrement steps when we're reversing
}

