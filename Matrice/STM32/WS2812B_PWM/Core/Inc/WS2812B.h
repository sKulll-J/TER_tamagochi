/*
 * WS2812B.h
 *
 *  Created on: Feb 1, 2024
 *      Author: Sluggcat
 */


#ifndef INC_WS2812B_H_
#define INC_WS2812B_H_
#include<stdio.h>

#define PI 3.14159265

#define MAX_LED 81 // définir le nombre de LED en série içi
#define USE_BRIGHTNESS 1

void Set_LED (int LED_id, int R, int G, int B);
void Set_Brightness (int brightness);

void WS2812_Send (void);

void LED_test_1();
void LED_test_0();
void LED_test_2();

#endif /* INC_WS2812B_H_ */
