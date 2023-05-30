/*
 * pather.cpp
 *
 * Leverage the use of CMSIS DSP library to potentially accelerate computation
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#include "pather.h"
#include "arm_math.h" //for useful optimized vector

void Pather::path_to_3space(float dist_along_path, move_path_param_t *move_path, float *pos_3space) {
	if(move_path->move_type == LINEAR)
		//pass off to a separate function that handles pathing through linear moves
		path_to_3space_linear(dist_along_path, &move_path->move_params.linear_move_params, pos_3space);

	else if(move_path->move_type == ARC)
		; //todo
}


//=========================== PRIVATE FUNTION DEFS ==========================

void Pather::path_to_3space_linear(float dist_along_path, linear_move_param_t *linear_move_params, float *pos_3space) {
	//this computation is accomplished by a vector multiply by a scalar and a vector addition
	//accomplish this through the CMSIS DSP library for any potential optimizations

	//first do the vector * scalar multiplication
	arm_scale_f32(	linear_move_params->move_unit_vec, dist_along_path,
					lin_intermediate_result, linear_move_params->num_axes);

	//then do the element-wise vector addition
	arm_add_f32(	linear_move_params->starting_pos, lin_intermediate_result,
					pos_3space, linear_move_params->num_axes);
}
