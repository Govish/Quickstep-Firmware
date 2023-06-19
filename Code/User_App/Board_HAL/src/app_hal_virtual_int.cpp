/*
 * app_hal_virtual_int.cpp
 *
 *  Created on: May 30, 2023
 *      Author: Ishaan
 */

#include "app_hal_virtual_int.h"

//========================= VIRTUAL INTERRUPT IRQ MAPPINGS  ============================
#define VIRTUAL_INT_CHAN_0_IRQ_HANDLER		SPDIF_RX_IRQHandler //channel 0
#define VIRTUAL_INT_CHAN_1_IRQ_HANDLER		CEC_IRQHandler //channel 1

//=========================== INITIALIZING STAIC MEMBERS HERE ==========================
//initializing these empty callbacks for now, associate them with the proper callback funcs in the initializers
void empty_virtual_irq_handler();

//THIS IS HOW THE TIMER OBJECT MAPS TO THE PHSYICAL HARDWARE
//cannot conceive of using these two interrupt channels, so gonna make them useful here
const IRQn_Type Virtual_Int::hard_int_channels[] = {
		SPDIF_RX_IRQn,
		CEC_IRQn
};

//initialize the callback function array to just be emtpy handlers at the start
callback_function_t Virtual_Int::callbacks[] = {
		empty_virtual_irq_handler,
		empty_virtual_irq_handler
};

//======================= PUBLIC FUNCTION DEFINITIONS =========================
Virtual_Int::Virtual_Int(virtual_int_channel_t _channel): channel((int)_channel) {
	HAL_NVIC_EnableIRQ(Virtual_Int::hard_int_channels[channel]);
} //constructor

void Virtual_Int::set_callback_func(callback_function_t cb) const {
	//just store the pointer to the callback function in the array
	callbacks[channel] = cb;
}

void Virtual_Int::set_int_priority(int_priority_t prio) const {
	//setup the interrupt with the corresponding priority passed to the function
	HAL_NVIC_SetPriority(Virtual_Int::hard_int_channels[channel], (uint32_t)prio, 0);
}

void Virtual_Int::trigger() const {
	//trigger the interrupt by setting the interrupt to pending
	HAL_NVIC_SetPendingIRQ(Virtual_Int::hard_int_channels[channel]);
}

//================================== VIRTUAL INT CLASS INTERRUPT SERVICE ROUTINE ===================================

void Virtual_Int::ISR_func(int channel) {
	//run the callback function of the corresponding timer channel
	Virtual_Int::callbacks[channel]();
}

//======================================= VIRTUAL INT ISRs MAPPED TO VECTOR TABLE ===================================

void VIRTUAL_INT_CHAN_0_IRQ_HANDLER(void) {
	//service the ISR with the class on channel 0
	Virtual_Int::ISR_func(0);
}

void VIRTUAL_INT_CHAN_1_IRQ_HANDLER(void) {
	//service the ISR with the class on channel 1
	Virtual_Int::ISR_func(1);
}


void empty_virtual_irq_handler() {}
