#include <stdint.h> // uint8_t sinon il hurle
#include <stdlib.h>
#include <Arduino.h> // millis

#include "terlib.h"

game_t tron(game_t game_data, uint8_t owninput, uint8_t oppsinput)
{
    owninput++;
    oppsinput++;
    game_data.printmatrix[5][3] = COL_BLANC;
    
    return game_data;
}
