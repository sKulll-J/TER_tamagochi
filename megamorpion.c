#include <stdint.h> // uint8_t sinon il hurle
#include <math.h>   // NaN et inf
#include <time.h>   // rand()

#include "terlib.h"

#define INIT_VALUE 255

/* fonctions nécessaires 
    - fonction jeu prend en paramètre tout les inputs et plus tard les inputs adverses
        -> struct input_t owninput et input_t opps_input (ive got opps everywhere)
    - fonction de jeu return une struct game_s qui contient l'état de la partie (en cours/terminée), le winner, la matrice à afficher etc
    - gestion d'erreur si aucun adversaire n'est connecté (clignotement rouge ou whatnot)
    - utilisation de registre pour utiliser moins de place (verifier si le code de bitshift prend plus de place que faire avec des uint normaux mais je pense pas)
    - déplacement du curseur (couleur blanche pour distinguer des "x/o" déjà placés)
    - update et draw les leds en accord avec la xomatrix
    - check win (scalable sur une minigrid et une megagrid)
*/

// DECLARATION DE FONCTIONS ------------------------------------
static uint8_t check_win(float (*mat)[3]); // magie noire

game_t megamorpion(game_t game_data, uint8_t input)
{
    // DECLARATIONS/INITIALISATIONS ----------------------------
    srand(time(NULL));

    /* Il faut tout mettre en static parce que la fonction est appelée genre 60 fois par seconde donc il faut pas réinitialiser des valeurs
       problème : on peut pas mettre des valeurs random dans un static (on veut un random pour commencer au pif qqpart dans la grille)
       solution : on initialise une seule fois (avec INIT_VALUE qui vaut 255 donc impossible à obtenir avec un rand() % 9)
                  puis on re-initialise avec des valeurs qu'on veut
    */
    static uint8_t init_flag = 1;
    static uint8_t minix = INIT_VALUE;  // position x sur une minigrille
    static uint8_t miniy = INIT_VALUE;  // position y sur une minigrille
    static uint8_t megax = INIT_VALUE;  // position x sur la megagrille
    static uint8_t megay = INIT_VALUE;  // position y sur la megagrille
    if (init_flag) {
        minix = rand() % 3;
        miniy = rand() % 3;
        megax = rand() % 3;
        megay = rand() % 3;

        init_flag = 0;
    }
    static float xomatrix[3][3][3][3] = {NAN};  // matrice qui contient les valeurs 0=own, 1=opps, NaN=personne - matrice LOGIQUE du jeu sur lequel on fait les calculs de victoire
    static uint8_t colmatrix[9][9] = {0};       // matrice qui contient les couleurs 0=noir, 1=couleur J1, 2=couleur J2, 3=blanc(curseur) - matrice AFFICHAGE du jeu sur lequel on change les valeurs des leds
    static float megamatrix[3][3] = {NAN};      // matrice 3x3 mega contenant les valeurs 0, 1, NaN, inf (inf = égalité sur la minigrille, pas de vainqueur possible)
    static uint8_t miniwin[3][3];               // couleur de chaque minigrille[x][y]


    // GAME LOOP -----------------------------------------------
    switch (input) {
        case INPUT_LEFT :  if (minix > 0) minix--; break;
        case INPUT_RIGHT : if (minix < 2) minix++; break;
        case INPUT_DOWN :  if (miniy > 0) miniy--; break;
        case INPUT_UP :    if (miniy < 2) miniy++; break;
        case INPUT_A :
            if (isnan(xomatrix[megax][megay][minix][miniy])) { // si la case est libre
                xomatrix[megax][megay][minix][miniy] = 0;
                colmatrix[megax*3+minix][megay*3+miniy] = game_data.current_player;
            } 
            
            colmatrix[megax*3+minix][megay*3+miniy] = LED_BLANC;    // formule magique qui transforme une coordonnee 3x3x3x3 en 9x9
            megax = minix;  // prochain coup dans la même mégacase que le coup joué dans la minicase
            megay = miniy;

            //+ protection si ya plus de place possible dans la case, que faire ? tp ailleurs ?
            // for i,j, if xomatrix[prochain x][prochain y][i][j] == NaN
            //  tp ailleurs
            while (!isnan(xomatrix[megax][megay][minix][miniy])) {  // ptit random pour tp dans la minicase suivante un peu au pif mais sur une case libre
                minix = rand() % 3;
                miniy = rand() % 3;
            }

            /* Tout ce bout de code vérifie si chaque minigrid a une victoire
             * Puis "remonte" d'un cran en stockant la valeur de la victoire dans une grille 3x3 (megamatrix)
             * Afin de faire une check_win sur cette matrice la pour déterminer s'il y a un gagnant à la partie
            */
            for (uint8_t i=0; i<3; i++) {
                for (uint8_t j=0; j<3; j++) {
                    if (miniwin[j][i] == LED_NOIR) {     // si ya deja une win il faut pas revérifier
                        miniwin[j][i] = check_win(xomatrix[i][j]);  // ? pourquoi miniwin[j][i] et pas [i][j]?? je sais pas mais ça marche alors nsm
                        
                        if (miniwin[j][i] == PLAYER1) megamatrix[j][i] = 0;
                        if (miniwin[j][i] == PLAYER2) megamatrix[j][i] = 1;
                    }

                    if (check_win(megamatrix) == PLAYER1) game_data.winlose = WIN;
                    if (check_win(megamatrix) == PLAYER2) game_data.winlose = LOSE;
                }
            }

            
            break;
        default : return game_data; // pas d'input = quitte direct le bail
    }

    for (uint8_t i=0; i<MAT_WIDTH; i++) {   // on met la matrice de couleur dans la matrice de game_data
        for (uint8_t j=0; j<MAT_HEIGHT; j++) {
            game_data.printmatrix[i][j] = colmatrix[i][j];
        }
    }
    
    return game_data;
}


// FONCTIONS ---------------------------------------------------
static uint8_t check_win(float (*mat)[3])
{
    uint8_t i, j;
    float col, lin;      // somme de la colonne / somme de la ligne
    float diag1, diag2;  // somme des diagonales

    for (i=0; i<3; i++) {
        lin = 0;
        col = 0;
        diag1 = 0;
        diag2 = 0;

        for (j=0; j<3; j++) {
            lin += mat[i][j];
            col += mat[j][i];
            diag1 += mat[j][j];
            diag2 += mat[j][2-j];
        }

        if (!isnan(lin) && (lin == 0)) return PLAYER1;
        else if (!isnan(lin) && (lin == 3)) return PLAYER2;
        else if (!isnan(col) && (col == 0)) return PLAYER1;
        else if (!isnan(col) && (col == 3)) return PLAYER2;
        else if (!isnan(diag1) && !isinf(diag1) && (diag1 == 0)) return PLAYER1;
        else if (!isnan(diag1) && !isinf(diag1) && (diag1 == 3)) return PLAYER2;
        else if (!isnan(diag2) && !isinf(diag2) && (diag2 == 0)) return PLAYER1;
        else if (!isnan(diag2) && !isinf(diag2) && (diag2 == 3)) return PLAYER2;

        //else if (!isinf())
    }

    return 0;  // revient à renvoyer "la somme de la ligne/colonne/diagonales est un NaN"
}
