/*
 * WS2812.c
 *
 *  Created on: Feb 1, 2024
 *      Author: Sluggcat
 */
#include<stdio.h>
#include <math.h>
#include "WS2812B.h"
#include "tim.h"


uint8_t LED_Data[MAX_LED][4];

uint8_t LED_Mod[MAX_LED][4];

int FLAG_DataSent = 0 ;

/**
 * @brief Ciblage série d'une LED.
 * @param[in] LEDnum : identifiant de la led (rang sur la ligne)
 * @param[in] Red, Green, Blue : Couleurs RGB
 */
void Set_LED (int LED_id, int R, int G, int B)
{
	LED_Data[LED_id][0] = LED_id;
	LED_Data[LED_id][1] = G;
	LED_Data[LED_id][2] = R;
	LED_Data[LED_id][3] = B;
}


/**
 * @brief Fonction pour configurer  la luminosité. Utilise une fonction pseudo-linéaire pour ajuster la luminosité.
 * @param[in] brightness : luminosité souhaitée entre [0-45]
 */
void Set_Brightness (int brightness)  // 0-45
{
#if USE_BRIGHTNESS

	if (brightness > 45) brightness = 45;
	for (int i=0; i<MAX_LED; i++)
	{
		LED_Mod[i][0] = LED_Data[i][0];
		for (int j=1; j<4; j++)
		{
			float angle = 90-brightness;  // en degrés
			angle = angle*PI / 180;  // en rad
			LED_Mod[i][j] = (LED_Data[i][j])/(tan(angle));
		}
	}

#endif

}


uint16_t pwmData[(24*MAX_LED)+50]; // 3 octets pour couleurs RGB + 50 bits de reset par LED

/**
 * @brief Fonction d'envoi de la commande. Convertit les commandes en signaux PWM
 */
void WS2812_Send (void)
{
	uint32_t indx=0;
	uint32_t color;

	for (int i= 0; i<MAX_LED; i++)
	{
		#if USE_BRIGHTNESS
			color = ((LED_Mod[i][1]<<16) | (LED_Mod[i][2]<<8) | (LED_Mod[i][3]));
		#else
			color = ((LED_Data[i][1]<<16) | (LED_Data[i][2]<<8) | (LED_Data[i][3]));
		#endif

		for (int i=23; i>=0; i--)
		{
			if (color&(1<<i))
			{
				pwmData[indx] = 60;  // 2/3 de 90 (2/3 du rapport cyclique) => envoi d'un 1 logique (cf.datasheet)
			}

			else pwmData[indx] = 30;  // 1/3 de 90 (1/3 du rapport cyclique) => envoi d'un 0 logique (cf.datasheet)

			indx++;
		}

	}


	for (int i=0; i<50; i++) // 50 zéros pour les 50 us de reset (cf.datasheet)
	{
		pwmData[indx] = 0;
		indx++;
	}

	//HAL_Delay(100); // délai de reset, échec : débordement de la trame de commande sur LED suivante
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)pwmData, indx);
	while (!FLAG_DataSent){};
	FLAG_DataSent = 0;
}

/**
 * @brief Fonction de routine, pour arrêter la DMA et la génération PWM en fin de transmission
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
	FLAG_DataSent=1;
}

void LED_test_0()
{
	//Set_LED(1,255,255,255);
	//Set_LED(0,255,255,255);
	//Set_LED(80,255,0,0);
	Set_Brightness(5);

	for(int i=0; i<=MAX_LED ; i++)
	{
		Set_LED(i, 255, 255, 255);
	}
	Set_Brightness(45);
	WS2812_Send();

	//HAL_Delay(500);
}

void LED_test_1()
{
	  for(int i = 0 ; i<=MAX_LED ; i++)
	  {
		  Set_LED(i, 225-2*i, 100+1.5*i, 100-5*i);
	  }

	  for (int i=0; i<10; i++)
	  {
		  Set_Brightness(i);
		  WS2812_Send();
		  HAL_Delay(100);
	  }

	  for (int i=10; i>=0; i--)
	  {
		  Set_Brightness(i);
		  WS2812_Send();
		  HAL_Delay(50);
	  }

}

void LED_test_2() // refresh test
{
	for(int i = 0 ; i<=MAX_LED ; i++)
	{
		Set_LED(i, 255, 0, 255);
	}
	Set_Brightness(5);

	WS2812_Send();


	  for(int i = 0 ; i<=MAX_LED ; i++)
	  {
		  Set_LED(i, 0, 0, 0);

	  }
	WS2812_Send();

}
