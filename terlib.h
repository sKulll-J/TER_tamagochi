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

//def des pins de boutons
#define PIN_A            3   // bouton "valider"
#define PIN_B            4   // bouton "annuler"
#define PIN_UP           5   // bouton croix directionnelle "haut"
#define PIN_LEFT         6   // bouton croix directionnelle "gauche"
#define PIN_DOWN         7   // bouton croix directionnelle "bas"
#define PIN_RIGHT        8   // bouton croix directionnelle "droite"

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
#define MAGIC_NO_INPUT 0xFF     // magie noire

#define RUN         1
#define STOP        0
#define WIN         1
#define LOSE        0

#define LED_NOIR    0
#define LED_BLANC   3
#define PLAYER1     1
#define PLAYER2     2
#define OWN_COLOR   CRGB::Green
#define OPPS_COLOR  CRGB::Red

// Communication
#define PIN_RX           0
#define PIN_TX           1
#define MAGIC_PAIRING   0xC2

#include <stdint.h>
#include <stdbool.h>

// TYPEDEF -----------------------------------------------------
typedef struct {  // Matrice de LED
    const uint8_t width;
    const uint8_t height;
    uint8_t led[MAT_WIDTH][MAT_HEIGHT][3];    // width/height/channel GRB
} mat_t;

enum game_type {    // choix du jeu actuel (snake, morpion etc)
    NONE,
    SNAKE,
    MEGAMORPION,
    TRON,
    FANORONA
};

enum game_mode {
    UNDEFINED,  // undefined
    SOLO,       // jeu solo (sans déconner erwann)
    TBS,        // Turned Based Strategy - Tour par tour
    RT          // Real-Time
};

typedef struct {
    enum game_type current_game;    // jeu actuel choisi 
    enum game_mode mode;            // jeu solo/multi/synchrone
    uint8_t current_player;         // qui va jouer le coup suivant
    bool state;                     // 0 en cours - 1 partie terminée
    bool winlose;                   // est-ce que PLAYER1 a gagné ou perdu
    uint8_t printmatrix[9][9];      // matrice à traiter
} game_t;


// DECLARATION DE FONCTIONS ------------------------------------
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    game_t megamorpion(game_t game, uint8_t input);
    game_t snake(game_t game, uint8_t input);
    game_t tron(game_t game, uint8_t input);
    game_t fanorona(game_t game, uint8_t input);

    uint8_t readinput(void);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif