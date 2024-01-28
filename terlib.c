#include <stdlib.h>
#include <stdint.h>

#include "terlib.h"

void key_unused()
{
    //wrapper->gb->running = 0;
}

void key_a()
{

}

void key_b()
{
    
}

void key_up()
{
    
}

void key_down()
{
   
}

void key_left()
{
  
}

void key_right()
{
    
}

void (*input_functions[INPUT_COUNT])();

void init_input_functions(void)
{
    for (uint8_t i=0; i<INPUT_COUNT; i++)
        input_functions[i] = &key_unused;

    input_functions[INPUT_A] = &key_a;
    input_functions[INPUT_B] = &key_b;
    input_functions[INPUT_UP] = &key_up;
    input_functions[INPUT_DOWN] = &key_down;
    input_functions[INPUT_LEFT] = &key_left;
    input_functions[INPUT_RIGHT] = &key_right;   
}

// je les met là pour le moment
game_t snake(game_t game, uint8_t input){}
game_t tron(game_t game, uint8_t input){}
game_t fanorona(game_t game, uint8_t input){}