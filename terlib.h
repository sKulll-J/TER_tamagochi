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
#define INPUT_A 0b00000001
#define INPUT_B 0b00000010
#define INPUT_L 0b00000100
#define INPUT_R 0b00001000
#define INPUT_D 0b00010000
#define INPUT_U 0b00100000

#define RUN         1
#define ter_STOP    0
#define WIN         1
#define LOSE        0
#define TIE         2
#define PLAYER1     0   // utilisable en tant que booléen pour inverser facilement avec "!"
#define PLAYER2     1   // utilisable en tant que booléen pour inverser facilement avec "!"


// game_type - choix du jeu actuel (snake, morpion etc)
#define NONE        0
#define SNAKE       1  
#define MEGAMORPION 2
#define FANORONA    3
#define TRON        4
#define SELECTOR    5

// game_mode
#define UNDEFINED   0   // undefined
#define SOLO        1   // jeu solo (sans déconner erwann)
#define SEQUENTIEL  2   // Tour par tour
#define SYNCHRONE   3   // Tourne à 60fps

#include <stdint.h>
#include <stdbool.h>

// TYPEDEF -----------------------------------------------------
typedef struct {  // Matrice de LED
    const uint8_t width;
    const uint8_t height;
    uint8_t led[MAT_WIDTH][MAT_HEIGHT][3];    // width/height/channel GRB
} mat_t;


struct game_s {
    uint8_t current_game;                       // jeu actuel choisi 
    uint8_t mode;                               // jeu solo/sequentiel/synchrone
    struct game_s (*function)(struct game_s, uint8_t); // pointeur de fonction
    uint8_t current_player;                     // qui va jouer le coup suivant
    bool state;                                 // 0 en cours - 1 partie terminée
    bool winlose;                               // est-ce que PLAYER1 a gagné ou perdu
    uint8_t printmatrix[9][9];                  // matrice à traiter
    uint8_t previous_printmatrix[9][9];         // matrice à traiter du coup d'avant
    unsigned long game_time;
    uint8_t tickrate;                           // temps entre chaque frame
};


// DECLARATION DE FONCTIONS ------------------------------------
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    struct game_s snake(struct game_s game_data, uint8_t input);
    struct game_s megamorpion(struct game_s game_data, uint8_t input);
    struct game_s fanorona(struct game_s game_data, uint8_t input);
    struct game_s tron(struct game_s game_data, uint8_t input);
    struct game_s selector(struct game_s game_data, uint8_t input); // pour debug
#ifdef __cplusplus
}
#endif // __cplusplus

#endif