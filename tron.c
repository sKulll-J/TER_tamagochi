#include <stdint.h> // uint8_t sinon il hurle
#include <stdlib.h>
#include <Arduino.h> // millis

#include "terlib.h"
#include "color.h"

struct game_s tron(struct game_s game_data, uint8_t input)
{
    input++;
    game_data.printmatrix[5][3] = COL_BLANC;
    
    return game_data;
}
