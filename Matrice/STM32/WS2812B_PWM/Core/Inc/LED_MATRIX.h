/*
 * LED_MATRIX.h
 *
 *  Created on: Feb 2, 2024
 *      Author: G45
 */
#include <stdint.h>
#ifndef INC_LED_MATRIX_H_
#define INC_LED_MATRIX_H_



/* Matrice disposée en serpentin
    0 >  1 >  2 >  3 >  4
                        =
    9 <  8 <  7 <  6 <  5
    =
    10 > 11 > 12 > 13 > 14
                         =
    19 < 18 < 17 < 16 < 15
*/

#define LEDS_nb (kMatrixWidth * kMatrixHeight)


uint16_t XY( uint8_t x, uint8_t y);


void LEDMATRIX_on_xy(int X, int Y, int R, int G, int B,int L);

#endif /* INC_LED_MATRIX_H_ */
