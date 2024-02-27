#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>

#include "terlib.h"

// Gestion input
uint8_t readinput(void)
{
    uint8_t data_input = 0;

    data_input |= (digitalRead(PIN_A) & INPUT_A);
    data_input |= (digitalRead(PIN_B) << 1 & INPUT_B);
    data_input |= (digitalRead(PIN_LEFT) << 2 & INPUT_LEFT);
    data_input |= (digitalRead(PIN_RIGHT) << 3 & INPUT_RIGHT);
    data_input |= (digitalRead(PIN_DOWN) << 4 & INPUT_DOWN);
    data_input |= (digitalRead(PIN_UP) << 5 & INPUT_UP);

    if (data_input == 0) return 0;  // renvoie 0 si aucun input n'est inputé (lol)

    return 63-data_input; //par défaut a 63 soit `0b0011 1111` -> lorsqu'un bouton est cliqué, son bit de ref passe a zero -> return inversion
}

uint8_t parseinput(void)
{
    return 1;
}