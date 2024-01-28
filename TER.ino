/* INFORMATIONS
 *  Commentaires :
 *      //! = trucs à supprimer
 *      //? = questions pour certains endroits 
 *
 *  Syntaxe :
 *      - Merci d'écrire proprement 
 *      - Les variables qui se rapportent au matériel réel doivent être nommées avec le préfixe "ter" e.g. "termat" pour la matrice de led physique
*/

#include <stdint.h>     // uint8_t
#include <FastLED.h>    // bon cest logique
#include "terlib.h"


// DEFINE ------------------------------------------------------
// PIN des boutons
#define PIN_A            3   // bouton "valider"
#define PIN_B            4   // bouton "annuler"
#define PIN_UP           5   // bouton croix directionnelle "haut"
#define PIN_LEFT         6   // bouton croix directionnelle "gauche"
#define PIN_DOWN         7   // bouton croix directionnelle "bas"
#define PIN_RIGHT        8   // bouton croix directionnelle "droite"
#define PIN_CARTOUCHE_0  9   // pin pour lire quelle "cartouche" est insérée
#define PIN_CARTOUCHE_1  10  // ---
#define PIN_CARTOUCHE_2  11  // ---

#define LED_DATA_PIN    2   // pin sur laquelle transite les données de la matrice
#define COLOR_ORDER     GRB // ordre des couleurs Green-Red-Blue
#define CHIPSET         WS2812  // osef
#define BRIGHTNESS      2   // luminosité réglable


// JE SAIS PAS OU RANGER CE BOUT DE CODE -----------------------
#define NUM_LEDS (MAT_WIDTH * MAT_HEIGHT)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);


// DECLARATION DE FONCTIONS ------------------------------------
static uint8_t XY(uint8_t x, uint8_t y);    // static car la fonction est passée de utin16 à uint8, sauf que 9x9 = 81 on a pas besoin de 16 bits pour aller jusque là ; sauf que la fonction est déjà déclarée dans fastLED, donc la foutre en static la rend accessible seulement ici et pas de warning ^^
uint8_t calcul_coordonnee(uint8_t x, uint8_t y);
void initMatrice(mat_t mat);
void refreshscr(void);
void clearscr(void);


// INIT GLOBAL -------------------------------------------------
game_t tergame = {
    .current_game = NONE,
    .current_player = PLAYER1,
    .state = RUN,
    .printmatrix = {0},
    .winlose = 0,
};

mat_t termat = {    // NOTRE MATRICE
    .width = MAT_WIDTH,
    .height = MAT_HEIGHT,
    .led = {0},
};

uint8_t owninput = 0;
uint8_t oppsinput = 0;
uint8_t terinput = 0;

// ID PIN cartouche
uint8_t IDP = 0;

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
    pinMode(PIN_CARTOUCHE_0, INPUT);
    pinMode(PIN_CARTOUCHE_1, INPUT);
    pinMode(PIN_CARTOUCHE_2, INPUT);

    //! pas sur qu'on ai besoin du serial à part pour debug
    Serial.begin(9600);

    //initMatrice(termat); //? POURQUOI FAIRE CETTE FONCTION SERT A RIEN LOL
}

void loop()
{
    // "Menu principal"
    while (tergame.current_game == NONE)
    {
        /* Ici on utilise les pins de cartouche pour écrire un mot binaire de 3 bits en faisant un CC avec la broche +5V
           Il faut penser à bitshift sinon on overwrite le premier bit
           ? On peut utiliser les pin Analog si jamais on a besoin de plus de pin Digital
        */
        IDP += digitalRead(PIN_CARTOUCHE_0);
        IDP += digitalRead(PIN_CARTOUCHE_1) << 1;
        IDP += digitalRead(PIN_CARTOUCHE_2) << 2;
        switch (IDP) {
            case MEGAMORPION : tergame.current_game = MEGAMORPION;  break;
            case SNAKE :       tergame.current_game = SNAKE;        break;
            case FANORONA :    tergame.current_game = FANORONA;     break;
            case TRON :        tergame.current_game = TRON;         break;
            case NONE : break;
        }       
    }
    tergame.state = RUN;
 
    // Appel à la fonction de jeu
    switch (tergame.current_game) {
        case MEGAMORPION : tergame = megamorpion(tergame, terinput);
                            //! debug
                            Serial.println("jeu : megamorpion");    break;
        case SNAKE :       tergame = snake(tergame, terinput);      break;
        case TRON :        tergame = tron(tergame, terinput);       break;
        case FANORONA :    tergame = fanorona(tergame, terinput);   break;
        case NONE : break;
    }

    terinput = 0;   // efface l'input pour le prochain input


    // interprétation de la matrice reçue qu'il faut update sur l'écran
    for (uint8_t i=0; i<MAT_WIDTH; i++) {
        for (uint8_t j=0; j<MAT_HEIGHT; j++) {
            switch (tergame.printmatrix[i][j]) {
                case LED_NOIR  : leds[XY(i,j)] = CRGB::Black;   break;
                case PLAYER1   : leds[XY(i,j)] = OWN_COLOR;     break;
                case PLAYER2   : leds[XY(i,j)] = OPPS_COLOR;    break;
                case LED_BLANC : leds[XY(i,j)] = CRGB::White;   break;
                default : break;
            }   
        }
    }
        
    if (tergame.state == STOP)
        tergame.current_game = NONE;
}


// FONCTIONS ---------------------------------------------------
/* 
    @fn uint8_t calcul_coordonnee(uint8_t x, uint8_t y)
    @brief transforme la matrice (daisy chained) en un giga vecteur 1D
    @var x [0 - MAT_WIDTH]
    @var y [0 - MAT_HEIGHT]
    @retval i indice du vecteur
*/
uint8_t calcul_coordonnee(uint8_t x, uint8_t y)
{
    uint8_t i;

    // lignes impaires : ordre inversé  
    if (y & 0x01) { // un nombre impair a forcément le premier bit à 1
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
/*
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
*/

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
