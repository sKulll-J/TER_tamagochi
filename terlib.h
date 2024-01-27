#ifndef TERLIB_H
#define TERLIB_H


// DEFINE ------------------------------------------------------
#define MAT_WIDTH       9   // taille horizontale de la matrice de led
#define MAT_HEIGHT      9   // taille verticale   de la matrice de led


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

typedef struct {
    enum game_type current_game;    // jeu actuel choisi 
    bool current_player;            // qui va jouer le coup suivant
    bool state;                     // 0 en cours - 1 partie terminée
} game_t;

typedef struct {
    // je sais pas
} input_t;

game_t g_megamorpion(mat_t mat, input_t owninput, input_t oppsinput);
game_t g_snake(mat_t mat, input_t owninput);
game_t g_tron(mat_t mat, input_t owninput, input_t oppsinput);
game_t g_fanorona(mat_t mat, input_t owninput, input_t oppsinput);

#endif