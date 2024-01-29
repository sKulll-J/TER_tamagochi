#include <stdint.h> // uint8_t sinon il hurle
#include <stdlib.h> // rand()
#include <time.h>   // time()

#include "terlib.h"

game_t snake(game_t game_data, uint8_t input)
{

    // go liste chainée pour le déplacement
    //snake[index][x][y]
    //static uint8_t colmatrix[9][9];
    static uint8_t headpos = 0x44; //position au milieux
    static uint8_t dir = 0x01; //un bits pour VERTICAL/HORIZONTAL un bit pour +/-
     
    //mange fruit => rajoute un element à la liste
    for(int i=0;i<9;i++)
    {
        for(int j=0;j<9; j++)
        {   
            game_data.printmatrix[i][j]=LED_NOIR;
        }
    }
    game_data.printmatrix[headpos& 0x0F][headpos>>4]=PLAYER2; //set la tete du serpent
    
/*
    switch case droite gauche haut bas

    
*/


    return game_data;
}