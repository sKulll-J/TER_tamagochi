#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>

#include "terlib.h"


// je les met là pour le moment
game_t tron(game_t game, uint8_t input)
{
    input++;
    return game;
}

game_t fanorona(game_t game, uint8_t input)
{
    input++;
    return game;
}

// Gestion input
uint8_t readinput(void)
{
    uint8_t data_input = 0;

    data_input |= (digitalRead(PIN_A) & INPUT_A);
    data_input |= (digitalRead(PIN_B) & INPUT_B);
    data_input |= (digitalRead(PIN_UP) & INPUT_UP);
    data_input |= (digitalRead(PIN_DOWN) & INPUT_DOWN);
    data_input |= (digitalRead(PIN_LEFT) & INPUT_LEFT);
    data_input |= (digitalRead(PIN_RIGHT) & INPUT_RIGHT);

    if (data_input == 0) data_input = MAGIC_NO_INPUT;
    
    return data_input;
}

uint8_t parseinput(void)
{
    return 1;
}