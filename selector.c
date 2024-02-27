#include <stdint.h> // uint8_t sinon il hurle
#include <math.h>   // NaN et inf
#include <Arduino.h>// random()

#include "terlib.h"

void color_matching(game_t game_data, uint8_t *col_current, uint8_t *col_current_clair);

game_t selector(game_t game_data, uint8_t input)
{
    // DECLARATIONS --------------------------------------------
    static uint8_t x = 5;              // position x absolue 9x9
    static uint8_t y = 5;              // position y absolue 9x9
    static uint8_t col_current = COL_OWN;
    static uint8_t col_current_clair = COL_OWN_CLAIR;
    static uint8_t flag_init = 1;

    // INITIALISATION ------------------------------------------
    if (flag_init == 1) {
        game_data.printmatrix[x][y] = col_current;
        flag_init = 0;
    }

    // GAME LOOP -----------------------------------------------
    if (input > INPUT_A) {  // magie noire (INPUT_A = 0b0000 0001 donc ça ne prend en compte que les directions et B)
        game_data.printmatrix[x][y] = game_data.previous_printmatrix[x][y];    // sert à effacer l'ancienne position du selecteur
        
        if      (input == INPUT_LEFT)  { if (x > 0) x--; } 
        else if (input == INPUT_RIGHT) { if (x < 8) x++; }
        else if (input == INPUT_DOWN)  { if (y > 0) y--; }
        else if (input == INPUT_UP)    { if (y < 8) y++; }

        game_data.printmatrix[x][y] = col_current;    // déplacement du curseur de sélection
        
    } else if (input == INPUT_A) {               
            game_data.current_player = !game_data.current_player;               // inversion des joueurs
            color_matching(game_data, &col_current, &col_current_clair);        
    }
    
    return game_data;
}


// FONCTIONS ---------------------------------------------------
/* Cette fonction attribue la paire de couleur foncée/claire au tour actuel
 * ça sert à inverser les palettes quoi
 * ? j'en fait une fonction car ce bout de code apparait 2 fois : redondance = fonction dédiée
 */
void color_matching(game_t game_data, uint8_t *col_current, uint8_t *col_current_clair)
{
    if (game_data.current_player == PLAYER1) {
        *col_current = COL_OWN;
        *col_current_clair = COL_OWN_CLAIR;
    } else if (game_data.current_player == PLAYER2){
        *col_current = COL_OPPS;
        *col_current_clair = COL_OPPS_CLAIR;
    }
}