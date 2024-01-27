/* INFORMATIONS
 *  Commentaires :
 *      //! = trucs à supprimer
 *      //? = questions pour certains endroits 
 *
 *  Syntaxe :
 *      - Merci d'écrire proprement 
 *      - Les variables qui se rapportent au matériel réel doivent être nommées avec le préfixe "ter" e.g. "termat" pour la matrice de led physique
*/

#include <stdint.h>
#include <FastLED.h>
#include "terlib.h"


// DEFINE ------------------------------------------------------
// PIN des boutons
#define PIN_A           4   // bouton "valider"
#define PIN_B           4   // bouton "retour"
#define PIN_UP          6   // bouton croix directionnelle "haut"
#define PIN_LEFT        7   // bouton croix directionnelle "gauche"
#define PIN_DOWN        8   // bouton croix directionnelle "bas"
#define PIN_RIGHT       5   // bouton croix directionnelle "droite"

#define LED_DATA_PIN    2   // pin sur laquelle transite les données de la matrice
#define COLOR_ORDER     GRB // ordre des couleurs Green-Red-Blue
#define CHIPSET         WS2812  // osef
#define BRIGHTNESS      2   // luminosité réglable


// JE SAIS PAS OU RANGER CE BOUT DE CODE -----------------------
#define NUM_LEDS (MAT_WIDTH * MAT_HEIGHT)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);


// DECLARATION DE FONCTIONS ------------------------------------
uint8_t XY (uint8_t x, uint8_t y);
uint8_t calcul_coordonnee(uint8_t x, uint8_t y);
void initMatrice(mat_t mat);
void refreshscr(void);
void clearscr(void);


// INIT GLOBAL -------------------------------------------------
game_t tergame = {
    .current_game = NONE,
    .current_player = 0,
    .state = 0,     // en cours
};

mat_t termat = {    // NOTRE MATRICE
    .width = MAT_WIDTH,
    .height = MAT_HEIGHT,
    .led = {0},
};


// PROGRAMME PRINCIPAL -----------------------------------------
void setup()
{
    // truc de fastled
    FastLED.addLeds<CHIPSET, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setMaxPowerInVoltsAndMilliamps(5,500);  //garder absolument
    FastLED.clear();
    FastLED.show();

    // setup pins
    pinMode(PIN_A,     INPUT_PULLUP);
    pinMode(PIN_B,     INPUT_PULLUP);
    pinMode(PIN_UP,    INPUT_PULLUP);
    pinMode(PIN_LEFT,  INPUT_PULLUP);
    pinMode(PIN_DOWN,  INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);

    //! pas sur qu'on ai besoin du serial à part pour debug
    Serial.begin(9600);

    initMatrice(termat); //? POURQUOI FAIRE CETTE FONCTION SERT A RIEN LOL

}

void loop()
{

    // menu principal (pour linstant)
    while(game.current == NONE)
    {
        // choix du jeu avec <- -> et A
        //...
        // ex :
        game.current = SNAKE;
    }


    // loop de jeu
    while(1)
    {
        switch tergame {
            case MEGAMORPION : g_megamorpion(termat, owninput, oppsinpunt);
            case SNAKE : g_snake(termat, owninput, oppsinput);
            case TRON : g_tron(termat, owninput, oppsinput);
            case FANORONA : g_fanorona(termat, owninput, oppsinput);
        }
        
        if (tergame.)
        break;
    }

}


// FONCTIONS ---------------------------------------------------
/*  @fn uint8_t calcul_coordonnee(uint8_t x, uint8_t y)
 *  @brief transforme la matrice (daisy chained) en un giga vecteur 1D
 *  @var x [0 - MAT_WIDTH]
 *  @var y [0 - MAT_HEIGHT]
 *  @retval i indice du vecteur
 */
uint8_t calcul_coordonnee(uint8_t x, uint8_t y)
{
    uint8_t i;

    // lignes impaires : ordre inversé  
    //? bombre impair a forcément le premier bit à 1
    if (y & 0x01) {                 
        i = (y * MAT_WIDTH) + MAT_WIDTH - x - 1;
    // lignes paires : ordre normal 
    } else {                        
        i = (y * MAT_WIDTH) + x;
    }
      
    return i;
}

//? execute le calcul de coordonnées que si cest une coordonnée valable 
uint8_t XY(uint8_t x, uint8_t y)
{
    if (x >= MAT_WIDTH)  return -1; // dangereux car si on veut acceder l'indice -1 du gigavecteur ca tombe en dehors !! à voir si ya vrm besoin de la protec en fin de compte
    if (y >= MAT_HEIGHT) return -1;
    return calcul_coordonnee(x,y);
}


//? initialise l'écran à du full blanc partout
//! sert a rien a suprimer
void initMatrice(mat_t mat)
{
    for(int x=0; x<9; x++) {
        for(int y=0 ; y<9; y++) {
            mat.led[x][y][0] = 255;
            mat.led[x][y][1] = 255;
            mat.led[x][y][2] = 255;
        }
    }
}

//? actualise chaque channel RGB ? je crois
void refreshscr(void)
{
    for (int x=0; x<9; x++) {
        for (int y=0; y<9; y++) {
            leds[XY(x,y)] = CRGB(termat.led[x][y][0],
                                 termat.led[x][y][1],
                                 termat.led[x][y][2]);
        }
    }
}

/*  @fn void clearscr(void)
 *  @brief Eteint tout les pixels de la matrice
 *  @retval NONE
 */
void clearscr(void)
{
    for(int x=0; x<9; x++) {
        for(int y=0; y<9; y++) {
            leds [XY(x,y)] = CRGB::Black;
        }
    }
}
