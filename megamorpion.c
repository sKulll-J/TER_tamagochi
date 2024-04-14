#include <stdlib.h>
#include <time.h>
#include <stdint.h>  // uint8_t sinon il hurle
#include <math.h>    // NaN et inf
#include <Arduino.h> // random()

#include "terlib.h"
#include "color.h"


// DECLARATION DE FONCTIONS ------------------------------------
static void color_matching(struct game_s game_data, uint8_t *col_current_own, uint8_t *col_current_clair_own, uint8_t *col_current_opp, uint8_t *col_current_clair_opp);
static void calcul_coord(uint8_t *x, uint8_t *y, uint8_t megax, uint8_t megay, uint8_t minix, uint8_t miniy);  // transforme les megax/miniy... en x,y de 9x9
static void croix_directionnelle(uint8_t input, uint8_t *x, uint8_t *y);
static uint8_t check_win(float (*mat)[3]); // magie noire
static uint8_t checksub(float val);
static void joker_mode(uint8_t input, float (*xomatrix)[3][3][3], uint8_t megax, uint8_t megay);   // gros selector

struct game_s megamorpion(struct game_s game_data, uint8_t input)
{
    // DECLARATIONS --------------------------------------------
    
    /* Il faut tout mettre en static parce que la fonction est appelée genre 60 fois par seconde donc il faut pas réinitialiser des valeurs
     *      problème : on peut pas mettre des valeurs random dans un static (on veut un random pour commencer au pif qqpart dans la grille)
     *      solution : on appelle la fonction de jeu avec comme input le nombre généré aléatoirement pour connaitre qui commence en premier et on le %3 pour avoir une valeur de départ
     */
    static uint8_t x;                   // position x absolue 9x9
    static uint8_t y;                   // position y absolue 9x9
    static uint8_t minix = 0;           // position x sur une minigrille
    static uint8_t miniy = 0;           // position y sur une minigrille
    static uint8_t megax = 0;           // position x sur la megagrille
    static uint8_t megay = 0;           // position y sur la megagrille
    static float xomatrix[3][3][3][3];  // matrice qui contient les valeurs 0=own, 1=opp, NaN=personne - matrice LOGIQUE du jeu sur lequel on fait les calculs de victoire
    static float megawin[3][3];         // matrice 3x3 mega contenant les valeurs 0, 1, NaN, inf (inf = égalité sur la minigrille, pas de vainqueur possible)
    static uint8_t miniwin[3][3] = {0}; // couleur de chaque minigrille[x][y]
    static uint8_t col_current_own;
    static uint8_t col_current_own_clair;
    static uint8_t col_current_opp;
    static uint8_t col_current_opp_clair;
    static uint8_t flag_input = 1;

    // INITIALISATIONS -----------------------------------------
    if (flag_input == 1) {
        // on a passé initalement en input tergame.current_player après avoir determiné qui commence. On l'utilise pour avoir du random dans la position, car si on faisait x=rand(3) par exemple on aurait des positions differentes sur chaque console
        input %= 9;
        megax = input % 3;
        megay = input % 3;
        minix = (input - megax + 1) % 3; // transforme [9][9] en [3][3][3][3] je crois (au pire osef cest juste un peu de random)
        miniy = (input - megay - 1) % 3; 

        #if DEBUG
            Serial.println("[?] Init");
            Serial.println();
            Serial.print(input); Serial.println(" % 3");
            Serial.print("pos: "); 
            Serial.print("[");Serial.print(megax);Serial.print("]");
            Serial.print("[");Serial.print(megay);Serial.print("]");
            Serial.print("[");Serial.print(minix);Serial.print("]");
            Serial.print("[");Serial.print(miniy);Serial.println("]");
        #endif

        // Remplissage des matrices en "vide"
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

        // Quelle couleur commence
        color_matching(game_data, &col_current_own, &col_current_own_clair, &col_current_opp, &col_current_opp_clair);

        // Affiche le selecteur avant un input
        calcul_coord(&x, &y, megax, megay, minix, miniy);
        game_data.printmatrix[x][y] = col_current_own;

        flag_input = 0;
    }
    
    
    // GAME LOOP -----------------------------------------------
    color_matching(game_data, &col_current_own, &col_current_own_clair, &col_current_opp, &col_current_opp_clair);

    if (input != INPUT_A) {  
        game_data.printmatrix[x][y] = game_data.previous_printmatrix[x][y];    // sert à effacer l'ancienne position du selecteur
        croix_directionnelle(input, &minix, &miniy);
        calcul_coord(&x, &y, megax, megay, minix, miniy);

        // déplacement du curseur de sélection - clair si ya deja un pion de placé
        if (xomatrix[megax][megay][minix][miniy] == 0) { 
            game_data.printmatrix[x][y] = col_current_own_clair; 
        } else if (xomatrix[megax][megay][minix][miniy] == 1) { 
            game_data.printmatrix[x][y] = col_current_opp_clair;
        } else { 
            game_data.printmatrix[x][y] = col_current_own;
        }
    } else if (input == INPUT_A) {       
        if (isnan(xomatrix[megax][megay][minix][miniy])) {                      // si la case est libre
            xomatrix[megax][megay][minix][miniy] = game_data.current_player;    // on met la valeur du joueur
            game_data.printmatrix[x][y] = col_current_own;                      // et sa couleur associée

            game_data.current_player = !game_data.current_player;               // inversion des joueurs
            color_matching(game_data, &col_current_own, &col_current_own_clair, &col_current_opp, &col_current_opp_clair);

            /*  Tout ce bout de code vérifie si chaque minigrid a une victoire
             *  Puis "remonte" d'un cran en stockant la valeur de la victoire dans une grille 3x3 (megawin)
             *  Afin de faire une check_win sur cette matrice la pour déterminer s'il y a un gagnant à la partie
             */
            for (uint8_t i=0; i<3; i++) {
                for (uint8_t j=0; j<3; j++) {
                    if (miniwin[j][i] == COL_NOIR) {                // si ya deja une win il faut pas revérifier
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

            // Teleportation fin de tour
            megax = minix;  // prochain coup dans la même mégacase que le coup joué dans la minicase
            megay = miniy;  // ---
            for (int i=0; i<3; i++) {
                for (int j=0; j<3; j++) {
                    if (!isnan(xomatrix[megax][megay][i][j])) { // for i,j, if xomatrix[prochain x][prochain y][i][j] == NaN
                        minix = i;
                        miniy = j;
                        break;
                    }
                }
                if (!isnan(xomatrix[megax][megay][minix][miniy])) break;
                //else joker_mode(input, xomatrix, megax, megay);  // si aucune case libre : mode joker où on peut se relocaliser nimporte où sur la map
            }
            calcul_coord(&x, &y, megax, megay, minix, miniy);
            game_data.printmatrix[x][y] = col_current_own;  // affiche le pion téléporté
        }
        
            
    }
    //else return game_data; // pas d'input = quitte direct le bail

    #if DEBUG
        Serial.println("\n[?] XOmatrix");
        Serial.print("C'est au tour de ");
        if (game_data.current_player == PLAYER1) Serial.println("MOI");
        if (game_data.current_player == PLAYER2) Serial.println("LUI");
        for (int i=0; i<9; i++) {
            for (int j=0; j<9; j++) {
                if (game_data.printmatrix[i][j] == COL_NOIR) Serial.print(".");
                else Serial.print("O");

                if (j==2 || j==5) Serial.print(" ");
            }
            if (i==2 || i==5) Serial.println();
            Serial.println();
        }
        Serial.println("[?] Fin update");
    #endif
    
    return game_data;
}


// FONCTIONS ---------------------------------------------------
/**
 *  @brief Cette fonction attribue la paire de couleur foncée/claire au tour actuel (ça sert à inverser les palettes quoi)
 */
void color_matching(struct game_s game_data, uint8_t *col_current_own, uint8_t *col_current_own_clair, uint8_t *col_current_opp, uint8_t *col_current_opp_clair)
{
    if (game_data.current_player == PLAYER1) {
        *col_current_own = COL_OWN;
        *col_current_own_clair = COL_OWN_CLAIR;
        *col_current_opp = COL_OPP;
        *col_current_opp_clair = COL_OPP_CLAIR;
    } else if (game_data.current_player == PLAYER2){
        *col_current_own = COL_OPP;
        *col_current_own_clair = COL_OPP_CLAIR;
        *col_current_opp = COL_OWN;
        *col_current_opp_clair = COL_OWN_CLAIR;
    }
}

/** 
 *  @brief Cette fonction transforme les coordonnées [3][3][3][3] en [9][9] 
 *  @param x coordonnée x de la matrice 9x9
 *  @param y coordonnée y de la matrice 9x9
 *  @param megax coordonnée x dans la grande matrice 3x3
 *  @param megay coordonnée y dans la grande matrice 3x3
 *  @param minix coordonnée x dans la petite matrice 3x3
 *  @param miniy coordonnée y dans la petite matrice 3x3
 */
void calcul_coord(uint8_t *x, uint8_t *y, uint8_t megax, uint8_t megay, uint8_t minix, uint8_t miniy)
{
    *x = megax * 3 + minix;
    *y = megay * 3 + miniy;
}

void croix_directionnelle(uint8_t input, uint8_t *x, uint8_t *y) // 17719
{
    if (input == INPUT_L) { if (*x > 0) (*x)--; } 
    else if (input == INPUT_R) { if (*x < 2) (*x)++; }
    else if (input == INPUT_D) { if (*y > 0) (*y)--; }
    else if (input == INPUT_U) { if (*y < 2) (*y)++; }
}

void joker_mode(uint8_t input, float (*xomatrix)[3][3][3], uint8_t megax, uint8_t megay)
{
    croix_directionnelle(input, &megax, &megay);
    xomatrix[1][1][1][1] = 3;

}


/** 
 *  @brief Cette fonction prend une matrice 3x3 et vérifie si l'un des deux joueurs a gagné.  
 *         La vérification se fait en calculant la somme de chaque ligne, colonne et diagonale suivant le principe que:
 *              NaN + x = NaN => la lin/col/diag n'est pas complète
 *              0 + 0 + 0 = 0 => la lin/col/diag est gagnée par le J1
 *              1 + 1 + 1 = 3 => la lin/col/diag est gagnée par le J2
 *              1 + 0 + 1 = 2 => la lin/col/diag n'est gagnée par personne
 *  @param mat matrice 3x3
 *
 *  @todo rajouter les test isinf() pour detecter une megacase où ya une égalité
 */
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

/** 
 *  @brief Cette fonction vérifie quelle est la valeur qu'elle reçoit
 *          NaN => return 0
 *          0 => return J1
 *          3 => return J2
 *  @param val 
 */
uint8_t checksub(float val)
{
    if (isnan(val) == false) {
        if (val == 0) return PLAYER1;
        if (val == 3) return PLAYER2;
    }
    return 0;
}
