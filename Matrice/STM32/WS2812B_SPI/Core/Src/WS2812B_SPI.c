/*
 * WS2812B_SPI.c
 *
 *  Created on: Feb 8, 2024
 *      Author: Sluggcat
 */

#include<stdio.h>
#include"WS2812B_SPI.h"
#include "stm32l4xx_hal.h"
#include "spi.h"

uint8_t LED_Data[NUM_LED][4];
int brightness = 50 ;

void LED_set (int LED_id, int R, int G, int B)
{
	LED_Data[LED_id][0] = LED_id;
	LED_Data[LED_id][1] = G;
	LED_Data[LED_id][2] = R;
	LED_Data[LED_id][3] = B;
}

void WS2812_Send_Spi (int G, int R, int B)
{
#if USE_BRIGHTNESS
	if (brightness>100)brightness = 100;
	G = G*brightness/100;
	R = R*brightness/100;
	B = B*brightness/100;
#endif
	uint32_t color = G<<16 | R<<8 | B;
	uint8_t sendData[24];
	int indx = 0;

	for (int i=23; i>=0; i--)
	{
		if (((color>>i)&0x01) == 1) sendData[indx++] = 0b110;  // store 1
		else sendData[indx++] = 0b100;  // store 0
	}

	HAL_SPI_Transmit(&hspi2,sendData, 24, 1000);
}

void WS2812_Send()
{
	for (int i=0; i<NUM_LED; i++)
	{
		WS2812_Send_Spi(LED_Data[i][1],LED_Data[i][2],LED_Data[i][3]);
	}
	HAL_Delay (1);
}

void test_0()
{
	 	 for (int i=0; i<81; i++)
		  {
			 LED_set(i, 255, 0, 0);
		  }
		  WS2812_Send(5);
		  HAL_Delay(1000);

		  for (int i=0; i<81; i++)
		  {
			 LED_set(i, 0, 255, 0);
		  }
		  WS2812_Send(5);
		  HAL_Delay(1000);

		  for (int i=0; i<81; i++)
		  {
			 LED_set(i, 0, 0, 255);
		  }
		  WS2812_Send(5);
		  HAL_Delay(1000);
}
