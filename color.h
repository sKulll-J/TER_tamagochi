#ifndef COLOR_H
#define COLOR_H

// indice palette qui se trouve dans la printmatrix
#define COL_NOIR       0
#define COL_BLANC      1
#define COL_OWN        2
#define COL_OWN_CLAIR  3
#define COL_OPP        4
#define COL_OPP_CLAIR  5

// couleur réelle à changer selon quelle console on flashe
// (R)OUGE, (B)LEU, (V)ERT, (J)AUNE, (M)AGENTA
#define ter_R 0
#define ter_B 1
#define ter_V 2
#define ter_J 3
#define ter_M 4

#define COULEUR_J1 ter_J
#define COULEUR_J2 ter_M

#define ROUGE     CRGB(  0, 255,   0)
#define ROUGE_C   CRGB(255, 119, 119)
#define BLEU      CRGB(  0,   0, 255)
#define BLEU_C    CRGB(119, 178, 255) 
#define VERT      CRGB(  0, 255,   0)
#define VERT_C    CRGB( 50, 255,  30)
#define JAUNE     CRGB(255, 255,   0)
#define JAUNE_C   CRGB(255, 255,  60)
#define MAGENTA   CRGB(255,   0, 255)
#define MAGENTA_C CRGB(255, 120, 255)

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

// Couleur du joueur 2 (OPP)
#if COULEUR_J2 == ter_R
    #define OPP_COLOR        ROUGE
    #define OPP_CLAIR_COLOR  ROUGE_C
#elif COULEUR_J2 == ter_B
    #define OPP_COLOR        BLEU
    #define OPP_CLAIR_COLOR  BLEU_C
#elif COULEUR_J2 == ter_V
    #define OPP_COLOR        VERT
    #define OPP_CLAIR_COLOR  VERT_C
#elif COULEUR_J2 == ter_J
    #define OPP_COLOR        JAUNE
    #define OPP_CLAIR_COLOR  JAUNE_C
#elif COULEUR_J2 == ter_M
    #define OPP_COLOR        MAGENTA
    #define OPP_CLAIR_COLOR  MAGENTA_C
#endif

#endif