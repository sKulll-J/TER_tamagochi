#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

//#include <FastLED.h>
//#include "ter-megamorpion.h"

#include "terlib.h"

/* fonctions nécessaires 
    - fonction jeu prend en paramètre tout les inputs et plus tard les inputs adverses
        -> struct input_t terinput et input_t teropps_input (ive got opps everywhere)
    - fonction de jeu return une struct game_s qui contient l'état de la partie (en cours/terminée), le winner, et la matrice à afficher

    - gestion d'erreur si aucun adversaire n'est connecté (clignotement rouge ou whatnot)
    - utilisation de registre pour utiliser moins de place (verifier si le code de bitshift prend plus de place que faire avec des uint normaux mais je pense pas)

    
    - déplacement du curseur (couleur blanche pour distinguer des "x/o" déjà placés)
    - update et draw les leds en accord avec la xomatrix
    - check win (scalable sur une minigrid et une megagrid)



*/



game_t g_megamorpion(input_t terinput, input_t oppsinput)
{
    // DECLARATIONS --------------------------------------------
    game_t megamorpion = {

    };

    //? pas sur de ça 
    /** Registre de position regxy
     * xxxx xxxx
     * ^ X   ^Y
     * 4 bits MSB = x
     * 4 bits LSB = Y
     * 2^4 = 16 => on pourrait utiliser une matrice max 16x16
     */
    //uint8_t regxy;

    srand(time(NULL));

    uint8_t x = rand() % 9; // position x du curseur sur l'écran
    uint8_t y = rand() % 9; // position y du curseur sur l'écran
    uint8_t prev_x = x;     // précédente position x du curseur
    uint8_t prev_y = y;     // précédente position y du curseur

    bool current_player = rand() % 2;  // J1 = own = 0, J2 = opps = 1 - à changer plus tard lors de la communication et qui joue en premier

    // INITIALISATION ------------------------------------------




    // GAME LOOP -----------------------------------------------


    return megamorpion;
}