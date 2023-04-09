/*
 * app_hal_timing.cpp
 *
 *  Created on: Mar 10, 2023
 *      Author: Ishaan
 *
 *	TIMER MAPPINGS:
 *      - TIM13 >>> DEBOUNCE COMPUTATION TIMER
 *      - TIM14 >>> DEBOUNCE SAMPLE TIMER
 *
 */

#include "app_hal_timing.h"
extern "C" {
	#include "tim.h"
}

//=========================== INITIALIZING STAIC MEMBERS HERE ==========================
//initializing these empty callbacks for now, associate them with the proper callback funcs in the initializers
void empty_handler();

callback_function_t Timing::db_cb = &empty_handler;
callback_function_t Timing::soft_pwm_cb = &empty_handler;
callback_function_t Timing::supervisor_cb = &empty_handler;

//=========================== SANITY CHECK RANGES FOR DEBOUNCE TIMERS =================================
#define DB_TIMER_FREQ_MIN	500 //Hz, minimum timer frequency for debouncer timer
#define DB_TIMER_FREQ_MAX	2000

//========================= TIMER MAPPINGS ============================
#define DB_TIM_INIT_FUNC	MX_TIM13_Init
#define DB_TIM 				htim13
#define DB_IRQn				TIM8_UP_TIM13_IRQn
#define DB_IRQ_HANDLER		TIM8_UP_TIM13_IRQHandler

#define SOFT_PWM_TIM_INIT_FUNC		MX_TIM14_Init
#define SOFT_PWM_TIM 				htim14
#define SOFT_PWM_IRQn				TIM8_TRG_COM_TIM14_IRQn
#define SOFT_PWM_IRQ_HANDLER		TIM8_TRG_COM_TIM14_IRQHandler

#define SUP_TIM_INIT_FUNC			MX_TIM9_Init
#define SUP_TIM						htim9
#define SUP_IRQn					TIM1_BRK_TIM9_IRQn
#define SUP_IRQ_HANDLER				TIM1_BRK_TIM9_IRQHandler

void Timing::init_db_timer (
		const float freq, const float offset,
		callback_function_t _db_cb, int_priority_t prio) {

	//refer the callback function to the one passed
	Timing::db_cb = _db_cb;

	DB_TIM_INIT_FUNC();
	DB_TIM.Instance->PSC = 89; //tick the timer over at 1MHz
	uint32_t period = (uint32_t)(1000000.0/freq);
	uint32_t offset_cnt = (uint32_t)(period * offset);
	offset_cnt = (offset_cnt >= period) ? period - 1 : offset_cnt;
	DB_TIM.Instance->ARR = period - 1;
	DB_TIM.Instance->CCR1 = offset_cnt;

	//configure the interrupt to just be when we hit the compare interrupt
	DB_TIM.Instance->DIER = TIM_DIER_CC1IE; //don't want the update interrupt

	//and configure the NVIC with the corresponding priority
	HAL_NVIC_SetPriority(DB_IRQn, (uint32_t)prio, 0);
	HAL_NVIC_DisableIRQ(DB_IRQn); //don't enable the interrupt just yet
	HAL_NVIC_ClearPendingIRQ(DB_IRQn); //and clear the NVIC if we have something pending
}

void Timing::init_soft_pwm_timer (
		const float freq, const float offset,
		callback_function_t _soft_pwm_cb, int_priority_t prio) {

	//refer the callback function to the one passed
	Timing::soft_pwm_cb = _soft_pwm_cb;

	SOFT_PWM_TIM_INIT_FUNC();
	SOFT_PWM_TIM.Instance->PSC = 89; //tick the timer over at 1MHz
	uint32_t period = (uint32_t)(1000000.0/freq); //timer ticks over internally at 10kHz
	uint32_t offset_cnt = (uint32_t)(period * offset);
	offset_cnt = (offset_cnt >= period) ? period - 1 : offset_cnt;
	SOFT_PWM_TIM.Instance->ARR = period - 1;
	SOFT_PWM_TIM.Instance->CCR1 = offset_cnt;

	//configure the interrupt to just be when we hit the compare interrupt
	SOFT_PWM_TIM.Instance->DIER = TIM_DIER_CC1IE; //don't want the update interrupt

	//and configure the NVIC with the corresponding priority
	HAL_NVIC_SetPriority(SOFT_PWM_IRQn, (uint32_t)prio, 0);
	HAL_NVIC_DisableIRQ(SOFT_PWM_IRQn); //don't enable the interrupt just yet
	HAL_NVIC_ClearPendingIRQ(SOFT_PWM_IRQn); //and clear the NVIC if we have something pending
}

void Timing::init_supervisor_timer (
		const float freq, const float offset,
		callback_function_t _supervisor_cb, int_priority_t prio) {

	//refer the callback function to the one passed
	Timing::supervisor_cb = _supervisor_cb;

	SUP_TIM_INIT_FUNC();
	SUP_TIM.Instance->PSC = 3599; //tick the timer over at 25kHz
	uint32_t period = (uint32_t)(25000.0/freq); //calculate the period register val based off the timer tick frequency (25kHz)
	uint32_t offset_cnt = (uint32_t)(period * offset);
	offset_cnt = (offset_cnt >= period) ? period - 1 : offset_cnt;
	SUP_TIM.Instance->ARR = period - 1;
	SUP_TIM.Instance->CCR1 = offset_cnt;

	//configure the interrupt to just be when we hit the compare interrupt
	SUP_TIM.Instance->DIER = TIM_DIER_CC1IE; //just have the channel 1 compare interrupt firing

	//and configure the NVIC with the corresponding priority
	HAL_NVIC_SetPriority(SUP_IRQn, (uint32_t)prio, 0);
	HAL_NVIC_DisableIRQ(SUP_IRQn); //don't enable the interrupt just yet
	HAL_NVIC_ClearPendingIRQ(SUP_IRQn); //and clear the NVIC if we have something pending
}

void Timing::start_timers() {
	DB_TIM.Instance->CR1 = TIM_CR1_CEN; //enable the debounce compute timer
	SOFT_PWM_TIM.Instance->CR1 = TIM_CR1_CEN; //enable the debounce sample timer
	SUP_TIM.Instance->CR1 = TIM_CR1_CEN; //enable the supervisor sample timer

	//enable the interrupts in the NVIC
	HAL_NVIC_EnableIRQ(DB_IRQn);
	HAL_NVIC_EnableIRQ(SOFT_PWM_IRQn);
	HAL_NVIC_EnableIRQ(SUP_IRQn);
}

void Timing::delay_ms(uint32_t ms) {
	HAL_Delay(ms);
}

uint32_t Timing::get_ms(){
	return HAL_GetTick();
}

//================================== TIMING CLASS INTERRUPT SERVICE ROUTINES ===================================

void Timing::db_isr() {
	//run the callback
	Timing::db_cb();
	//we know that there's only one possible interrupt source and it's the compare interrupt
	DB_TIM.Instance->SR = 0; //clears it so we can run it back in the future
}

void Timing::soft_pwm_isr() {
	//run the callback
	Timing::soft_pwm_cb();
	//we know that there's only one possible interrupt source and it's the compare interrupt
	SOFT_PWM_TIM.Instance->SR = 0; //clear all interrupt flags so we can run it back in the future
}

void Timing::sup_isr() {
	//run the callback
	Timing::supervisor_cb();
	//we know that there's only one possible interrupt source and it's the channel 1 compare interrupt
	SUP_TIM.Instance->SR = 0; //clear all interrupt flags so we can run it back in the future
}

//======================================= TIMER ISRs MAPPED TO VECTOR TABLE ===================================

void DB_IRQ_HANDLER(void) {
	//service the ISR with the class ISR method
	Timing::db_isr();
}

void SOFT_PWM_IRQ_HANDLER(void) {
	//service the ISR with the class ISR method
	Timing::soft_pwm_isr();
}

void SUP_IRQ_HANDLER(void) {
	//service the ISR with the class ISR method
	Timing::sup_isr();
}

void empty_handler() {}
