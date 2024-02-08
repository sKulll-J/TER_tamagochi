/*
 * LED_MATRIX.c
 *
 *  Created on: Feb 2, 2024
 *      Author: G45
 */

#include<stdio.h>
#include"LED_MATRIX.h"
#include "WS2812B.h"
#include <stdint.h>

// Dimensions de la matrice
const uint8_t Matrice_largeur = 9;
const uint8_t Matrice_hauteur = 9;

uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;

  if( y & 0x01)
  {
    // lignes impaires : ordre inversé
    uint16_t reverseX = (Matrice_largeur - 1) - x;
    i = (y * Matrice_largeur) + reverseX;
  }
  else
  {
    // lignes paires : ordre normal
    i = (y * Matrice_largeur) + x;
  }

  return i;
}

void LEDMATRIX_on_xy(int X, int Y, int R, int G, int B, int L)
{
	Set_LED(XY(X,Y), R, G, B);
	Set_Brightness(L);
	WS2812_Send();
}

void LEDMATRIX_clear()
{
	Set_Brightness(0);
	WS2812_Send();
}
