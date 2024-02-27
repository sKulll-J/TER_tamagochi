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
#include <stdlib.h>
#include "terlib.h"

#define DEBUG_input true
#define DEBUG true

// DEFINE ------------------------------------------------------
#define TICK_RATE 400        // milliseconds, temps de 1 frame (60fps = 16.6ms)

// PIN des boutons
#define PIN_CARTOUCHE_0  9   // pin pour lire quelle "cartouche" est insérée
#define PIN_CARTOUCHE_1  10  // ---
#define PIN_CARTOUCHE_2  11  // ---

// FastLED
#define LED_DATA_PIN    13   // pin sur laquelle transite les données de la matrice
#define COLOR_ORDER     GRB // ordre des couleurs Green-Red-Blue
#define CHIPSET         WS2812  // osef
#define BRIGHTNESS      20  // luminosité réglable


// JE SAIS PAS OU RANGER CE BOUT DE CODE -----------------------
#define NUM_LEDS (MAT_WIDTH * MAT_HEIGHT)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);


// DECLARATION DE FONCTIONS ------------------------------------
static uint8_t XY(uint8_t x, uint8_t y);    // static car la fonction est passée de uint16 à uint8. 9x9 = 81 on a pas besoin de 16 bits pour aller jusque là ; sauf que la fonction est déjà déclarée dans fastLED, donc la foutre en static la rend accessible seulement ici et pas de warning ^^
uint8_t calcul_coordonnee(uint8_t x, uint8_t y);
void initMatrice(mat_t mat);
void refreshscr(void);
void clearscr(void);
void tererror(uint8_t*** led);
uint8_t readCartouche(void);    // fonction qui lit les 3 bits de cartouche

void ledtoggle(void); // juste pour test ca sert un peu à rien ce truc


// INIT GLOBAL -------------------------------------------------
unsigned long time_now = 0;
unsigned long time_last = 0;

game_t tergame = {
    .current_game = NONE,
    .mode = UNDEFINED,
    .current_player = PLAYER1,
    .state = RUN,
    .winlose = 0,
    .printmatrix = {0},
    .previous_printmatrix = {0},
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
uint8_t input_buffer[16] = {0};   // sert à stocker de multiples inputs avant le render d'une frame
uint8_t input_counter = 0;

uint8_t id_random_own;
uint8_t id_random_opps;
uint8_t connected = 0;
// ID PIN cartouche
uint8_t IDP = 255;  // valeur impossible à avoir avec 3 bits

// PROGRAMME PRINCIPAL -----------------------------------------
void setup()
{
    // Trucs de fastled
    FastLED.addLeds<CHIPSET, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setMaxPowerInVoltsAndMilliamps(5,500);  //garder absolument
    FastLED.clear();
    FastLED.show();

    // Setup pins
    pinMode(PIN_RX, INPUT);
    pinMode(PIN_TX, OUTPUT);
    pinMode(PIN_A,     INPUT_PULLUP);
    pinMode(PIN_B,     INPUT_PULLUP);
    pinMode(PIN_UP,    INPUT_PULLUP);
    pinMode(PIN_LEFT,  INPUT_PULLUP);
    pinMode(PIN_DOWN,  INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_CARTOUCHE_0, INPUT); // surtout pas en INPUT_PULLUP, sinon quand on débranche la cartouche on voudrait 000 et on aurait 111
    pinMode(PIN_CARTOUCHE_1, INPUT);
    pinMode(PIN_CARTOUCHE_2, INPUT);

    // Debug Arduino
    pinMode(LED_BUILTIN, OUTPUT);

    #if DEBUG
        Serial.begin(9600);
        Serial.println("Salut! (9600)");
    #endif
    // 16/31250 / 16 bit/s => 0.51 ms à trasmettre 2 octets
}

// LOOP ----------------------------------------------------
void loop()
{
    /*  Séquence de la gameloop:
        1. choix du jeu
        2. initialisation communication RX/TX
            2.1 appairage des deux consoles
            2.2 qui commence en premier ? (si jeu tour par tour)
        3. appel à la fonction de jeu
        4. execution de la fonction de jeu
        5. render de la matrice
    */

    // Initialisation
    if (readCartouche() != IDP) { // permet de réinit la console si on enleve la cartouche (feature demandée par erwann)
        #if DEBUG
            Serial.print("[?] IDP Change:\t");
        #endif

        if (tergame.current_game != NONE) tergame.current_game = NONE;  // alambiqué mais c'est malin
        while (tergame.current_game == NONE)
        {
            /* Ici on utilise les pins de cartouche pour écrire un mot binaire de 3 bits en faisant un CC avec la broche +5V
            Il faut penser à bitshift sinon on overwrite le premier bit
            ? On peut utiliser les pin Analog si jamais on a besoin de plus de pin Digital
            */
            IDP = readCartouche();
            #if DEBUG
                Serial.print("ID = ");
                Serial.print(IDP);
            #endif

            switch (IDP) {
                case SNAKE:         tergame.current_game = SNAKE;
                                    tergame.mode = SOLO;
                                    tergame.state = RUN;
                                    #if DEBUG
                                        Serial.println("\tSNAKE");
                                    #endif
                                    break;

                case MEGAMORPION:   tergame.current_game = MEGAMORPION;
                                    tergame.mode = TBS;
                                    tergame.state = RUN;
                                    //tergame = megamorpion2(tergame, INPUT_INIT);
                                    #if DEBUG
                                        Serial.println("\tMEGAMORPION");
                                    #endif
                                    break; 

                case FANORONA:      tergame.current_game = FANORONA;    
                                    tergame.mode = TBS;
                                    tergame.state = RUN;
                                    #if DEBUG
                                        Serial.println("\tFANORONA");
                                    #endif
                                    break;

                case TRON:          tergame.current_game = TRON;
                                    tergame.mode = RT;
                                    tergame.state = RUN;
                                    #if DEBUG
                                        Serial.println("\tTRON");
                                    #endif
                                    break;

                case SELECTOR:      tergame.current_game = SELECTOR;
                                    tergame.mode = SOLO;
                                    tergame.state = RUN;
                                    #if DEBUG
                                        Serial.println("\tSELECTOR");
                                    #endif
                                    break;
                default: 
                                    #if DEBUG
                                        Serial.print("\n");
                                    #endif
                                    break;
            }
        }
    }

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
    // Début de la loop
    #if DEBUG
        //Serial.println("*** NOUVELLE LOOP ***");
    #endif


    time_now = millis();
    if (time_now - time_last > TICK_RATE)
    {
        
        // UPDATE
        for(uint8_t n = 0; n <= input_counter; n++) {

            #if DEBUG
                Serial.print("[!] UPDATE (");
                Serial.print(n+1);
                Serial.print(") : ");
            
                switch (tergame.current_game) {
                    case SNAKE:       Serial.println("SNAKE");      break;
                    case MEGAMORPION: Serial.println("MEGAMORPION");break;
                    case FANORONA:    Serial.println("FANORONA");   break;
                    case TRON:        Serial.println("TRON");       break;
                    case SELECTOR:    Serial.println("SELECTOR");   break;
                    case NONE:        Serial.println("NONE");       break;
                }
            #endif

            if (tergame.current_game == SNAKE)              tergame = snake(tergame, terinput);         // 1274 octets
            //else if (tergame.current_game == MEGAMORPION)   tergame = megamorpion(tergame, terinput);   // jsp  octets
            else if (tergame.current_game == FANORONA)      tergame = fanorona(tergame, terinput);       
            //else if (tergame.current_game == TRON)          tergame = tron(tergame, owninput, oppsinput);
            else if (tergame.current_game == SELECTOR)      tergame = selector(tergame, input_buffer[n]);       
            //case NONE: break;
            //default:    Serial.println("***BREAK***");break;

        
            
            // RENDER
            #if DEBUG
                Serial.println("[!] RENDER");
            #endif
            
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
            
            #if DEBUG
                Serial.println("FIN RENDER");
            #endif
        
            input_buffer[n] = 0;    // vidage des couilles
            delay(5);
        }
        input_counter = 0;
        time_last = time_now;
        //terinput = 0;
    }


    // Gestion des inputs
    if (readinput() != 0) {
        input_buffer[input_counter] = readinput();
        input_counter++;
        //terserial.print(owninput);

        if (tergame.mode != SOLO) {  // mode MULTI
            //if (terserial.available() > 0) {
            //    oppsinput = terserial.read();
            //}
        }
        
        // Attribution des inputs own/opps
        if (tergame.mode == TBS) {
            if (tergame.current_player == PLAYER1)
                terinput = owninput;
            else if (tergame.current_player == PLAYER2)
                terinput = oppsinput;
            
        }
    }
    
    #if DEBUG_input
            Serial.print("[?] INPUT > ");
            Serial.print(input_buffer[input_counter]);
            Serial.print("(");
            Serial.print(input_counter);
            Serial.println(")");
    #endif

    
    // Fin du jeu
    // TODO: faire blinker la matrice de la couleur du gagnant pour signifier la victoire (avec un delay entre le jeu et l'ecran de victoire)
    if (tergame.state == STOP) {
        #if DEBUG
            Serial.println("-----------STOP GAME-----------");
        #endif
        clearscr();
        tergame.current_game = NONE;
    }

    
    delay(50);  // avoid busy waiting

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
            leds[XY(x,y)] = CRGB::Black;
        }
    }
    FastLED.show();
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

///////////////////////////////////////////////////////////////////////////


