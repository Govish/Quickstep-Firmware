/*
 * kinematics.cpp
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#include "kinematics.h"

#include "arm_math.h" //use CMSIS DSP library to potentially speed up any calculations

void Kinematics::space_to_steps(float* space_coords, kinematic_param_t *params, float* axis_steps) {
	switch (params->kinematics_to_do) {
		case STEPS_PER_MM_SCALING:
			scale_steps_per_mm(space_coords, params, axis_steps);
			break;
		case XY_STEPS_PER_MM_SCALING:
			scale_xy_steps_per_mm(space_coords, params, axis_steps);
			break;
		case XYZ_TRANSFORM:
			xyz_transform(space_coords, params, axis_steps);
			break;
		case VALUE_PASSTHROUGH:
		default: //let this be VALUE_PASSTHROUGH
			passthrough(space_coords, params, axis_steps);
			break;
	}
}

//=================== PRIVATE FUNCTION DEFS ===================

//scale all n-space coordinates by a corresponding steps-per-mm value
void Kinematics::scale_steps_per_mm(float* space_coords, kinematic_param_t *params, float* axis_steps) {
	//use CMSIS DSP library to do a fast vector multiply here
	arm_mult_f32(space_coords, params->kin_math_factors, axis_steps, params->num_axes);
}

//scale just the xy values by a steps-per-mm value, pass the rest of the coordinates 1:1 to the output
void Kinematics::scale_xy_steps_per_mm(float* space_coords, kinematic_param_t *params, float* axis_steps) {
	//do a vector multiply for the first two axes
	arm_mult_f32(space_coords, params->kin_math_factors, axis_steps, 2);

	//and just passthrough the spatial coordinates to the output
	if(params->num_axes > 2)
		arm_copy_f32(&space_coords[2], &axis_steps[2], params->num_axes - 2);
}

//perform a coordinate transform on the 3D spatial coordinates given a transformation matrix
//pass the rest of the coordinates 1:1 to the output
void Kinematics::xyz_transform(float* space_coords, kinematic_param_t *params, float* axis_steps) {
	//matrix-vector multiply doesn't exist in the DSP library I'm not using, so doing it manually via dot products
	arm_dot_prod_f32(space_coords, &params->kin_math_factors[0], 3, &axis_steps[0]); //first row of matrix
	arm_dot_prod_f32(space_coords, &params->kin_math_factors[3], 3, &axis_steps[1]); //second row of matrix
	arm_dot_prod_f32(space_coords, &params->kin_math_factors[6], 3, &axis_steps[2]); //second row of matrix

	//and just passthrough the spatial coordinates to the output
	if(params->num_axes > 3)
		arm_copy_f32(&space_coords[3], &axis_steps[3], params->num_axes - 3);
}

//pass the inputs directly to the output (but copy for safety)
void Kinematics::passthrough(float* space_coords, kinematic_param_t *params, float* axis_steps) {
	//use CMSIS DSP library to do the copy quickly
	arm_copy_f32(space_coords, axis_steps, params->num_axes);
}
