#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "terlib.h"
#include "color.h"

#define OWN 1
#define OPP 2
#define NOT_SELECTED 0
#define SELECTED 1

typedef struct {
    uint8_t x; // Coordonnée x du pion
    uint8_t y; // Coordonnée y du pion
} Pion;

typedef struct {
    uint8_t dx; // Composante x du vecteur
    uint8_t dy; // Composante y du vecteur
} Vecteur;

// ---------------------------------------------------------------- 

/*
    FONCTIONS NECESSAIRES
        - initialiser le plateau de jeu : ok
        - vérifier victoire : ok
        - vérifier entourage d'un pion : ok

        - selectionner/déselectionner un pion : en cours
            - déplacement : à tester

        - éliminations : à tester
            - calculer vecteur déplacement : ok

    TO DO : 
    - coder la gameloop 
    - tests
*/

struct game_s fanorona(struct game_s game_data, uint8_t input) 
{
    // DECLARATIONS --------------------------------------------
    static uint8_t plateau[5][9] = {0};    // grille de jeu
    static uint8_t x = 5;
    static uint8_t y = 3;
    static uint8_t flag_pion = NOT_SELECTED;


    // GAME LOOP -----------------------------------------------
    if (flag_pion == NOT_SELECTED) { // mode selection du pion à bouger
        switch (input) {
            case INPUT_L: if (x > 0) x--; break;
            case INPUT_R: if (x < 9) x++; break;
            case INPUT_D: if (y > 0) y--; break;
            case INPUT_U: if (y < 5) y++; break;
            case INPUT_A: 
                if (game_data.current_player == plateau[y][x]) {   // si le joueur a selectionné un de ses pions
                    flag_pion = SELECTED; 
                } else {
                    // joue un petit "bip" (si on ajoute un système audio)
                }
                break;
            case INPUT_B: break; // rien
            default: break;
        }
    } else if (flag_pion == SELECTED) { // mode déplacement du pion sélectionné
        switch (input) {
            case INPUT_L: if (x > 0) x--; break;
            case INPUT_R: if (x < 9) x++; break;
            case INPUT_D: if (y > 0) y--; break;
            case INPUT_U: if (y < 5) y++; break;
            case INPUT_A: break; // rien
            case INPUT_B: break;
            default: break;
        }

        // affichage du curseur (blanc sur du vide)
        switch (plateau[y][x]) {
            case OWN:  game_data.printmatrix[y][x] = COL_OWN_CLAIR; break;
            case OPP:  game_data.printmatrix[y][x] = COL_OPP_CLAIR; break;
            case 0:    game_data.printmatrix[y][x] = COL_BLANC;     break;
                //! pas sur du [y][x]
        }
    }

    return game_data;
}


// FONCTIONS ---------------------------------------------------
/*  
    Fonction pour initialiser le plateau : pour allumer les leds de chaque joueur.
*/
void plateau_init(uint8_t plateau[5][9])
{
    for(int i = 0 ; i<5 ; i++) {
        for(int j = 0 ; j<9 ; j++) {
            if(i == 0 | i == 1) plateau[i][j] = 2;
            else if(i == 3 | i == 4) plateau[i][j] = 1;
            else if(i==3) {
                if(j==0|j==2|j==5|j==7) plateau[i][j] = 2;
                if(j==1|j==3|j==6|j==8) plateau[i][j] = 1;
            } else plateau[i][j] = 0 ;
        }
    }
}

/*  
    Cette fonction scrute le plateau de jeu, vérifie les pions restants et détermine le vainqueur.
    Victoire par anihilation. 
    Immobilisation non traitée ...
*/
uint8_t check_win(uint8_t plateau[5][9])
{   
    int pion_J1 = 0; // Variable pour détecter un pion du J1
    int pion_J2 = 0; // Variable pour détecter un pion du J2
    
    for(int i = 0; i<5 ; i++) {
        for(int j = 0 ; j<9 ; j++) {
            if      (plateau[i][j] == 1) pion_J1 = 1;
            else if (plateau[i][j] == 2) pion_J2 = 1;
        }
    }

    // Vérification
    if (pion_J1 && !pion_J2) {
        return 1; // Joueur 1 gagne
    } else if (!pion_J1 && pion_J2) {
        return 2; // Joueur 2 gagne
    } else {
        return 0; // La partie continue
    } 
    
}

/*  
    Cette fonction vérifie l'entourage du pion sélectionné.
*/
void check_entourage(uint8_t plateau[5][9], uint8_t entourage[5][5], Pion pion) 
{
    // Parcours de l'entourage 5x5 autour du pion sélectionné
    for (int i = pion.x - 2, x_entourage = 0; i <= pion.x + 2; i++, x_entourage++) {   
        for (int j = pion.y - 2, y_entourage = 0; j <= pion.y + 2; j++, y_entourage++) {
            // Vérification des limites du plateau
            if (i >= 0 && i < 5 && j >= 0 && j < 9) {
                // Si la case correspond au pion sélectionné, copier sa valeur depuis le plateau
                if (i == pion.x && j == pion.y) {
                    entourage[x_entourage][y_entourage] = plateau[pion.x][pion.y];
                }
                // Sinon, copier la valeur de la case du plateau dans l'entourage
                else {
                    entourage[x_entourage][y_entourage] = plateau[i][j];
                }
            }

            // Si la case est en dehors du plateau, la considérer comme vide (0)
            else {
                entourage[x_entourage][y_entourage] = 0;
            }
        }
    }
}

/*  
    Cette fonction calcule un vecteur déplacement.
*/
void compute_vector(Vecteur deplacement, Pion pion_avant, Pion pion_apres)
{
    // Calcul des composantes du vecteur de déplacement
    deplacement.dx = pion_apres.x - pion_avant.x;
    deplacement.dy = pion_apres.y - pion_avant.y;

}

/*  
    Fonction pour valider un déplacement.
*/
bool move_pion(uint8_t plateau[5][9], Pion pion_depart, Pion pion_arrivee, Vecteur deplacement)
{
    // Vérification si les coordonnées de départ et d'arrivée sont valides
    if ( pion_depart.x < 0 || pion_depart.x >= 5  || pion_depart.y < 0  || pion_depart.y >= 9 ||
        pion_arrivee.x < 0 || pion_arrivee.x >= 5 || pion_arrivee.y < 0 || pion_arrivee.y >= 9   ) 
    {
        return false; // Déplacement invalide
    }

    // Vérification si la case d'arrivée est vide
    if (plateau[pion_arrivee.x][pion_arrivee.y] != 0) {
        return false; // Déplacement invalide - case occupée
    }

    // Calcul du vecteur de déplacement
    compute_vector(deplacement, pion_depart, pion_arrivee);

    // Vérification du mouvement
    if ((deplacement.dx == 1 || deplacement.dx == -1 || deplacement.dx == 0) &&
        (deplacement.dy == 1 || deplacement.dy == -1 || deplacement.dy == 0)) 
    {
        // Déplacement autorisé, maj du plateau
        plateau[pion_arrivee.x][pion_arrivee.y] = plateau[pion_depart.x][pion_depart.y];
        plateau[pion_depart.x][pion_depart.y] = 0; // Vide la case de départ

        return true ;
    }
    else {
        return false; // Déplacement invalide
    }
}


/*
    Fonction pour éliminations
*/
void kill(uint8_t plateau[5][9], uint8_t entourage[5][5], Pion pion_arrivee, Pion pion_depart, Vecteur deplacement)
{   
    int kil1_x = pion_arrivee.x + deplacement.dx ;
    int kil1_y = pion_arrivee.y + deplacement.dy ;
    int kil2_x = pion_depart.x - deplacement.dx ;
    int kil2_y = pion_depart.y - deplacement.dx ;
    
    while ((plateau[kil1_x][kil1_y] =! 0) && (plateau[kil1_x][kil1_y] =! plateau[pion_arrivee.x][pion_arrivee.y])) {
        if ((kil1_x <5 && kil1_x >= 0) && (kil1_y <9 && kil1_y >=0)) {
            plateau[kil1_x][kil1_y] = 0 ; // éliminations par percussion
            kil1_x = kil1_x + deplacement.dx ;
            kil1_y = kil1_y + deplacement.dy ;            
        }

    }

    while ((plateau[kil2_x][kil2_y] =! 0) && (plateau[kil2_x][kil2_y] =! plateau[pion_depart.x][pion_depart.y])) {    
        if ((kil2_x <5 && kil2_x >= 0) && (kil2_y <9 && kil2_y >=0)) {
            plateau[kil2_x][kil2_y] = 0 ; // éliminations par aspiration
            kil2_x = kil2_x - deplacement.dx ;
            kil2_y = kil2_y - deplacement.dy ;
        }
    } 
}