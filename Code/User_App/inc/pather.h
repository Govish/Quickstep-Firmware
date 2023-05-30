/*
 * pather.h
 *
 *  Created on: May 24, 2023
 *      Author: Ishaan
 */

#ifndef INC_PATHER_H_
#define INC_PATHER_H_

#include "config.h"

#include "stm32f4xx.h" //for uint8_t

typedef enum {
	LINEAR,
	ARC
} move_t;

//for a linear move, we need
typedef struct {
	uint8_t num_axes; //number of axes involved in the move
	float *starting_pos; //starting position for each axis
	float *move_unit_vec; //how much each axis moves per unit path distance traversed
} linear_move_param_t;

typedef struct {
	float starting_points;
	//TODO: finish definition of arc moves
} arc_move_param_t;

typedef struct {
	move_t move_type;
	union {
		linear_move_param_t linear_move_params;
		arc_move_param_t arc_move_params;
	} move_params;
} move_path_param_t;

class Pather {
public:

	//take the distance along path, move parameters and fill in the pos_3space variable
	static void path_to_3space(float dist_along_path, move_path_param_t *move_path, float *pos_3space);

private:
	Pather(); //don't allow instantiation
	static void path_to_3space_linear(float dist_along_path, linear_move_param_t *linear_move_params, float *pos_3space);

	//constant spot in memory to store the intermediate result of the linear pathing calculation
	//might not be used depending on compiler optimizations we'll see
	//will just need to have as many elements as axes, so just making it kinda big for future-proofing
	//I don't think this will bite me in the future, but just gonna flag with a TODO: increase size as necessary
	static float lin_intermediate_result[16];
};



#endif /* INC_PATHER_H_ */
