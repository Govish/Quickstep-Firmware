/*
 * app_main.h
 *
 *  Created on: Mar 10, 2023
 *      Author: Ishaan
 */

#ifndef USER_APP_APP_MAIN_H_
#define USER_APP_APP_MAIN_H_

//have to define these as C functions so they can be called from main.c
#ifdef __cplusplus
extern "C"
{
#endif

void app_init();
void app_loop();

#ifdef __cplusplus
}
#endif

#endif /* USER_APP_APP_MAIN_H_ */
