/* INFORMATIONS
 *  Commentaires :
 *      //! = trucs à supprimer
 *      //? = questions pour certains endroits 
 *
 *  Syntaxe :
 *      - Merci d'écrire proprement 
 *      - Les variables qui se rapportent au matériel réel doivent être nommées avec le préfixe "ter" e.g. "termat" pour la matrice de led physique
*/

#include <Arduino.h>
#include <stdint.h>     // uint8_t
#include <FastLED.h>    // bon cest logique
#include <time.h>
#include <SoftwareSerial.h>

#include "terlib.h"

#define DEBUG 0

// DEFINE ------------------------------------------------------
// PIN des boutons
#define PIN_CARTOUCHE_0  10   // pin pour lire quelle "cartouche" est insérée
#define PIN_CARTOUCHE_1  11  // ---
#define PIN_CARTOUCHE_2  12  // ---

// FastLED
#define LED_DATA_PIN    13   // pin sur laquelle transite les données de la matrice
#define COLOR_ORDER     GRB // ordre des couleurs Green-Red-Blue
#define CHIPSET         WS2812  // osef
#define BRIGHTNESS      20  // luminosité réglable


// JE SAIS PAS OU RANGER CE BOUT DE CODE -----------------------
#define NUM_LEDS (MAT_WIDTH * MAT_HEIGHT)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);


//SoftwareSerial commSerial(PIN_RX, PIN_TX);

// DECLARATION DE FONCTIONS ------------------------------------
static uint8_t XY(uint8_t x, uint8_t y);    // static car la fonction est passée de uint16 à uint8. 9x9 = 81 on a pas besoin de 16 bits pour aller jusque là ; sauf que la fonction est déjà déclarée dans fastLED, donc la foutre en static la rend accessible seulement ici et pas de warning ^^
uint8_t calcul_coordonnee(uint8_t x, uint8_t y);
void initMatrice(mat_t mat);
void refreshscr(void);
void clearscr(void);
void tererror(uint8_t*** led);
uint8_t readCartouche(void);    // fonction qui lit les 3 bits de cartouche

void ledtoggle(void);


// INIT GLOBAL -------------------------------------------------
game_t tergame = {
    .current_game = NONE,
    .mode = UNDEFINED,
    .current_player = PLAYER2,
    .state = RUN,
    .winlose = 0,
    .printmatrix = {0},
    .game_time = 0,
};

mat_t termat = {
    .width = MAT_WIDTH,
    .height = MAT_HEIGHT,
    .led = {0},
};

// communication/input
uint8_t owninput = 0;
uint8_t oppsinput = 0;
uint8_t terinput = 0;
uint8_t id_random_own;
uint8_t id_random_opps;
uint8_t connected = 0;
// ID PIN cartouche
uint8_t IDP = 0;




// PROGRAMME PRINCIPAL -----------------------------------------
void setup()
{
    // Trucs de fastled
    FastLED.addLeds<CHIPSET, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setMaxPowerInVoltsAndMilliamps(5,500);  //garder absolument
    FastLED.clear();
    FastLED.show();

    // Setup pins
    //pinMode(PIN_RX, INPUT);
    //pinMode(PIN_TX, OUTPUT);
    pinMode(PIN_A,     INPUT_PULLUP);
    pinMode(PIN_B,     INPUT_PULLUP);
    pinMode(PIN_UP,    INPUT_PULLUP);
    pinMode(PIN_LEFT,  INPUT_PULLUP);
    pinMode(PIN_DOWN,  INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_CARTOUCHE_0, INPUT_PULLUP); //met les pins pour detecter que la masse est mise -> 5V fonctionne pas car considéré comme HIGH(toutes tension != 0)
    pinMode(PIN_CARTOUCHE_1, INPUT_PULLUP);
    pinMode(PIN_CARTOUCHE_2, INPUT_PULLUP);

    // Debug Arduino
    pinMode(LED_BUILTIN, OUTPUT);

    
    //#if DEBUG
        Serial.begin(31250);
        //commSerial.begin(9600);
    //#endif

    //initMatrice(termat); //? POURQUOI FAIRE CETTE FONCTION SERT A RIEN LOL
}

void loop()
{
    //#if DEBUG
        Serial.print("\n*** NOUVELLE LOOP ***\n");
    //#endif
    tergame.game_time = millis();//set actual time to game time

    Serial.println(tergame.game_time);

    /*  Séquence de la gameloop:
        1. choix du jeu
        2. initialisation communication RX/TX
            2.1 appairage des deux consoles
            2.2 qui commence en premier ? (si jeu tour par tour)
        3. appel à la fonction de jeu
        4. execution de la fonction de jeu
        5. render de la matrice
    */

    // Choix du jeu
    if ((7-readCartouche()) != IDP) { // permet de réinit la console si on enleve la cartouche (feature demandée par erwann)
        while (tergame.current_game == NONE)
        {
            /* Ici on utilise les pins de cartouche pour écrire un mot binaire de 3 bits en faisant un CC avec la broche +5V
            Il faut penser à bitshift sinon on overwrite le premier bit
            ? On peut utiliser les pin Analog si jamais on a besoin de plus de pin Digital
            */
            IDP = 7-readCartouche();

            #if DEBUG
                Serial.print("ID = ");
                Serial.print(IDP);
                Serial.print("\n");
            #endif

            switch (IDP) {
                case MEGAMORPION:   tergame.current_game = MEGAMORPION;
                                    tergame.mode = TBS;
                                    break; 
                case FANORONA:  tergame.current_game = FANORONA;    
                                tergame.mode = TBS;
                                break;
                case SNAKE: tergame.current_game = SNAKE;
                            tergame.mode = SOLO;
                            break;
                case TRON:  tergame.current_game = TRON;
                            tergame.mode = RT;
                            break;
                default: break;
            }
        }
    }
    tergame.state = RUN;

 /*
    // Communication RX/TX
    if ((tergame.mode != SOLO) && (!connected)) {
        // Appairage
        // Envoit en boucle la data MAGIC_PAIRING jusqu'à ce que OWN reçoit ce meme signal de la part de OPPS
        while (Serial.read() != MAGIC_PAIRING) {
            Serial.write(MAGIC_PAIRING);
        }
        connected = 1;

        // Qui commence ?
        if (tergame.mode == TBS) {      // il faut probablement des Serial.available() la dedans 
            id_random_own = rand();
            Serial.write(id_random_own);
            id_random_opps = Serial.read();

            if (id_random_own > id_random_opps)
                tergame.current_player = PLAYER1;
            else 
                tergame.current_player = PLAYER2;
        }
    }
*/
    // Gestion des inputs
    if (tergame.mode == SOLO) {     // se répete à chaque passage de la game loop
        terinput = readinput();
        delay(100);   // Limite le frame rate sinon ça va trop vite ; même problème pour les jeux RT... mais pour ça un delay bloquant est impensable
    }
    else if (tergame.mode == TBS) {  // mode bloquant pour les jeux tour par tour
        while (terinput == 0) {
            if (tergame.current_player == PLAYER1) {       // à mon tour de jouer
                terinput = readinput();
                //? il manque l'envoit de la donnée
            } 
            else if (tergame.current_player == PLAYER2) {  // au tour de l'adversaire - la condition en commentaire économise 2 octets
                if (Serial.available() > 0) {
                    terinput = Serial.read();
                }
            }
        }
    }
    else if (tergame.mode==RT)
    {
        if (tergame.current_player == PLAYER1) {       // à mon tour de jouer
                tergame.current_player = PLAYER2;
                terinput = readinput();
                Serial.write(terinput);
                //? il manque l'envoit de la donnée
            } 
        else if (tergame.current_player == PLAYER2) {  // au tour de l'adversaire - la condition en commentaire économise 2 octets
            tergame.current_player = PLAYER1;
            if (Serial.available() > 0) {
                terinput = Serial.read();
            }
            else
            {
                terinput = 0;
            }
        }
    }
    
        Serial.println(terinput, BIN);

        Serial.print("PLAYER");
        Serial.println(tergame.current_player);
    #if DEBUG
        Serial.print("Input : ");
        Serial.print(terinput, BIN);
        Serial.print("\n");
    #endif

    // Appel à la fonction de jeu
    switch (tergame.current_game) {
        case MEGAMORPION:   tergame = megamorpion(tergame, terinput);   break;  // 2530 octets 
        case SNAKE:         tergame = snake(tergame, terinput);         break;  // 1274 octets
        case TRON:          tergame = tron(tergame, terinput);          break;
        case FANORONA:      tergame = fanorona(tergame, terinput);      break;
        case NONE: break;
        default: break;
    }

  

    terinput = 0;   // efface l'input pour le prochain input

    // Interprétation de la matrice reçue qu'il faut update sur l'écran
    for (uint8_t i=0; i<MAT_WIDTH; i++) {
        for (uint8_t j=0; j<MAT_HEIGHT; j++) {
            switch (tergame.printmatrix[i][j]) {
                case COL_NOIR:  leds[XY(i,j)] = CRGB::Black; break;
                case COL_BLANC: leds[XY(i,j)] = CRGB::White; break;
                case COL_OWN:   leds[XY(i,j)] = OWN_COLOR;   break;
                case COL_OPPS:  leds[XY(i,j)] = OPPS_COLOR;  break;
                case COL_OWN_CLAIR:  leds[XY(i,j)] = OWN_CLAIR_COLOR;  break;
                case COL_OPPS_CLAIR: leds[XY(i,j)] = OPPS_CLAIR_COLOR; break;
                default: break;
            }   
        }
    }
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show(); //permet d'allumer les leds

    

    if (tergame.state == STOP) {
        #if DEBUG
            Serial.print("-----------STOP GAME-----------\n");
        #endif
        clearscr();
        tergame.current_game = NONE;
    }

    #if DEBUG
        Serial.print("*** FIN DE LA LOOP ***\n");
    #endif
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
            tergame.printmatrix[x][y] = COL_NOIR ; 
        }
    }
}


void tererror(uint8_t*** led) // appeller la fonction avec terreror(mat.led);
{
    for(int x=0; x<9; x++) {
        for(int y=0 ; y<9; y++) {
            led[x][y][0] = 0;
            led[x][y][1] = 255;
            led[x][y][2] = 0;
        }
    }
}

uint8_t readCartouche(void)
{
    uint8_t IDP = 0;
    IDP |= digitalRead(PIN_CARTOUCHE_0);
    IDP |= digitalRead(PIN_CARTOUCHE_1) << 1;
    IDP |= digitalRead(PIN_CARTOUCHE_2) << 2;
    return IDP;
}

void ledtoggle(void)
{
    static uint8_t mode = 1;

    mode = !mode;
    digitalWrite(LED_BUILTIN, mode);
}