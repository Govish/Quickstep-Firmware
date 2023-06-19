/*
 * app_hal_timing.cpp
 *
 *  Created on: Mar 10, 2023
 *      Author: Ishaan
 *
 *	TIMER MAPPINGS:
 *
 */

#include "app_hal_timing.h"
extern "C" {
	#include "tim.h"
}

//========================= TIMER IRQ MAPPINGS  ============================
#define CHAN_0_IRQ_HANDLER		TIM1_BRK_TIM9_IRQHandler //tim9
#define CHAN_1_IRQ_HANDLER		TIM1_TRG_COM_TIM11_IRQHandler //tim11
#define CHAN_2_IRQ_HANDLER		TIM8_TRG_COM_TIM14_IRQHandler //tim14

#define TIM_F_CLK 90000000.0f //90MHz--just so other parts of the program can use this for whatever reason
#define TIM_MAX_CNT ((uint16_t)65536) //the maximum value (plus 1) that we can shove into the ARR registers

//=========================== INITIALIZING STAIC MEMBERS HERE ==========================
//initializing these empty callbacks for now, associate them with the proper callback funcs in the initializers
void empty_timer_handler();

//THIS IS HOW THE TIMER OBJECT MAPS TO THE PHSYICAL HARDWARE
const timer_config_struct_t Timer::timer_chan_configs[] = {
		{MX_TIM9_Init, htim9, TIM1_BRK_TIM9_IRQn}, //channel 0 on timer 9
		{MX_TIM11_Init, htim11, TIM1_TRG_COM_TIM11_IRQn}, //channel 1 on timer 11
		{MX_TIM14_Init, htim14, TIM8_TRG_COM_TIM14_IRQn}, //channel 2 on timer 14
};

//initialize the callback function array to just be emtpy handlers at the start
callback_function_t Timer::callbacks[] = {
		empty_timer_handler,
		empty_timer_handler,
		empty_timer_handler
};

//=================== section here just to defining frequency presets ======================
//first number is prescaler value, second is auto-reload value
//make sure to subtract 1 from the values!
const timer_freq_t Timer::FREQ_400kHz = {0, 224}; //90MHz tick
const timer_freq_t Timer::FREQ_200kHz = {0, 449}; //90MHz tick
const timer_freq_t Timer::FREQ_100kHz = {0, 899}; //90MHz tick
const timer_freq_t Timer::FREQ_50kHz = {0, 1799}; //90MHz tick
const timer_freq_t Timer::FREQ_20kHz = {0, 4499}; //90MHz tick
const timer_freq_t Timer::FREQ_10kHz = {0, 8999}; //90MHz tick
const timer_freq_t Timer::FREQ_5kHz = {9, 1799}; //9MHz tick
const timer_freq_t Timer::FREQ_2kHz = {9, 4499}; //9MHz tick
const timer_freq_t Timer::FREQ_1kHz = {9, 8999}; //9MHz tick
const timer_freq_t Timer::FREQ_500Hz = {99, 1799}; //900kHz tick
const timer_freq_t Timer::FREQ_200Hz = {99, 4499}; //900kHz tick
const timer_freq_t Timer::FREQ_100Hz = {99, 8999}; //900kHz tick
const timer_freq_t Timer::FREQ_50Hz = {999, 1799}; //90kHz tick
const timer_freq_t Timer::FREQ_20Hz = {999, 4499}; //90kHz tick
const timer_freq_t Timer::FREQ_10Hz = {999, 8999}; //90kHz tick
const timer_freq_t Timer::FREQ_5Hz = {9999, 1799}; //9kHz tick
const timer_freq_t Timer::FREQ_2Hz = {9999, 4499}; //9kHz tick
const timer_freq_t Timer::FREQ_1Hz = {9999, 8999}; //9kHz tick
const timer_freq_t Timer::FREQ_0p5Hz = {44999, 3999}; //2kHz tick
const timer_freq_t Timer::FREQ_0p2Hz = {44999, 9999}; //2kHz tick
const timer_freq_t Timer::FREQ_0p1Hz = {44999, 19999}; //2kHz tick

//=================== END section defining frequency presets ===================

//======================= PUBLIC FUNCTION DEFINITIONS =========================
Timer::Timer(timer_channel_t _channel): channel((int)_channel) {} //constructor

void Timer::init() const {
	//call the initialization function defined by CubeMX
	Timer::timer_chan_configs[channel].init_func();

	//configure the interrupt to just be when we hit the compare interrupt
	Timer::timer_chan_configs[channel].htim.Instance->DIER = TIM_DIER_CC1IE;
	//don't want the update interrupt just yet though

	//and configure the NVIC with the corresponding priority
	//start it off with medium priority, adjust the priority with the appropriate function
	set_int_priority(Priorities::MED);
}

void Timer::set_freq(timer_freq_t freq) const {
	//basically just drop in the prescaler and auto reload values from the struct into the appropriate registers
	Timer::timer_chan_configs[channel].htim.Instance->ARR = freq.auto_reload;
	Timer::timer_chan_configs[channel].htim.Instance->PSC = freq.prescaler;

}

void Timer::set_phase(float phase) const {
	//make sure phase is in a valid range
	if(phase < 0) return;
	if(phase >= 1) return;

	//phase the counter by seeding the count value to a fraction of the ARR register
	uint16_t max_count = Timer::timer_chan_configs[channel].htim.Instance->ARR;
	Timer::timer_chan_configs[channel].htim.Instance->CNT = (uint16_t)(max_count * phase);
}

void Timer::reset_count() const {
	//start counting from 0 again
	Timer::timer_chan_configs[channel].htim.Instance->CNT = 0;
}

void Timer::set_callback_func(callback_function_t cb) const {
	//just store the pointer to the callback function in the array
	Timer::callbacks[channel] = cb;
}

void Timer::set_int_priority(int_priority_t prio) const {
	//start with disabling the IRQ as we adjust the priority
	HAL_NVIC_DisableIRQ(Timer::timer_chan_configs[channel].irq_type);

	//setup the interrupt with the corresponding priority passed to the function
	HAL_NVIC_SetPriority(Timer::timer_chan_configs[channel].irq_type, (uint32_t)prio, 0);

	//clear the NVIC if we have something pending
	HAL_NVIC_ClearPendingIRQ(Timer::timer_chan_configs[channel].irq_type);
}

void Timer::enable_int() const {
	//enable the NVIC channel
	HAL_NVIC_EnableIRQ(Timer::timer_chan_configs[channel].irq_type);

	//instead just enable the flag in the interrupt control register
	Timer::timer_chan_configs[channel].htim.Instance->DIER = TIM_DIER_CC1IE;
}

void Timer::disable_int() const {
	//kill the interrupt source in the timer control register
	Timer::timer_chan_configs[channel].htim.Instance->DIER = 0;

	//clear any looming interrupts from the NVIC so we don't trigger immediately on timer restart
	//theoretically might not be the best thing to do in the event that the timer shares an interrupt source with something else
	//but want to avoid that situation entirely for performance reasons
	HAL_NVIC_DisableIRQ(Timer::timer_chan_configs[channel].irq_type);
	HAL_NVIC_ClearPendingIRQ(Timer::timer_chan_configs[channel].irq_type);
}

void Timer::enable_tim() const {
	//just set the enable flag in the timer control register and let the hardware do its thing
	Timer::timer_chan_configs[channel].htim.Instance->CR1 |= TIM_CR1_CEN;
}

void Timer::disable_tim() const {
	//just clear the enable flag in the timer control register and let the hardware do its thing
	Timer::timer_chan_configs[channel].htim.Instance->CR1 &= ~(TIM_CR1_CEN);
}

float Timer::get_freq() const {
	//start with timer clock frequency
	//then divide that by the
	return (TIM_F_CLK /
		((Timer::timer_chan_configs[channel].htim.Instance->PSC + 1) *
 		 (Timer::timer_chan_configs[channel].htim.Instance->ARR + 1) ));
}

float Timer::get_tim_fclk() const {
	//useful for other functions that need to compute timer counts and stuff like that
	return TIM_F_CLK;
}

//utility delay function
//should really never be called in the program if we write stuff well
//but useful for debugging
void Timer::delay_ms(uint32_t ms) {
	HAL_Delay(ms);
}

//system tick wrapper
//good for millisecond polling-based scheduling
uint32_t Timer::get_ms(){
	return HAL_GetTick();
}

//================================== TIMING CLASS INTERRUPT SERVICE ROUTINE ===================================

void Timer::ISR_func(int channel) {
	//clear the flag in the corresponding timer register
	Timer::timer_chan_configs[channel].htim.Instance->SR = 0;

	//run the callback function of the corresponding timer channel
	Timer::callbacks[channel]();
}

//======================================= TIMER ISRs MAPPED TO VECTOR TABLE ===================================

void CHAN_0_IRQ_HANDLER(void) {
	//service the ISR with the class on channel 0
	Timer::ISR_func(0);
}

void CHAN_1_IRQ_HANDLER(void) {
	//service the ISR with the class on channel 1
	Timer::ISR_func(1);
}

void CHAN_2_IRQ_HANDLER(void) {
	//service the ISR with the class on channel 2
	Timer::ISR_func(2);
}

void empty_timer_handler() {}
