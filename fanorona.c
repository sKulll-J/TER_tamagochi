#include <stdio.h>
#include <stdint.h>
#include "terlib.h"

#define OWN 1
#define OPPS 2
#define NOT_SELECTED 0
#define SELECTED 1


typedef struct 
{
    uint8_t  x; // Coordonnée x du pion
    uint8_t y; // Coordonnée y du pion
} Pion;

typedef struct 
{
    uint8_t dx; // Composante x du vecteur
    uint8_t dy; // Composante y du vecteur
} Vecteur;

// ---------------------------------------------------------------- 

/*
    FONCTIONS NECESSAIRES
        - initialiser le plateau de jeu : à faire
        - vérifier victoire : ok
        - vérifier entourage d'un pion : ok

        - selectionner/déselectionner un pion : en cours
            - confirmer déplacement : à faire

        - éliminations : en cours
            - calculer vecteur déplacement : ok
            - aspiration : à faire
            - percussion : à faire

*/

game_t fanorona(game_t game_data, uint8_t input) 
{
    // DECLARATIONS --------------------------------------------
    static uint8_t pion[5][9] = {0};    // grille de jeu
    static uint8_t x = 5;
    static uint8_t y = 3;
    static uint8_t flag_pion = NOT_SELECTED;


    // GAME LOOP -----------------------------------------------
    if (flag_pion == NOT_SELECTED) { // mode selection du pion à bouger
        switch (input) {
            case INPUT_LEFT:  if (x > 0) x--; break;
            case INPUT_RIGHT: if (x < 9) x++; break;
            case INPUT_DOWN:  if (y > 0) y--; break;
            case INPUT_UP:    if (y < 5) y++; break;
            case INPUT_A: 
                if (game_data.current_player == pion[y][x]) {   // si le joueur a selectionné un de ses pions
                    flag_pion = SELECTED; 
                } else {
                    // joue un petit "bip" (si on ajoute un système audio)
                }
                break;
            case INPUT_B: break; // rien

        }
    } else if (flag_pion == SELECTED) { // mode déplacement du pion sélectionné
        switch (input) {
            case INPUT_LEFT:  if (x > 0) x--; break;
            case INPUT_RIGHT: if (x < 9) x++; break;
            case INPUT_DOWN:  if (y > 0) y--; break;
            case INPUT_UP:    if (y < 5) y++; break;
            case INPUT_A: break; // rien
            case INPUT_B: 
            
            break;

        }

        // affichage du curseur (blanc sur du vide)
        switch (plateau[y][x]) {
            case OWN:  game_data.printmatrix[y][x] = COL_OWN_CLAIR;  break;
            case OPPS: game_data.printmatrix[y][x] = COL_OPPS_CLAIR; break;
            case 0:    game_data.printmatrix[y][x] = COL_BLANC;      break;
                //! pas sur du [y][x]
        }

    }

    return game_data;
}
















