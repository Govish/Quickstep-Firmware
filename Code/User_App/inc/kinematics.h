/*
 * kinematics.h
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#ifndef INC_KINEMATICS_H_
#define INC_KINEMATICS_H_

#include "axis.h" //to pull steps per mm value from each axis

#include "stm32f4xx.h" //for uint8_t

/**
 * different types of computation we can perform in the inematics calculation stage
 * 	- VALUE_PASSTHROUGH			will pass all values from input directly to the output with no scaling/transform applied
 * 								is useful for linear moves and cartesian kinematics
 * 								when steps per mm scaling has already been computed in pathing stage;
 * 								very computationally efficient since no computation is done lol
 *
 * 	- STEPS_PER_MM_SCALING		will scale all axis values by the corresponding values in the steps per mm array
 * 								I don't imagine this being used often, but just putting this here for completeness
 *
 * 	- XY_STEPS_PER_MM_SCALING	will scale only the first two values by the corresponding values in the steps per mm array
 * 								useful for nonlinear moves in xy space when steps per mm scaling has been done for other axes
 * 								avoids the extra computation time required for steps per mm scaling all other axes
 * 								num axes should be at least 2, excess axes will be passed 1:1 to the output
 *
 *  - XYZ_TRANSFORM				will apply a transformation matrix to the xyz coordinates only
 *  							with first element of matrix pointed to by float pointer
 *  							useful for nonlinear moves with tramming correction; all other axes will be mapped one-to-one
 *								num axes should be at least 3, excess axes will be passed 1:1 to the output
 *								matrix should be ordered (row, column): (0,0); (0,1); (0,2); (1,0); (1,1), etc.
 *
 * 	Future development may handle extrusion separately
 */
typedef enum {
	VALUE_PASSTHROUGH,
	STEPS_PER_MM_SCALING,
	XY_STEPS_PER_MM_SCALING,
	XYZ_TRANSFORM
} kinematic_calc_t;

typedef struct {
	uint8_t num_axes; //how many axes involved in move
	kinematic_calc_t kinematics_to_do; //what kinda kinematic computation we should do, see above
	float *kin_math_factors; //pre-extracted steps-per-mm values or transform matrix values
} kinematic_param_t;

class Kinematics {
public:
	//perform the desired kinematic calculation to map space coords to axis steps
	static void space_to_steps(float* space_coords, kinematic_param_t *params, float* axis_steps);

private:
	Kinematics(); //prevent instantiation of a kinematics class

	static void passthrough(float* space_coords, kinematic_param_t *params, float* axis_steps);
	static void scale_steps_per_mm(float* space_coords, kinematic_param_t *params, float* axis_steps);
	static void scale_xy_steps_per_mm(float* space_coords, kinematic_param_t *params, float* axis_steps);
	static void xyz_transform(float* space_coords, kinematic_param_t *params, float* axis_steps);
};



#endif /* INC_KINEMATICS_H_ */
