#include "terlib.h"
#include "pin.h"
#include "color.h"

#include <Arduino.h>
#include <stdint.h>         // uint8_t
#include <FastLED.h>        // bon cest logique
#include <SoftwareSerial.h> // RX TX

#define DEBUG        true
#define DEBUG_input  true
#define DEBUG_screen true

// DEFINE ------------------------------------------------------
#define TICK_RATE 200        // milliseconds, temps de 1 frame (60fps = 16.6ms)
SoftwareSerial terserial(PIN_RX, PIN_TX); // RX, TX

// FastLED
#define COLOR_ORDER     GRB // ordre des couleurs Green-Red-Blue
#define CHIPSET         WS2812  // osef
#define BRIGHTNESS      20  // luminosité réglable
#define NUM_LEDS (MAT_WIDTH * MAT_HEIGHT)
CRGB leds_plus_safety_pixel[NUM_LEDS + 1];
CRGB* const leds(leds_plus_safety_pixel + 1);


// DECLARATION DE FONCTIONS ------------------------------------
static uint8_t XY(uint8_t x, uint8_t y);    // static car la fonction est passée de uint16 à uint8. 9x9 = 81 on a pas besoin de 16 bits pour aller jusque là ; sauf que la fonction est déjà déclarée dans fastLED, donc la foutre en static la rend accessible seulement ici et pas de warning ^^
uint8_t calcul_coordonnee(uint8_t x, uint8_t y);
uint8_t readCartouche(void);    // fonction qui lit les 3 bits de cartouche
void update(struct game_s *game, uint8_t input);
void render(struct game_s game);


// INIT GLOBAL -------------------------------------------------
unsigned long time_now = 0;
unsigned long time_last = 0;

struct game_s tergame = {
    .current_game = NONE,
    .mode = UNDEFINED,
    .function = selector,
    .current_player = PLAYER1,
    .state = RUN,
    .winlose = 0,
    .printmatrix = {0},
    .previous_printmatrix = {0},
    .game_time = 0,
};

mat_t termat = {
    .width = MAT_WIDTH,
    .height = MAT_HEIGHT,
    .led = {0},
};

// input
uint8_t data_input = 0;
uint8_t own_input = 0;
uint8_t opp_input = 0;
uint8_t terinput = 0;

// communication
uint8_t id_random_own = 0;
uint8_t id_random_opp = 0;
bool connected = false;

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
    pinMode(PIN_RANDOM, INPUT);
    randomSeed(analogRead(PIN_RANDOM));

    pinMode(PIN_A, INPUT_PULLUP);
    pinMode(PIN_B, INPUT_PULLUP);
    pinMode(PIN_U, INPUT_PULLUP);
    pinMode(PIN_L, INPUT_PULLUP);
    pinMode(PIN_D, INPUT_PULLUP);
    pinMode(PIN_R, INPUT_PULLUP);
    pinMode(PIN_CARTOUCHE_0, INPUT); // surtout pas en INPUT_PULLUP, sinon quand on débranche la cartouche on voudrait 000 et on aurait 111
    pinMode(PIN_CARTOUCHE_1, INPUT);
    pinMode(PIN_CARTOUCHE_2, INPUT);

    // Interruptions
    attachInterrupt(digitalPinToInterrupt(PIN_A), handle_A, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_B), handle_B, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_U), handle_U, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_L), handle_L, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_D), handle_D, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_R), handle_R, RISING);

    #if DEBUG   
        // Debug Arduino
        pinMode(LED_BUILTIN, OUTPUT);

        Serial.begin(9600);
        Serial.println("Salut! (9600)");
        delay(500);
    #endif

    terserial.begin(9600);
}

// LOOP ----------------------------------------------------
void loop()
{
    /*  Séquence de la gameloop:
     *  1. détection du jeu via les pin de cartouche
     *  2. si le jeu est SEQUENTIEL (tour par tour) = MEGAMORPION, FANORONA
     *      a. qui commence en premier ? (au hasard)
     *      b. quand c'est mon tour:
     *          -> les IT sur les input appellent la fonction de jeu (update) et affichent le resultat (render)
     *          -> mon input est envoyé sur le port série à l'autre console
     *      c. quand c'est le tour de l'adversaire:
     *          -> scrutation buffer de reception série pour l'input adverse
     *          -> update + render
     * 
     *  3. si le jeu est SYNCHRONE (le jeu peu avancer de lui meme sans les input) = SNAKE, TRON
     *      a. update + render à chaque tick (60 fps par exemple)
     *      b. lorsqu'il n'est pas encore l'heure de update + render:
     *          -> scrutation buffer de reception série pour l'input adverse
     *          -> les IT permettent d'enregistrer mon input et envoit sur port série
     *          -> les inputs seront prit en compte lors du prochain render
     */

    // INITIALISATION
    if (readCartouche() != IDP) { // permet de réinit la console si on enleve la cartouche (feature demandée par erwann)
        #if DEBUG
            Serial.print("[?] IDP Change:\t");
        #endif

        if (tergame.current_game != NONE) tergame.current_game = NONE;  // alambiqué mais c'est malin
        while (tergame.current_game == NONE)
        {
            /* Ici on utilise les pins de cartouche pour écrire un mot binaire de 3 bits en faisant un CC avec la broche +5V
             *Il faut penser à bitshift sinon on overwrite le premier bit
             *? On peut utiliser les pin Analog si jamais on a besoin de plus de pin Digital
             */
            IDP = readCartouche();
            #if DEBUG
                Serial.print("ID = ");
                Serial.print(IDP);
                switch(IDP) {
                    case SNAKE: Serial.println("\tSNAKE"); break;
                    case MEGAMORPION: Serial.println("\tMEGAMORPION"); break;
                    case FANORONA: Serial.println("\tFANORONA"); break;
                    case TRON: Serial.println("\tTRON"); break;
                    case SELECTOR: Serial.println("\tSELECTOR"); break;
                    default: Serial.print("\n"); break;
                }
            #endif

            switch (IDP) {
                // JEU: tergame = (cast struct){JEU, SEQ/SYNC, pointeur de fonction}
                case SNAKE: tergame = (struct game_s){SNAKE, SOLO, snake}; break;
                case MEGAMORPION: tergame = (struct game_s){MEGAMORPION, SEQUENTIEL, megamorpion}; break;
                case FANORONA: tergame = (struct game_s){FANORONA, SEQUENTIEL, fanorona}; break;
            
                case TRON: tergame = (struct game_s){TRON, SOLO, tron}; break;
                case SELECTOR: tergame = (struct game_s){SELECTOR, SEQUENTIEL, selector}; break;
                
                default: break;
            }
            tergame.state = RUN;
        }
    }

    // GAMELOOP SEQUENTIEL 
    if (tergame.mode == SEQUENTIEL) {   // uniquement pour les tour par tour
        // Communication RX/TX - Attribution qui joue en premier (generation de nombre random puis comparaison de qui a la plus grosse)
        if (connected == false) {
            #if DEBUG
                Serial.println("[X] Connexion...");
            #endif
            
            id_random_own = random(100, 254);    // random  ]0-255[
            while (((id_random_opp == 0) || (id_random_opp == 255))) {  // condition lunaire inexplicable : RX reste à 255 si on branche rien, ou bien reste à 0 si on met pas la condition. Appelez Sherlock Holmes
                terserial.write(id_random_own);
                id_random_opp = terserial.read();

                if (id_random_own == id_random_opp) { // si par malheur on a généré le meme nombre, on regenere et on recommence la loop
                    id_random_own = random(100, 254); 
                    continue;
                } 

                #if DEBUG
                    Serial.print("  [X] ID own/opp: (");
                    Serial.print(id_random_own); 
                    Serial.print("/");
                    switch (id_random_opp) {
                        case 0:   Serial.println("---)"); break;
                        case 255: Serial.println("---)"); break;
                        default: 
                            Serial.print(id_random_opp);
                            Serial.println(")");
                            break;
                    }
                #endif
                delay(500);
            }

            while(terserial.available() > 0) {  // on vide le buffer de reception si jamais on a reçu plus d'une fois le nombre de l'autre console
                terserial.read();
            }

            tergame.current_player = (id_random_own > id_random_opp) ? PLAYER1 : PLAYER2;
            connected = true;
        
            #if DEBUG
                Serial.println("[X] Connexion réussie.");
                Serial.print("  [X] own ID: "); Serial.println(id_random_own);
                Serial.print("  [X] opp ID: "); Serial.println(id_random_opp);
                (id_random_own > id_random_opp) ? Serial.println("[X] OWN commence") : Serial.println("[X] OPP commence") ;
                Serial.println("[#] DEBUT DE LA LOOP");
            #endif

            // Met un coup d'update sinon il attend un input pour afficher un truc sur la matrice            
            update(&tergame, (id_random_own > id_random_opp) ? id_random_own : id_random_opp);
            render(tergame);
        }

        // Reception serie
        if (tergame.current_player == PLAYER2) {
            #if DEBUG
                Serial.print("[@] Oui allo jecoute");
            #endif
            while (1) {
                if (terserial.available() > 0) {
                    opp_input = terserial.read();
                    break;
                }
            }
            #if DEBUG
                Serial.print("[@] Oui allo jai entendu");
                Serial.print("[+] INPUT (OPP): ");
                switch(opp_input) {
                    case INPUT_U: Serial.println("[^]");  break;
                    case INPUT_D: Serial.println("[v]");  break;
                    case INPUT_L: Serial.println("[<]");  break;
                    case INPUT_R: Serial.println("[>]");  break;
                    case INPUT_A: Serial.println("[A]");  break;
                    case INPUT_B: Serial.println("[B]");  break;
                    default:      Serial.println("[ ]");  break;
                }
            #endif

            update(&tergame, opp_input);
            render(tergame);
        }
    }

    // GAMELOOP SYNCHRONE
    if (tergame.mode == SYNCHRONE) {
        time_now = millis();
        if ((time_now - time_last) > TICK_RATE) {   // remplacer TICK par tergame.tick pour qui'l soit modifiable (vitesse du Tron)
            update(&tergame, own_input);
            update(&tergame, opp_input);
            render(tergame);

            time_last = time_now;
            own_input = 0;
            opp_input = 0;
        }
    
        // Reception serie de l'input adverse
        if (tergame.mode != SOLO) {
            opp_input = terserial.read();
        }
    }
    
    // Fin du jeu
    // TODO: faire blinker la matrice de la couleur du gagnant pour signifier la victoire (avec un delay entre le jeu et l'ecran de victoire)
    if (tergame.state == ter_STOP) {
        #if DEBUG
            Serial.print("-----------STOP GAME-----------\n");
        #endif
        //clearscr();
        if(tergame.winlose==WIN)
        {
            for (uint8_t i=0; i<9; i++) {
                for (uint8_t j=0; j<9; j++) {   
                    leds[XY(i,j)]=CRGB::Red;
                }
            }
        }        
        if(tergame.winlose==LOSE)
        {
            for (uint8_t i=0; i<9; i++) {
                for (uint8_t j=0; j<9; j++) {   
                    leds[XY(i,j)]=CRGB::White;
                }
            }
        }
        FastLED.show();
        tergame.current_game = NONE;
    }

}


// FONCTIONS ---------------------------------------------------
/** 
 *  @brief transforme la matrice (daisy chained) en un vecteur 1D
 *  @param x [0 - MAT_WIDTH]
 *  @param y [0 - MAT_HEIGHT]
 *  @retval i indice du vecteur
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

/**
 *  @brief execute le calcul de coordonnées que si cest une coordonnée valable
 *  @param x indice horizontal
 *  @param y indice vertical
 *  @retval l'indice de la led visée
 */
uint8_t XY(uint8_t x, uint8_t y)
{
    if (x >= MAT_WIDTH)  return -1; // dangereux car si on veut acceder l'indice -1 du gigavecteur ca tombe en dehors !! à voir si ya vrm besoin de la protec en fin de compte
    if (y >= MAT_HEIGHT) return -1;
    return calcul_coordonnee(x,y);
}


/**
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

/**
 *  @brief Lit les 3 pins de cartouche pour déterminer quel jeu utiliser 
 *  @retval mot binaire de 3 bits correspondant au jeu
 */
uint8_t readCartouche(void)
{
    uint8_t IDP = 0;
    IDP |= digitalRead(PIN_CARTOUCHE_0);
    IDP |= digitalRead(PIN_CARTOUCHE_1) << 1;
    IDP |= digitalRead(PIN_CARTOUCHE_2) << 2;
    return IDP;
}

/**
 *  @brief Fonction de mise à jour du jeu selon l'input. Elle appelle la fonction de jeu pointée par game.function
 *  @param game structure du jeu en cours
 *  @param input l'input récupéré
 */
void update(struct game_s *game, uint8_t input) 
{
    #if DEBUG
        Serial.print("[!] UPDATE");  
        switch (tergame.current_game) {
            case SNAKE:       Serial.print("SNAKE");      break;
            case MEGAMORPION: Serial.print("MEGAMORPION");break;
            case FANORONA:    Serial.print("FANORONA");   break;
            case TRON:        Serial.print("TRON");       break;
            case SELECTOR:    Serial.print("SELECTOR");   break;
            case NONE:        Serial.print("NONE");       break;
        }
    #endif

    *game = tergame.function(*game, input);
}

/**
 *  @brief Compare une à une toutes les cases de la matrice de jeu avec les couleurs de la palette de couleur définie dans color.h
 *  @param game structure du jeu en cours
 */
void render(struct game_s game)
{
    #if DEBUG_screen
        Serial.println(" [!] RENDER");
        uint8_t i, j;
        for (i=0; i<8; i++) {
            for (j=0; j<=8; j++) {
                if (tergame.printmatrix[i][j] == COL_NOIR) Serial.print(".");
                else Serial.print("O");
            }
            Serial.println();
        }
    #endif

    for (uint8_t i=0; i<MAT_WIDTH; i++) {
        for (uint8_t j=0; j<MAT_HEIGHT; j++) {    
            switch (game.printmatrix[i][j]) {
                case COL_NOIR:      leds[XY(i,j)] = CRGB::Black;     break;
                case COL_BLANC:     leds[XY(i,j)] = CRGB::White;     break;
                case COL_OWN:       leds[XY(i,j)] = OWN_COLOR;       break;
                case COL_OPP:       leds[XY(i,j)] = OPP_COLOR;       break;
                case COL_OWN_CLAIR: leds[XY(i,j)] = OWN_CLAIR_COLOR; break;
                case COL_OPP_CLAIR: leds[XY(i,j)] = OPP_CLAIR_COLOR; break;
                default: break;
            }
        }
    }
    FastLED.show();
}


// INTERRUPTIONS -----------------------------------------------
void handle_A() { handle_input(INPUT_A); }
void handle_B() { handle_input(INPUT_B); }
void handle_U() { handle_input(INPUT_U); }
void handle_L() { handle_input(INPUT_L); }
void handle_D() { handle_input(INPUT_D); }
void handle_R() { handle_input(INPUT_R); }
/**
 *  @brief Fonction appellée via chaque interruption. Décide des actions à effectuer lors des appuis de bouton selon le type de jeu
 *  @param input_x l'input considéré (INPUT_A, INPUT_B, ...)
 */
void handle_input(uint8_t input_x)
{
    #if DEBUG
        Serial.print("[+] INPUT (OWN): ");
        switch(input_x) {
            case INPUT_U: Serial.println("[^]");  break;
            case INPUT_D: Serial.println("[v]");  break;
            case INPUT_L: Serial.println("[<]");  break;
            case INPUT_R: Serial.println("[>]");  break;
            case INPUT_A: Serial.println("[A]");  break;
            case INPUT_B: Serial.println("[B]");  break;
            default:      Serial.println("[ ]");  break;
        }
    #endif

    if (tergame.mode == SOLO) {
        update(&tergame, input_x);
        render(tergame);
    }

    if (tergame.mode == SYNCHRONE) {
        own_input = input_x;
        terserial.write(input_x);
        return;
    }
    
    if ((tergame.mode == SEQUENTIEL) && (tergame.current_player == PLAYER1)) { // SEQUENTIEL et c'est mon tour
        terserial.write(input_x); // Transmission serie
        update(&tergame, input_x);
        render(tergame);
    }
}

// TRANSITIONS -------------------------------------------------
void tr_clignote(uint8_t R, uint8_t G, uint8_t B)
{
    for (int i=0; i<3; i++) {   // 3 clignotements
        for (int x=0; x<9; x++) {
            for (int y=0 ; y<9; y++) {
                leds[XY(x,y)] = CRGB(R,G,B);
            }
        }
        FastLED.show();
        delay(500);

        for (int x=0; x<9; x++) {
            for (int y=0 ; y<9; y++) {
                leds[XY(x,y)] = CRGB(0,0,0);
            }
        }
        FastLED.show();
        delay(500);
    }
}


