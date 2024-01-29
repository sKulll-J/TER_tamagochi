#ifndef TERLIB_H
#define TERLIB_H

/*
 * @brief Cette librairie contient tout le code commun à chaque jeu :
 *      - communication entre console
 *      - la tonne de define
 *      - les structs/typedef
 *      - les fonctions de jeu
*/

// DEFINE ------------------------------------------------------
#define MAT_WIDTH   9   // taille horizontale de la matrice de led
#define MAT_HEIGHT  9   // taille verticale   de la matrice de led

// Magic numbers pour le uint8_t input
/*
    INPUT :
        A
        B
        droite
        gauche
        haut
        bas
    => 6 possibilités
    on met ca dans un uint8 et on l'utilise comme un registre
*/
#define INPUT_A     0b00000001
#define INPUT_B     0b00000010
#define INPUT_LEFT  0b00000100
#define INPUT_RIGHT 0b00001000
#define INPUT_DOWN  0b00010000
#define INPUT_UP    0b00100000
#define INPUT_COUNT 6           // nombre de touches totales

#define RUN         1
#define STOP        0
#define WIN         1
#define LOSE        0

#define LED_NOIR    0
#define LED_BLANC   3
#define PLAYER1     1
#define PLAYER2     2
#define OWN_COLOR   CRGB::Red
#define OPPS_COLOR  CRGB::Green

#include <stdint.h>
#include <stdbool.h>

// TYPEDEF -----------------------------------------------------
typedef struct {  // Matrice de LED
    const uint8_t width;
    const uint8_t height;
    uint8_t led[MAT_WIDTH][MAT_HEIGHT][3];    // width/height/channel GRB
} mat_t;

enum game_type {    // choix du jeu actuel (snake, morpion etc)
    NONE, //0
    MEGAMORPION, //1
    SNAKE, //2
    TRON,  //3
    FANORONA
};

typedef struct {
    enum game_type current_game;    // jeu actuel choisi 
    uint8_t current_player;         // qui va jouer le coup suivant
    bool state;                     // 0 en cours - 1 partie terminée
    uint8_t printmatrix[9][9];      // matrice à traiter
    bool winlose;                   // est-ce que PLAYER1 a gagné ou perdu
} game_t;


// DECLARATION DE FONCTIONS ------------------------------------
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    game_t megamorpion(game_t game, uint8_t input);
    game_t snake(game_t game, uint8_t input);
    game_t tron(game_t game, uint8_t input);
    game_t fanorona(game_t game, uint8_t input);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif