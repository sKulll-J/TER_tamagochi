#include <stdint.h> // uint8_t sinon il hurle
#include <math.h>   // NaN et inf
#include <Arduino.h>// random()

#include "terlib.h"

#define INIT_VALUE 255

/* fonctions nécessaires 
    - fonction jeu prend en paramètre tout les inputs et plus tard les inputs adverses
        -> struct input_t owninput et input_t opps_input (ive got opps everywhere)
    - gestion d'erreur si aucun adversaire n'est connecté (clignotement rouge ou whatnot)
*/

// DECLARATION DE FONCTIONS ------------------------------------
static uint8_t check_win(float (*mat)[3]); // magie noire
static uint8_t checksub(float val);

game_t megamorpion(game_t game_data, uint8_t input)
{
    // DECLARATIONS --------------------------------------------
    /* Il faut tout mettre en static parce que la fonction est appelée genre 60 fois par seconde donc il faut pas réinitialiser des valeurs
       problème : on peut pas mettre des valeurs random dans un static (on veut un random pour commencer au pif qqpart dans la grille)
       solution : on initialise une seule fois (avec INIT_VALUE qui vaut 255 donc impossible à obtenir avec un rand() % 3)
                  puis on re-initialise avec des valeurs qu'on veut
    */
    static uint8_t init_flag = true;
    static uint8_t minix = INIT_VALUE;      // position x sur une minigrille
    static uint8_t miniy = INIT_VALUE;      // position y sur une minigrille
    static uint8_t megax = INIT_VALUE;      // position x sur la megagrille
    static uint8_t megay = INIT_VALUE;      // position y sur la megagrille
    static float xomatrix[3][3][3][3];      // matrice qui contient les valeurs 0=own, 1=opps, NaN=personne - matrice LOGIQUE du jeu sur lequel on fait les calculs de victoire
    static float megawin[3][3];          // matrice 3x3 mega contenant les valeurs 0, 1, NaN, inf (inf = égalité sur la minigrille, pas de vainqueur possible)
    static uint8_t colmatrix[9][9] = {0};   // matrice qui contient les couleurs 0=noir, 1=couleur J1, 2=couleur J2, 3=blanc(curseur) - matrice AFFICHAGE du jeu sur lequel on change les valeurs des leds
    static uint8_t miniwin[3][3] = {0};     // couleur de chaque minigrille[x][y]


    // INITIALISATIONS -----------------------------------------
    if (init_flag == true) {
        minix = random() % 3;
        miniy = random() % 3;
        megax = random() % 3;
        megay = random() % 3;
    
        for (uint8_t i=0; i<3; i++) {
            for (uint8_t j=0; j<3; j++) {
                for (uint8_t k=0; k<3; k++) {
                    for (uint8_t l=0; l<3; l++) {
                        xomatrix[i][j][k][l] = NAN; // matrice de jeu vide
                    }
                }
                megawin[i][j] = NAN; // matrice de jeu vide
            }
        }

        init_flag = false;
    }


    // GAME LOOP -----------------------------------------------
    switch (input) {
        case INPUT_LEFT:  if (minix > 0) minix--; break;
        case INPUT_RIGHT: if (minix < 2) minix++; break;
        case INPUT_DOWN:  if (miniy > 0) miniy--; break;
        case INPUT_UP:    if (miniy < 2) miniy++; break;
        case INPUT_A:
            if (isnan(xomatrix[megax][megay][minix][miniy])) { // si la case est libre
                xomatrix[megax][megay][minix][miniy] = game_data.current_player;    // on met la valeur du joueur
                colmatrix[megax*3+minix][megay*3+miniy] = game_data.current_player; // et sa couleur associée
            
                megax = minix;  // prochain coup dans la même mégacase que le coup joué dans la minicase
                megay = miniy;  // ---

                //+ protection si ya plus de place possible dans la case, que faire ? tp ailleurs ?
                // for i,j, if xomatrix[prochain x][prochain y][i][j] == NaN
                //  tp ailleurs
                while (!isnan(xomatrix[megax][megay][minix][miniy])) {  // ptit random pour tp dans la minicase suivante un peu au pif mais sur une case libre
                    minix = rand() % 3;
                    miniy = rand() % 3;
                }

                /* Tout ce bout de code vérifie si chaque minigrid a une victoire
                    * Puis "remonte" d'un cran en stockant la valeur de la victoire dans une grille 3x3 (megawin)
                    * Afin de faire une check_win sur cette matrice la pour déterminer s'il y a un gagnant à la partie
                */
                for (uint8_t i=0; i<3; i++) {
                    for (uint8_t j=0; j<3; j++) {
                        if (miniwin[j][i] == COL_NOIR) {     // si ya deja une win il faut pas revérifier
                            miniwin[j][i] = check_win(xomatrix[i][j]);  // ? pourquoi miniwin[j][i] et pas [i][j]?? je sais pas mais ça marche alors nsm
                            
                            if (miniwin[j][i] == PLAYER1) megawin[j][i] = 0;
                            if (miniwin[j][i] == PLAYER2) megawin[j][i] = 1;
                        }

                        switch (check_win(megawin)) {
                            case PLAYER1: game_data.winlose = WIN;  break;
                            case PLAYER2: game_data.winlose = LOSE; break;
                            default: break;
                        }
                    }
                }
            }
            else {  //(donc si la case n'est pas libre), on passe le "curseur" en blanc pour signifier qu'on est au dessus d'un x/o 
                colmatrix[megax*3+minix][megay*3+miniy] = COL_BLANC;    // formule magique qui transforme une coordonnee 3x3x3x3 en 9x9
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
/* Cette fonction prend une matrice 3x3 et vérifie si l'un des deux joueurs a gagné.  
 * La vérification se fait en calculant la somme de chaque ligne, colonne et diagonale suivant le principe que:
    NaN + x = NaN => la lin/col/diag n'est pas complète
    0 + 0 + 0 = 0 => la lin/col/diag est gagnée par le J1
    1 + 1 + 1 = 3 => la lin/col/diag est gagnée par le J2
    1 + 0 + 1 = 2 => la lin/col/diag n'est gagnée par personne
 */
// TODO : rajouter les test isinf() pour detecter une megacase où ya une égalité
uint8_t check_win(float (*mat)[3]) 
{
    uint8_t CR;
    float lin = 0;
    float col = 0;
    float diag1 = 0;
    float diag2 = 0;
    
    // Calcul des valeurs
    for (int i=0; i<3; i++) {
        lin = 0;
        col = 0;
        for (int j=0; j<3; j++) {
            lin += mat[i][j];
            col += mat[j][i];
        }
        diag1 += mat[i][i];
        diag2 += mat[i][2-i];

        if ((CR = checksub(lin)) != 0) return CR;
        if ((CR = checksub(col)) != 0) return CR;
    }
    if ((CR = checksub(diag1)) != 0) return CR;
    if ((CR = checksub(diag2)) != 0) return CR;

    return 0;   // revient à renvoyer "la somme de la ligne/colonne/diagonales est un NaN"
}

/* Cette fonction vérifie quelle est la valeur qu'elle reçoit
 * NaN => return 0
 * 0 => return J1
 * 3 => return J2
 */
uint8_t checksub(float val)
{
    if (isnan(val) == false) {
        if (val == 0) return PLAYER1;
        if (val == 3) return PLAYER2;
    }
    return 0;
}