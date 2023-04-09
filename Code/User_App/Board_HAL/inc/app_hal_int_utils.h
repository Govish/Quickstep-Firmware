/*
 * app_hal_int_priority_maps.h
 *
 *  Created on: Mar 11, 2023
 *      Author: Ishaan
 */

#ifndef BOARD_HAL_INC_APP_HAL_INT_UTILS_H_
#define BOARD_HAL_INC_APP_HAL_INT_UTILS_H_

//define a type for callback functions that we can call when interrupts are handled
//doing this for easier readability
//https://stackoverflow.com/questions/6339970/c-using-function-as-parameter
typedef void (*callback_function_t)(void);

//enum numeric mappings correspond to to NVIC values
typedef enum Priorities {
	REALTIME = 0,
	HIGH = 1,
	MED_HIGH = 2,
	MED = 3,
	MED_LOW = 4,
	LOW = 5
} int_priority_t;


//====================== DECLARING INTERRUPT HANDLERS HERE =========================
extern "C" {
	void TIM8_UP_TIM13_IRQHandler(void); //debouncer timer
	void TIM8_TRG_COM_TIM14_IRQHandler(void); //soft PWM timer
	void TIM1_BRK_TIM9_IRQHandler(void); //supervisor timer
	void TIM2_IRQHandler(void); //hard PWM
	void TIM3_IRQHandler(void); //hard PWM
}

#endif /* BOARD_HAL_INC_APP_HAL_INT_UTILS_H_ */
