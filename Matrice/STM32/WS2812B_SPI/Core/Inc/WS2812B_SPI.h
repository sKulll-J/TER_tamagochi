/*
 * WS2812B_SPI.h
 *
 *  Created on: Feb 8, 2024
 *      Author: Sluggcat
 */



#ifndef INC_WS2812B_SPI_H_
#define INC_WS2812B_SPI_H_

#define NUM_LED 81


#define USE_BRIGHTNESS 1

void LED_set (int LED_id, int R, int G, int B);
void WS2812_Send_Spi (int G, int R, int B);
void WS2812_Send();

#endif /* INC_WS2812B_SPI_H_ */
