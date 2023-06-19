/*
 * app_hal_pwm.cpp
 *
 *  Created on: Mar 26, 2023
 *      Author: Ishaan
 */

#include "app_hal_pwm.h"
extern "C" {
	#include "tim.h"
}

#define CHANNEL_NOT_MAPPED 0xFF
#define PWM_RESOLUTION	100.0f //have 100 levels of PWM granularity

//========================= TIMER MAPPINGS ============================
#define PWM_A_INIT_FUNC			MX_TIM2_Init
#define PWM_A_TIM 				htim2
#define PWM_A_IRQn				TIM2_IRQn
#define PWM_A_IRQ_HANDLER		TIM2_IRQHandler

#define PWM_B_INIT_FUNC			MX_TIM3_Init
#define PWM_B_TIM 				htim3
#define PWM_B_IRQn				TIM3_IRQn
#define PWM_B_IRQ_HANDLER		TIM3_IRQHandler

#define DEASSERT(index)			Hard_PWM::channel_inverted[index] ? Hard_PWM::pwm_pins[index]->set(): Hard_PWM::pwm_pins[index]->clear()
#define ASSERT(index)			Hard_PWM::channel_inverted[index] ? Hard_PWM::pwm_pins[index]->clear(): Hard_PWM::pwm_pins[index]->set()

//initializing static members, doing this very explicitly bc the arrays aren't huge
bool Hard_PWM::blank_channel[8] = {false, false, false, false, false, false, false, false};
bool Hard_PWM::channel_inverted[8] = {false, false, false, false, false, false, false, false};
const DIO* Hard_PWM::pwm_pins[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
bool Hard_PWM::channel_in_use[8] = {false, false, false, false, false, false, false, false};

Hard_PWM::Hard_PWM(const DIO &_pin, const bool _inverted) {
	//map the new instance to the next free PWM channel
	uint8_t free_channel_check = 0;
	while(free_channel_check < NUM_PWM_CHANNELS) {

		//we have a free PWM channel
		if(!channel_in_use[free_channel_check]) {
			channel_mapping = free_channel_check;

			Hard_PWM::pwm_pins[channel_mapping] = &_pin; //save a pointer to the original pin
			Hard_PWM::channel_inverted[channel_mapping] = _inverted; //save whether the channel is inverted
			Hard_PWM::channel_in_use[channel_mapping] = true; //channel is now in use
			Hard_PWM::enable_chan_interrupt(channel_mapping); //this is basically here for dynamically constructed objects

			return; //done with the constructor, can leave
		}
		free_channel_check++;
	}

	//if we get here, there aren't any free channels
	channel_mapping = CHANNEL_NOT_MAPPED;
	return;
}

//should never be called, but writing this just in case
Hard_PWM::~Hard_PWM() {
	if(channel_mapping == CHANNEL_NOT_MAPPED) return; //if we didn't map the channel to hardware, just return
	//disable the interrupt channel
	Hard_PWM::disable_chan_interrupt(channel_mapping);
	//mark the pin as free
	Hard_PWM::channel_in_use[channel_mapping] = false;
	//and that's all we really need to do
}

//float from 0 to 1 inclusive
void Hard_PWM::set(float _pwm_val) {
	if(channel_mapping == CHANNEL_NOT_MAPPED) return; //quick sanity check if the channel is legit
	//sanity check PWM range
	if(_pwm_val < 0) return;
	if(_pwm_val > 1) return;

	//don't blank the channel but also don't pay attention to the compare interrupt
	//this is because the compare interrupt flag WILL STILL GET ASSERTED IF CCRx REG IS > ARR REG
	//page 565 of the datasheet describing the CC1IF bit
	//lol chatGPT helped me figure this one out
	if(_pwm_val == 1)
		Hard_PWM::disable_chan_interrupt(channel_mapping);

	else {
		//update the compare register for the appropriate timer/PWM channel and enable the ISR
		switch(channel_mapping) {
			case 0:
				PWM_A_TIM.Instance->CCR1 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_A_TIM.Instance->DIER |= TIM_DIER_CC1IE; //enable channel 1
				break;
			case 1:
				PWM_A_TIM.Instance->CCR2 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_A_TIM.Instance->DIER |= TIM_DIER_CC2IE; //enable channel 2
				break;
			case 2:
				PWM_A_TIM.Instance->CCR3 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_A_TIM.Instance->DIER |= TIM_DIER_CC3IE; //enable channel 3
				break;
			case 3:
				PWM_A_TIM.Instance->CCR4 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_A_TIM.Instance->DIER |= TIM_DIER_CC4IE; //enable channel 4
				break;
			case 4:
				PWM_B_TIM.Instance->CCR1 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_B_TIM.Instance->DIER |= TIM_DIER_CC1IE; //enable channel 5
				break;
			case 5:
				PWM_B_TIM.Instance->CCR2 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_B_TIM.Instance->DIER |= TIM_DIER_CC2IE; //enable channel 6
				break;
			case 6:
				PWM_B_TIM.Instance->CCR3 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_B_TIM.Instance->DIER |= TIM_DIER_CC3IE; //enable channel 7
				break;
			case 7:
				PWM_B_TIM.Instance->CCR4 = (uint32_t)(_pwm_val * PWM_RESOLUTION);
				PWM_B_TIM.Instance->DIER |= TIM_DIER_CC4IE; //enable channel 8
				break;
			default:
				break; //should never run but just in case
		}
	}

	//finally, unblank the channel (if applicable)
	Hard_PWM::blank_channel[channel_mapping] = false;
}

void Hard_PWM::operate_normally() {
	if(channel_mapping == CHANNEL_NOT_MAPPED) return; //quick sanity check if the channel is legit

	//un-blank the channel, and let the isr take care of setting up the pin after a single cycle
	Hard_PWM::blank_channel[channel_mapping] = false;

	//re-enable the corresponding ISR channel
	Hard_PWM::enable_chan_interrupt(channel_mapping);
}

void Hard_PWM::force_asserted() {
	if(channel_mapping == CHANNEL_NOT_MAPPED) return; //quick sanity check if the channel is legit

	//blank the pin first thing so we we have an ISR fire mid function, it won't update the pin
	Hard_PWM::blank_channel[channel_mapping] = true;

	//drive the pin according to whether the channel is inverted
	ASSERT(channel_mapping);

	//silence the corresponding ISR channel (so it doesn't trigger a useless ISR)
	Hard_PWM::disable_chan_interrupt(channel_mapping);
}

void Hard_PWM::force_deasserted() {
	if(channel_mapping == CHANNEL_NOT_MAPPED) return; //quick sanity check if the channel is legit

	//blank the pin first thing so we we have an ISR fire mid function, it won't update the pin
	Hard_PWM::blank_channel[channel_mapping] = true;

	//drive the pin according to whether the channel is inverted
	DEASSERT(channel_mapping);

	//silence the corresponding ISR channel (so it doesn't trigger a useless ISR)
	Hard_PWM::disable_chan_interrupt(channel_mapping);
}

void Hard_PWM::configure(const float _freq) {
	//call the initialization functions of both of the timers (ST HAL)
	PWM_A_INIT_FUNC();
	PWM_B_INIT_FUNC();

	//disable the timers when we're configuring everything
	PWM_A_TIM.Instance->CR1 &= ~(TIM_CR1_CEN);
	PWM_B_TIM.Instance->CR1 &= ~(TIM_CR1_CEN);

	//want the PWM resolution to be 100, set the frequency accordingly
	//for 1kHz PWM, prescaler registers should be 899
	PWM_A_TIM.Instance->PSC = ((uint32_t)((90000000.0/PWM_RESOLUTION)/_freq)) - 1;
	PWM_B_TIM.Instance->PSC = ((uint32_t)((90000000.0/PWM_RESOLUTION)/_freq)) - 1;

	//then set the counter auto reload register to 99 (counts from 0 to 99, 100 levels)
	PWM_A_TIM.Instance->ARR = PWM_RESOLUTION - 1;
	PWM_B_TIM.Instance->ARR = PWM_RESOLUTION - 1;

	//clear the interrupt status registers before moving forward just in case anything is set
	PWM_A_TIM.Instance->SR = 0;
	PWM_B_TIM.Instance->SR = 0;
	//start off by enabling compare interrupts for all active channels
	for(uint8_t i = 0; i < NUM_PWM_CHANNELS; i++) {
		if(Hard_PWM::channel_in_use[i])
			Hard_PWM::enable_chan_interrupt(i);
	}
	//configure the update/overflow interrupt in both of the PWM channels as well
	PWM_A_TIM.Instance->DIER |= TIM_DIER_UIE;
	PWM_B_TIM.Instance->DIER |= TIM_DIER_UIE;

	//reset the counters
	PWM_A_TIM.Instance->CNT = 0;
	PWM_B_TIM.Instance->CNT = 0;

	//configure the NVIC for the PWM channels, clear any pending interrupts
	HAL_NVIC_SetPriority(PWM_A_IRQn, (uint32_t)Priorities::HIGH, 0);
	HAL_NVIC_SetPriority(PWM_B_IRQn, (uint32_t)Priorities::HIGH, 0);
	HAL_NVIC_ClearPendingIRQ(PWM_A_IRQn);
	HAL_NVIC_ClearPendingIRQ(PWM_B_IRQn);
	//then enable the interrupt via the NVIC, and enable the timer
	HAL_NVIC_EnableIRQ(PWM_A_IRQn);
	HAL_NVIC_EnableIRQ(PWM_B_IRQn);
	PWM_A_TIM.Instance->CR1 |= TIM_CR1_CEN;
	PWM_B_TIM.Instance->CR1 |= TIM_CR1_CEN;
}

//================================ ISR HANDLING FUNCTIONS (class functions) ===============================

//aggressively optimize here since this will be called from timer ISRs
//splitting into two ISRs due to two separate channel groups of four
//THESE FUNCTIONS WILL NEVER BE CALLED FROM THE APP, SO CAN IMPLEMENT THE ISR
//HOWEVER YOU WANT BASED OFF OF WHAT'S BEST FOR YOUR HARDWARE
void __attribute__((optimize("O3"))) Hard_PWM::isr_groupA() {
	//read the timer interrupt flag register, checking against what interrupts were actually enabled
	uint32_t interrupt_status = PWM_A_TIM.Instance->SR & PWM_A_TIM.Instance->DIER;
	PWM_A_TIM.Instance->SR = 0; //clear all interrupt sources so we can pick another one up if it happens immediately

	//handle channel 0; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[0] && Hard_PWM::channel_in_use[0]) {
		if((interrupt_status & TIM_SR_CC1IF))
			DEASSERT(0); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(0); //we hit the normal update, assert the pin
	}

	//handle channel 1; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[1] && Hard_PWM::channel_in_use[1]) {
		if((interrupt_status & TIM_SR_CC2IF))
			DEASSERT(1); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(1); //we hit the normal update, assert the pin
	}

	//handle channel 2; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[2] && Hard_PWM::channel_in_use[2]) {
		if((interrupt_status & TIM_SR_CC3IF))
			DEASSERT(2); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(2); //we hit the normal update, assert the pin
	}

	//handle channel 3; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[3] && Hard_PWM::channel_in_use[3]) {
		if((interrupt_status & TIM_SR_CC4IF))
			DEASSERT(3); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(3); //we hit the normal update, assert the pin
	}
}

void __attribute__((optimize("O3"))) Hard_PWM::isr_groupB() {
	//read the timer interrupt flag register, checking against what interrupts were actually enabled
	uint32_t interrupt_status = PWM_B_TIM.Instance->SR & PWM_B_TIM.Instance->DIER;
	PWM_B_TIM.Instance->SR = 0; //clear all interrupt sources so we can pick another one up if it happens immediately

	//handle channel 4; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[4] && Hard_PWM::channel_in_use[4]) {
		if((interrupt_status & TIM_SR_CC1IF))
			DEASSERT(4); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(4); //we hit the normal update, assert the pin
	}

	//handle channel 5; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[5] && Hard_PWM::channel_in_use[5]) {
		if((interrupt_status & TIM_SR_CC2IF))
			DEASSERT(5); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(5); //we hit the normal update, assert the pin
	}

	//handle channel 6; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[6] && Hard_PWM::channel_in_use[6]) {
		if((interrupt_status & TIM_SR_CC3IF))
			DEASSERT(6); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(6); //we hit the normal update, assert the pin
	}

	//handle channel 7; first check if the channel is being actively controlled by the timer
	if(!Hard_PWM::blank_channel[7] && Hard_PWM::channel_in_use[7]) {
		if((interrupt_status & TIM_SR_CC4IF))
			DEASSERT(7); //if we hit the compare update (should also handle the case when PWM val is 0)
		else if(interrupt_status & TIM_SR_UIF)
			ASSERT(7); //we hit the normal update, assert the pin
	}
}

//=============================== PRIVATE FUNCTION DEFS ==========================
//enable the interrupt source by setting the corresponding bit in the approrpriate interrupt control reg
void Hard_PWM::enable_chan_interrupt(uint8_t pwm_channel) {
	switch(pwm_channel) {
		case 0:
			PWM_A_TIM.Instance->DIER |= TIM_DIER_CC1IE; //enable channel 1
			break;
		case 1:
			PWM_A_TIM.Instance->DIER |= TIM_DIER_CC2IE; //enable channel 2
			break;
		case 2:
			PWM_A_TIM.Instance->DIER |= TIM_DIER_CC3IE; //enable channel 3
			break;
		case 3:
			PWM_A_TIM.Instance->DIER |= TIM_DIER_CC4IE; //enable channel 4
			break;
		case 4:
			PWM_B_TIM.Instance->DIER |= TIM_DIER_CC1IE; //enable channel 5
			break;
		case 5:
			PWM_B_TIM.Instance->DIER |= TIM_DIER_CC2IE; //enable channel 6
			break;
		case 6:
			PWM_B_TIM.Instance->DIER |= TIM_DIER_CC3IE; //enable channel 7
			break;
		case 7:
			PWM_B_TIM.Instance->DIER |= TIM_DIER_CC4IE; //enable channel 8
			break;
		default:
			break; //should never run but just in case
	}
}

//disable the interrupt source by clearing the corresponding bit in the approrpriate interrupt control reg
void Hard_PWM::disable_chan_interrupt(uint8_t pwm_channel) {
	switch(pwm_channel) {
		case 0:
			PWM_A_TIM.Instance->DIER &= ~(TIM_DIER_CC1IE); //disable channel 1
			break;
		case 1:
			PWM_A_TIM.Instance->DIER &= ~(TIM_DIER_CC2IE); //disable channel 2
			break;
		case 2:
			PWM_A_TIM.Instance->DIER &= ~(TIM_DIER_CC3IE); //disable channel 3
			break;
		case 3:
			PWM_A_TIM.Instance->DIER &= ~(TIM_DIER_CC4IE); //disable channel 4
			break;
		case 4:
			PWM_B_TIM.Instance->DIER &= ~(TIM_DIER_CC1IE); //disable channel 5
			break;
		case 5:
			PWM_B_TIM.Instance->DIER &= ~(TIM_DIER_CC2IE); //disable channel 6
			break;
		case 6:
			PWM_B_TIM.Instance->DIER &= ~(TIM_DIER_CC3IE); //disable channel 7
			break;
		case 7:
			PWM_B_TIM.Instance->DIER &= ~(TIM_DIER_CC4IE); //disable channel 8
			break;
		default:
			break; //should never run but just in case
	}
}

//============================== ISRs (CALLED BY VECTOR TABLE) ================================
void PWM_A_IRQ_HANDLER(void) {
	//handle the ISR through the class function
	Hard_PWM::isr_groupA();
}

void PWM_B_IRQ_HANDLER(void) {
	//handle the ISR through the class function
	Hard_PWM::isr_groupB();
}
