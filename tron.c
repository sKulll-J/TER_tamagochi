#include <stdint.h> // uint8_t sinon il hurle

#include "terlib.h"

#define UP 1
#define DOWN 2
#define RIGHT 4
#define LEFT 8

void newPos(uint8_t * pos, uint8_t input);

game_t tron(game_t game_data, uint8_t input) 
{
    static uint8_t pos_1 = 0x34;
    static uint8_t pos_2 = 0x54; 

    for (uint8_t i=0; i<9; i++) {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[i][j] = COL_NOIR;
        }
    }

    if(game_data.current_player==PLAYER1)
    {
        newPos(&pos_1, input);
    }
    else if(game_data.current_player==PLAYER2)
    {
        newPos(&pos_2, input);
    }

    game_data.printmatrix[pos_1>>4][pos_1&0x0F] = COL_OWN;
    game_data.printmatrix[pos_2>>4][pos_2&0x0F] = COL_OPPS;



    return game_data;
}


void newPos(uint8_t * pos, uint8_t input)
{
    int x,y;
    x= ((*pos)>>4);
    y= ((*pos)&0x0F);
    switch(input)
    {
        case UP : (y)+=1;break;
        case DOWN : (y)-=1;break;
        case LEFT : (x)+=1;break;
        case RIGHT : (x)-=1;break;
        default : break;
    }
    
    if(x>=9)
        x=0;
    if(x<0)
        x=8;
    if(y>=9)
        y=0;
    if(y<0)
        y=8;

    (*pos) = y+(x<<4); 
}