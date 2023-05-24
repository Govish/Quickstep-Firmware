/*
 * app_pin_mapping.cpp
 *
 *  Created on: Mar 11, 2023
 *      Author: Ishaan
 */

#include "app_pin_mapping.h"

//====================== BEGIN PIN MAPPING DEFINITIONS ==========================

const dio_pin_t PinMap::status_led = {PORT_A, 5}; //port bit configuration for LED (xx corresponds to speed value): 010xx00
const dio_pin_t PinMap::user_button = {PORT_C, 13};
const dio_pin_t PinMap::red_led = {PORT_A, 10};
const dio_pin_t PinMap::yellow_led = {PORT_B, 3};
const dio_pin_t PinMap::green_led = {PORT_B, 5};
const dio_pin_t PinMap::x_mot_step = {PORT_C, 7};
const dio_pin_t PinMap::x_mot_dir = {PORT_A, 9};
const dio_pin_t PinMap::x_mot_en = {PORT_B, 6};
const dio_pin_t PinMap::y_mot_step = {PORT_B, 10};
const dio_pin_t PinMap::y_mot_dir = {PORT_A, 8};
const dio_pin_t PinMap::y_mot_en = {PORT_B, 4};

//====================== END PIN MAPPING DEFINITIONS ============================

