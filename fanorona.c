#include <stdio.h>
#include <stdint.h>

#include "terlib.h"
#include "color.h"

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
    static uint8_t plateau[5][9] = {0};    // grille de jeu
    static uint8_t x = 5;
    static uint8_t y = 3;
    static uint8_t flag_pion = NOT_SELECTED;


    // GAME LOOP -----------------------------------------------
    if (flag_pion == NOT_SELECTED) { // mode selection du pion à bouger
        switch (input) {
            case INPUT_L:  if (x > 0) x--; break;
            case INPUT_R: if (x < 9) x++; break;
            case INPUT_D:  if (y > 0) y--; break;
            case INPUT_U:    if (y < 5) y++; break;
            case INPUT_A: 
                if (game_data.current_player == plateau[y][x]) {   // si le joueur a selectionné un de ses pions
                    flag_pion = SELECTED; 
                } else {
                    // joue un petit "bip" (si on ajoute un système audio)
                }
                break;
            case INPUT_B: break; // rien

        }
    } else if (flag_pion == SELECTED) { // mode déplacement du pion sélectionné
        switch (input) {
            case INPUT_L:  if (x > 0) x--; break;
            case INPUT_R: if (x < 9) x++; break;
            case INPUT_D:  if (y > 0) y--; break;
            case INPUT_U:    if (y < 5) y++; break;
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


// FONCTIONS ---------------------------------------------------
/*  
    Fonction pour initialiser le plateau : allume les leds de chaque joueur.
*/
void plateau_init()
{


}

/*  
    Cette fonction scrute le plateau de jeu, vérifie les pions restants et détermine le vainqueur.
*/
uint8_t check_win(uint8_t plateau[5][9])
{   
    int pion_J1 = 0; // Variable pour détecter un pion du J1
    int pion_J2 = 0; // Variable pour détecter un pion du J2
    
    for(int i = 0; i<5 ; i++)
    {
        for(int j = 0 ; j<9 ; j++)
        {
            if (plateau[i][j] == 1)    pion_J1 = 1 ;
            else if (plateau[i][j] == 2)    pion_J2 = 1 ;
        }
    }

    // Vérification
    if (pion_J1 && !pion_J2) 
    {
        return 1; // Joueur 1 gagne
    } 
    
    else if (!pion_J1 && pion_J2) 
    {
        return 2; // Joueur 2 gagne
    } 
    
    else
    {
        return 0; // La partie continue
    } 
    
}

/*  
    Cette fonction vérifie l'entourage du pion sélectionné.
*/
void check_entourage(uint8_t plateau[5][9], uint8_t entourage[5][5], Pion pion) 
{
    // Parcours de l'entourage 5x5 autour du pion sélectionné
    for (int i = pion.x - 2, x_entourage = 0; i <= pion.x + 2; i++, x_entourage++) 
    {
        for (int j = pion.y - 2, y_entourage = 0; j <= pion.y + 2; j++, y_entourage++) 
        {
            // Vérification des limites du plateau
            if (i >= 0 && i < 5 && j >= 0 && j < 9) 
            {
                // Si la case correspond au pion sélectionné, copier sa valeur depuis le plateau
                if (i == pion.x && j == pion.y) 
                {
                    entourage[x_entourage][y_entourage] = plateau[pion.x][pion.y];
                }
                // Sinon, copier la valeur de la case du plateau dans l'entourage
                else 
                {
                    entourage[x_entourage][y_entourage] = plateau[i][j];
                }
            }

            // Si la case est en dehors du plateau, la considérer comme vide (0)
            else 
            {
                entourage[x_entourage][y_entourage] = 0;
            }
        }
    }
}

/*  
    Cette fonction calcule le vecteur déplacement du pion joué.
*/
Vecteur compute_vector(Pion pion_avant, Pion pion_apres)
{
    Vecteur deplacement;

    // Calcul des composantes du vecteur de déplacement
    deplacement.dx = pion_apres.x - pion_avant.x;
    deplacement.dy = pion_apres.y - pion_avant.y;

    return deplacement;
}

/*
    Fonction pour gérer les éliminations
*/
void kill(uint8_t plateau[5][9], uint8_t entourage[5][5], Vecteur deplacement, Pion pion_now)
{

}
