#ifndef TERLIB_H
#define TERLIB_H

#define NANO_axel

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

#ifdef UNO_axel
    // PIN de boutons
    #define PIN_A            6   // bouton "valider"
    #define PIN_B            7   // bouton "annuler"
    #define PIN_UP           3   // bouton croix directionnelle "haut"
    #define PIN_DOWN         5   // bouton croix directionnelle "gauche"
    #define PIN_LEFT         2   // bouton croix directionnelle "bas"
    #define PIN_RIGHT        4   // bouton croix directionnelle "droite"

    // PIN de la cartouche
    #define PIN_CARTOUCHE_0  8   // pin pour lire quelle "cartouche" est insérée
    #define PIN_CARTOUCHE_1  10  // ---
    #define PIN_CARTOUCHE_2  12  // ---

    // Fastled
    #define LED_DATA_PIN    13   // pin sur laquelle transite les données de la matrice
#endif

#ifdef NANO_axel
    // PIN de boutons
    #define PIN_A      6    // 24
    #define PIN_B      7    // 25
    #define PIN_DOWN   2    // 20
    #define PIN_LEFT  14    // 27
    #define PIN_UP    15    // 26
    #define PIN_RIGHT 16    // 25

    // Fastled
    #define LED_DATA_PIN    3   // 21 

    // PIN de la cartouche 
    #define PIN_CARTOUCHE_0  12  // 30
    #define PIN_CARTOUCHE_1  11  // 29
    #define PIN_CARTOUCHE_2  10  // 28

    
#endif


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
#define INPUT_INIT  0b10000000  // pas vraiment un input, mais sert de flag en quelque sorte
#define INPUT_COUNT 6           // nombre de touches totales
#define MAGIC_NO_INPUT 0xFF     // magie noire

#define RUN         1
#define ter_STOP        0
#define WIN         1
#define LOSE        0
#define PLAYER1     0   // utilisable en tant que booléen pour inverser facilement avec "!"
#define PLAYER2     1   // utilisable en tant que booléen pour inverser facilement avec "!"

// indice palette qui se trouve dans la printmatrix
#define COL_NOIR        0
#define COL_BLANC       1
#define COL_OWN         2
#define COL_OWN_CLAIR   3
#define COL_OPPS        4
#define COL_OPPS_CLAIR  5

// couleur réelle à changer selon quelle console on flashe
// (R)OUGE, (B)LEU, (V)ERT, (J)AUNE, (M)AGENTA
#define ter_R 0
#define ter_B 1
#define ter_V 2
#define ter_J 3
#define ter_M 4

#define COULEUR_J1 ter_J
#define COULEUR_J2 ter_M

#define ROUGE     CRGB(0, 255, 0)
#define ROUGE_C   CRGB(255, 119, 119)
#define BLEU      CRGB(0, 0, 255)
#define BLEU_C    CRGB(119, 178, 255) 
#define VERT      CRGB(0, 255, 0)
#define VERT_C    CRGB(50, 255, 30)
#define JAUNE     CRGB(255, 255, 0)
#define JAUNE_C   CRGB(255, 255, 60)
#define MAGENTA   CRGB(255, 0, 255);
#define MAGENTA_C CRGB(255, 120, 255);

// Couleur du joueur 1 (OWN)
#if COULEUR_J1 == ter_R
    #define OWN_COLOR        ROUGE
    #define OWN_CLAIR_COLOR  ROUGE_C
#elif COULEUR_J1 == ter_B
    #define OWN_COLOR        BLEU
    #define OWN_CLAIR_COLOR  BLEU_C
#elif COULEUR_J1 == ter_V
    #define OWN_COLOR        VERT
    #define OWN_CLAIR_COLOR  VERT_C
#elif COULEUR_J1 == ter_J
    #define OWN_COLOR        JAUNE
    #define OWN_CLAIR_COLOR  JAUNE_C
#elif COULEUR_J1 == ter_M
    #define OWN_COLOR        MAGENTA
    #define OWN_CLAIR_COLOR  MAGENTA_C
#endif

// Couleur du joueur 2 (OPPS)
#if COULEUR_J2 == ter_R
    #define OPPS_COLOR        ROUGE
    #define OPPS_CLAIR_COLOR  ROUGE_C
#elif COULEUR_J2 == ter_B
    #define OPPS_COLOR        BLEU
    #define OPPS_CLAIR_COLOR  BLEU_C
#elif COULEUR_J2 == ter_V
    #define OPPS_COLOR        VERT
    #define OPPS_CLAIR_COLOR  VERT_C
#elif COULEUR_J2 == ter_J
    #define OPPS_COLOR        JAUNE
    #define OPPS_CLAIR_COLOR  JAUNE_C
#elif COULEUR_J2 == ter_M
    #define OPPS_COLOR        MAGENTA
    #define OPPS_CLAIR_COLOR  MAGENTA_C
#endif


// Communication
#define PIN_RX           0
#define PIN_TX           1
#define MAGIC_PAIRING   0xC2


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
#define TBS         2   // Turned Based Strategy - Tour par tour
#define RT          3   // Real-Time


#include <stdint.h>
#include <stdbool.h>

// TYPEDEF -----------------------------------------------------
typedef struct {  // Matrice de LED
    const uint8_t width;
    const uint8_t height;
    uint8_t led[MAT_WIDTH][MAT_HEIGHT][3];    // width/height/channel GRB
} mat_t;


typedef struct {
    uint8_t current_game;               // jeu actuel choisi 
    uint8_t mode;                       // jeu solo/multi/synchrone
    uint8_t current_player;             // qui va jouer le coup suivant
    bool state;                         // 0 en cours - 1 partie terminée
    bool winlose;                       // est-ce que PLAYER1 a gagné ou perdu
    uint8_t printmatrix[9][9];          // matrice à traiter
    uint8_t previous_printmatrix[9][9]; // matrice à traiter du coup d'avant
} game_t;

typedef struct {
    uint8_t state;
    uint8_t prev_state; // etat précédent du bouton - sert à detecter un front montant eviter les rebond etc
    uint8_t pin;
    uint8_t bitshift;
    uint8_t bin;
} btn_t;


// DECLARATION DE FONCTIONS ------------------------------------
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    game_t snake(game_t game_data, uint8_t input);
    game_t megamorpion(game_t game_data, uint8_t input);
    game_t fanorona(game_t game_data, uint8_t input);
    game_t tron(game_t game_data, uint8_t input);
    game_t selector(game_t game_data, uint8_t input); // pour debug
#ifdef __cplusplus
}
#endif // __cplusplus

#endif