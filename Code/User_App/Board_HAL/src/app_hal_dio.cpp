/*
 * app_hal_dio.cpp
 *
 *  Created on: Mar 10, 2023
 *      Author: Ishaan
 */

#include "app_hal_dio.h"
extern "C" {
	#include "gpio.h"
}

#define BSRR_BASE_REG		0x40020018UL
#define IDR_BASE_REG		0x40020010UL
#define CLEAR_DATA_OFFSET 	16
#define SET_DATA_OFFSET 	0

DIO::DIO(const dio_pin_t &pin_name):
		pin_ref(pin_name),
		DRIVE_HIGH_MASK(1 << (pin_name.pin + SET_DATA_OFFSET)),
		DRIVE_LOW_MASK(1 << (pin_name.pin + CLEAR_DATA_OFFSET)),
		READ_MASK(1 << pin_name.pin),
		port_BSRR((volatile uint32_t*) (BSRR_BASE_REG + (uint32_t)pin_name.port)),
		port_IDR((volatile uint32_t*) (IDR_BASE_REG + (uint32_t)pin_name.port))
{
	//the bit set/reset register corresponds to the BSRR base register plus the port offset
	//encode the port offset into the port enumeration
	//same thing goes with the input data register
}

void DIO::init() {
	MX_GPIO_Init();
}

void DIO::set() const {
	*port_BSRR = DRIVE_HIGH_MASK;
}

void DIO::clear() const {
	*port_BSRR = DRIVE_LOW_MASK;
}

uint32_t DIO::read() const {
	return ( (*port_IDR) & READ_MASK );
}



