#include <stdint.h> // uint8_t sinon il hurle
#include <math.h>   // NaN et inf
#include <Arduino.h>// random()

#include "terlib.h"
#include "pin.h"
#include "color.h"

struct game_s selector(struct game_s game_data, uint8_t input)
{
    // DECLARATIONS --------------------------------------------
    static uint8_t x = 4;              // position x absolue 9x9
    static uint8_t y = 4;              // position y absolue 9x9
    static uint8_t col_current;
    static uint8_t col_current_clair = COL_OWN_CLAIR;
    static uint8_t flag_init = 1;

    color_matching(game_data, &col_current, &col_current_clair);

    // INITIALISATION ------------------------------------------
    if (flag_init == 1) {
        x = random() % 8;
        y = random() % 8;
        
        game_data.printmatrix[x][y] = col_current;
        flag_init = 0;
    }

    // GAME LOOP -----------------------------------------------
    if (input > INPUT_A) {  // magie noire (INPUT_A = 0b0000 0001 donc ça ne prend en compte que les directions et B)
        
        game_data.printmatrix[x][y] = game_data.previous_printmatrix[x][y];    // sert à effacer l'ancienne position du selecteur
        
        if      (input == INPUT_L) { if (x > 0) x--; } 
        else if (input == INPUT_R) { if (x < 8) x++; }
        else if (input == INPUT_D) { if (y > 0) y--; }
        else if (input == INPUT_U) { if (y < 8) y++; }

        #if DEBUG
            Serial.print("x = ");
            Serial.println(x);
            Serial.print("y = ");
            Serial.println(y);
        #endif

        game_data.printmatrix[x][y] = COL_OWN;    // déplacement du curseur de sélection
        
    } else if (input == INPUT_A) {               
        game_data.current_player = !game_data.current_player; // inversion des joueurs
        color_matching(game_data, &col_current, &col_current_clair);
    }
    
    game_data.printmatrix[x][y] = col_current;
    return game_data;
}


// FONCTIONS ---------------------------------------------------
/* Cette fonction attribue la paire de couleur foncée/claire au tour actuel
 * ça sert à inverser les palettes quoi
 * ? j'en fait une fonction car ce bout de code apparait 2 fois : redondance = fonction dédiée
 */
void color_matching(struct game_s game_data, uint8_t *col_current, uint8_t *col_current_clair)
{
    if (game_data.current_player == PLAYER1) {
        *col_current = COL_OWN;
        *col_current_clair = COL_OWN_CLAIR;
    } else if (game_data.current_player == PLAYER2){
        *col_current = COL_OPP;
        *col_current_clair = COL_OPP_CLAIR;
    }
}
