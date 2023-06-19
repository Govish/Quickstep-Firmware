/*
 * app_hal_virtual_int.h
 *
 *	A way to schedule the execution of a function as an interrupt service routine
 *	Leverages ARM Cortex NVIC for scheduling basically
 *
 *  Created on: May 30, 2023
 *      Author: Ishaan
 */

#ifndef BOARD_HAL_INC_APP_HAL_VIRTUAL_INT_H_
#define BOARD_HAL_INC_APP_HAL_VIRTUAL_INT_H_

extern "C" {
	#include "stm32f4xx_hal.h" //need this for NVIC functions
	#include "stm32f446xx.h" //need this for the IRQn_type
}

#include "app_hal_int_utils.h"

//have a very explicit enum type to map timer firmware instance to hardware
typedef enum Virtual_Int_Channels {
	VINT_CHANNEL_0 = 0,
	VINT_CHANNEL_1 = 1
} virtual_int_channel_t;

class Virtual_Int {
public:
	Virtual_Int(virtual_int_channel_t _channel);

	//assign a callback function and priority for the interrupt
	void set_callback_func(callback_function_t cb) const;
	void set_int_priority(int_priority_t prio) const;

	//this function actually triggers the interrupt virtually
	void trigger() const;

	//just have a single ISR function calls the appropriate callback function
	//if this is optimized hard enough, the array indexing should hopefully unroll and be
	//as fast as if we had individual ISRs for each channel
	//NOTE FOR PORTING: APP WILL NEVER CALL THIS FUNCTION, SO IMPLEMENT HOW YOU'D LIKE
	static void __attribute__((optimize("O3"))) ISR_func(int channel);

private:
	static const IRQn_Type hard_int_channels[];
	static callback_function_t callbacks[];

	int channel; //which channel the particular instance is mapped to
};

#endif /* BOARD_HAL_INC_APP_HAL_VIRTUAL_INT_H_ */
