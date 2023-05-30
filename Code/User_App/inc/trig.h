/*
 * trig.h
 *
 *  Created on: May 6, 2023
 *      Author: Ishaan
 */

#ifndef INC_TRIG_H_
#define INC_TRIG_H_

extern "C" {
	#include "stm32f4xx_hal.h" //for uint16_t
}

class Trig {
public:

	//implement a 12-bit LUT-based sine function
	//implements the function sin(2*pi/4096)
	//aggressively optimize for high performance in interrupt contexts
	static __attribute__((optimize("O3"))) float sin(uint16_t x);

	//implement a 12-bit LUT-based cosine function
	//implements the function cos(2*pi/4096)
	//aggressively optimize for high performance in interrupt contexts
	static __attribute__((optimize("O3"))) float cos(uint16_t x);

	//implement a 12-bit LUT-based cosine - 1 function
	//implements the function cos(2*pi/4096) - 1
	//aggressively optimize for high performance in interrupt contexts
	static __attribute__((optimize("O3"))) float cos_minus_1(uint16_t x);

private:
	Trig(); //don't allow instantiation

	//the actual look-up tables
	static const float SINE_LUT[4096];
	static const float COSINE_LUT[4096];
	static const float COSm1_LUT[4096];

};




#endif /* INC_TRIG_H_ */
