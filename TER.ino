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
#define BRIGHTNESS      10  // luminosité réglable
#define NUM_LEDS (MAT_WIDTH * MAT_HEIGHT)
CRGB leds_plus_safety_pixel[NUM_LEDS + 1];
CRGB* const leds(leds_plus_safety_pixel + 1);


// DECLARATION DE FONCTIONS ------------------------------------
static uint8_t XY(uint8_t x, uint8_t y);    // static car la fonction est passée de uint16 à uint8. 9x9 = 81 on a pas besoin de 16 bits pour aller jusque là ; sauf que la fonction est déjà déclarée dans fastLED, donc la foutre en static la rend accessible seulement ici et pas de warning ^^
uint8_t calcul_coordonnee(uint8_t x, uint8_t y);
uint8_t readCartouche(void);    // fonction qui lit les 3 bits de cartouche
void update(struct game_s *game, uint8_t input);
void render(struct game_s game);
void tr_clignote(CRGB color);


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
    FastLED.setBrightness(BRIGHTNESS);
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
        if (tergame.winlose == WIN) {
            tr_clignote(OWN_COLOR);
        } if (tergame.winlose==LOSE) {
            tr_clignote(OPP_COLOR);
        }
        //FastLED.show();
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
        for (i=0; i<9; i++) {
            for (j=0; j<9; j++) {
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
void tr_clignote(CRGB color)
{
    for (int i=0; i<3; i++) {   // 3 clignotements
        for (int x=0; x<9; x++) {
            for (int y=0 ; y<9; y++) {
                leds[XY(x,y)] = color;
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

/////////////////////////////////
static void color_matching(struct game_s game_data, uint8_t *col_current_own, uint8_t *col_current_clair_own, uint8_t *col_current_opp, uint8_t *col_current_clair_opp);
static void calcul_coord(uint8_t *x, uint8_t *y, uint8_t megax, uint8_t megay, uint8_t minix, uint8_t miniy);  // transforme les megax/miniy... en x,y de 9x9
static void croix_directionnelle(uint8_t input, uint8_t *x, uint8_t *y);
static uint8_t check_win(float (*mat)[3]); // magie noire
static uint8_t checksub(float val);
static void joker_mode(uint8_t input, float (*xomatrix)[3][3][3], uint8_t megax, uint8_t megay);   // gros selector

struct game_s megamorpion(struct game_s game_data, uint8_t input)
{
    // DECLARATIONS --------------------------------------------
    
    /* Il faut tout mettre en static parce que la fonction est appelée genre 60 fois par seconde donc il faut pas réinitialiser des valeurs
     *      problème : on peut pas mettre des valeurs random dans un static (on veut un random pour commencer au pif qqpart dans la grille)
     *      solution : on appelle la fonction de jeu avec comme input le nombre généré aléatoirement pour connaitre qui commence en premier et on le %3 pour avoir une valeur de départ
     */
    static uint8_t x;                   // position x absolue 9x9
    static uint8_t y;                   // position y absolue 9x9
    static uint8_t minix = 0;           // position x sur une minigrille
    static uint8_t miniy = 0;           // position y sur une minigrille
    static uint8_t megax = 0;           // position x sur la megagrille
    static uint8_t megay = 0;           // position y sur la megagrille
    static float xomatrix[3][3][3][3];  // matrice qui contient les valeurs 0=own, 1=opp, NaN=personne - matrice LOGIQUE du jeu sur lequel on fait les calculs de victoire
    static float megawin[3][3];         // matrice 3x3 mega contenant les valeurs 0, 1, NaN, inf (inf = égalité sur la minigrille, pas de vainqueur possible)
    static uint8_t miniwin[3][3] = {0}; // couleur de chaque minigrille[x][y]
    static uint8_t col_current_own;
    static uint8_t col_current_own_clair;
    static uint8_t col_current_opp;
    static uint8_t col_current_opp_clair;
    static uint8_t flag_input = 1;

    // INITIALISATIONS -----------------------------------------
    if (flag_input == 1) {
        // on a passé initalement en input tergame.current_player après avoir determiné qui commence. On l'utilise pour avoir du random dans la position, car si on faisait x=rand(3) par exemple on aurait des positions differentes sur chaque console
        input %= 9;
        megax = input % 3;
        megay = input % 3;
        minix = (input - megax + 1) % 3; // transforme [9][9] en [3][3][3][3] je crois (au pire osef cest juste un peu de random)
        miniy = (input - megay - 1) % 3; 

        #if DEBUG
            Serial.println("[?] Init");
            Serial.println();
            Serial.print(input); Serial.println(" % 3");
            Serial.print("pos: "); 
            Serial.print("[");Serial.print(megax);Serial.print("]");
            Serial.print("[");Serial.print(megay);Serial.print("]");
            Serial.print("[");Serial.print(minix);Serial.print("]");
            Serial.print("[");Serial.print(miniy);Serial.println("]");
        #endif

        // Remplissage des matrices en "vide"
        for (uint8_t i=0; i<3; i++) {
            for (uint8_t j=0; j<3; j++) {
                for (uint8_t k=0; k<3; k++) {
                    for (uint8_t l=0; l<3; l++) {
                        xomatrix[i][j][k][l] = NAN; // matrice de jeu vide
                    }
                }
                megawin[i][j] = NAN; // matrice de jeu vide
            }
        }

        // Quelle couleur commence
        color_matching(game_data, &col_current_own, &col_current_own_clair, &col_current_opp, &col_current_opp_clair);

        // Affiche le selecteur avant un input
        calcul_coord(&x, &y, megax, megay, minix, miniy);
        game_data.printmatrix[x][y] = col_current_own;

        flag_input = 0;
    }
    
    
    // GAME LOOP -----------------------------------------------
    color_matching(game_data, &col_current_own, &col_current_own_clair, &col_current_opp, &col_current_opp_clair);

    if (input != INPUT_A) {  
        game_data.printmatrix[x][y] = game_data.previous_printmatrix[x][y];    // sert à effacer l'ancienne position du selecteur
        croix_directionnelle(input, &minix, &miniy);
        calcul_coord(&x, &y, megax, megay, minix, miniy);

        // déplacement du curseur de sélection - clair si ya deja un pion de placé
        if (xomatrix[megax][megay][minix][miniy] == 0) { 
            game_data.printmatrix[x][y] = col_current_own_clair; 
        } else if (xomatrix[megax][megay][minix][miniy] == 1) { 
            game_data.printmatrix[x][y] = col_current_opp_clair;
        } else { 
            game_data.printmatrix[x][y] = col_current_own;
        }
    } else if (input == INPUT_A) {       
        if (isnan(xomatrix[megax][megay][minix][miniy])) {                      // si la case est libre
            xomatrix[megax][megay][minix][miniy] = game_data.current_player;    // on met la valeur du joueur
            game_data.printmatrix[x][y] = col_current_own;                      // et sa couleur associée

            game_data.current_player = !game_data.current_player;               // inversion des joueurs
            color_matching(game_data, &col_current_own, &col_current_own_clair, &col_current_opp, &col_current_opp_clair);

            /*  Tout ce bout de code vérifie si chaque minigrid a une victoire
             *  Puis "remonte" d'un cran en stockant la valeur de la victoire dans une grille 3x3 (megawin)
             *  Afin de faire une check_win sur cette matrice la pour déterminer s'il y a un gagnant à la partie
             */
            for (uint8_t i=0; i<3; i++) {
                for (uint8_t j=0; j<3; j++) {
                    if (miniwin[j][i] == COL_NOIR) {                // si ya deja une win il faut pas revérifier
                        miniwin[j][i] = check_win(xomatrix[i][j]);  // ? pourquoi miniwin[j][i] et pas [i][j]?? je sais pas mais ça marche alors nsm
                        
                        if (miniwin[j][i] == PLAYER1) megawin[j][i] = 0;
                        if (miniwin[j][i] == PLAYER2) megawin[j][i] = 1;
                    }

                    switch (check_win(megawin)) {
                        case PLAYER1: game_data.winlose = WIN;  break;
                        case PLAYER2: game_data.winlose = LOSE; break;
                        default: break;
                    }
                }
            }

            // Teleportation fin de tour
            megax = minix;  // prochain coup dans la même mégacase que le coup joué dans la minicase
            megay = miniy;  // ---
            for (int i=0, j=0; i<3, j<3; i++, j++) {
                if (isnan(xomatrix[megax][megay][i][j])) { // for i,j, if xomatrix[prochain x][prochain y][i][j] == NaN (=vide)
                    minix = i;
                    miniy = j;
                    break;
                }
                
                if (!isnan(xomatrix[megax][megay][minix][miniy])) break;
                else joker_mode(input, xomatrix, megax, megay);  // si aucune case libre : mode joker où on peut se relocaliser nimporte où sur la map
            }
            calcul_coord(&x, &y, megax, megay, minix, miniy);
            game_data.printmatrix[x][y] = col_current_own;  // affiche le pion téléporté
        }
        
            
    }
    //else return game_data; // pas d'input = quitte direct le bail

    #if DEBUG
        Serial.println("\n[?] XOmatrix");
        Serial.print("C'est au tour de ");
        if (game_data.current_player == PLAYER1) Serial.println("MOI");
        if (game_data.current_player == PLAYER2) Serial.println("LUI");
        for (int i=0; i<9; i++) {
            for (int j=0; j<9; j++) {
                if (game_data.printmatrix[i][j] == COL_NOIR) Serial.print(".");
                else Serial.print("O");

                if (j==2 || j==5) Serial.print(" ");
            }
            if (i==2 || i==5) Serial.println();
            Serial.println();
        }
        Serial.println("[?] Fin update");
    #endif
    
    return game_data;
}


// FONCTIONS ---------------------------------------------------
/**
 *  @brief Cette fonction attribue la paire de couleur foncée/claire au tour actuel (ça sert à inverser les palettes quoi)
 */
void color_matching(struct game_s game_data, uint8_t *col_current_own, uint8_t *col_current_own_clair, uint8_t *col_current_opp, uint8_t *col_current_opp_clair)
{
    if (game_data.current_player == PLAYER1) {
        *col_current_own = COL_OWN;
        *col_current_own_clair = COL_OWN_CLAIR;
        *col_current_opp = COL_OPP;
        *col_current_opp_clair = COL_OPP_CLAIR;
    } else if (game_data.current_player == PLAYER2){
        *col_current_own = COL_OPP;
        *col_current_own_clair = COL_OPP_CLAIR;
        *col_current_opp = COL_OWN;
        *col_current_opp_clair = COL_OWN_CLAIR;
    }
}

/** 
 *  @brief Cette fonction transforme les coordonnées [3][3][3][3] en [9][9] 
 *  @param x coordonnée x de la matrice 9x9
 *  @param y coordonnée y de la matrice 9x9
 *  @param megax coordonnée x dans la grande matrice 3x3
 *  @param megay coordonnée y dans la grande matrice 3x3
 *  @param minix coordonnée x dans la petite matrice 3x3
 *  @param miniy coordonnée y dans la petite matrice 3x3
 */
void calcul_coord(uint8_t *x, uint8_t *y, uint8_t megax, uint8_t megay, uint8_t minix, uint8_t miniy)
{
    *x = megax * 3 + minix;
    *y = megay * 3 + miniy;
}

void croix_directionnelle(uint8_t input, uint8_t *x, uint8_t *y) // 17719
{
    if (input == INPUT_L) { if (*x > 0) (*x)--; } 
    else if (input == INPUT_R) { if (*x < 2) (*x)++; }
    else if (input == INPUT_D) { if (*y > 0) (*y)--; }
    else if (input == INPUT_U) { if (*y < 2) (*y)++; }
}

void joker_mode(uint8_t input, float (*xomatrix)[3][3][3], uint8_t megax, uint8_t megay)
{
    croix_directionnelle(input, &megax, &megay);
    xomatrix[1][1][1][1] = 3;
}


/** 
 *  @brief Cette fonction prend une matrice 3x3 et vérifie si l'un des deux joueurs a gagné.  
 *         La vérification se fait en calculant la somme de chaque ligne, colonne et diagonale suivant le principe que:
 *              NaN + x = NaN => la lin/col/diag n'est pas complète
 *              0 + 0 + 0 = 0 => la lin/col/diag est gagnée par le J1
 *              1 + 1 + 1 = 3 => la lin/col/diag est gagnée par le J2
 *              1 + 0 + 1 = 2 => la lin/col/diag n'est gagnée par personne
 *  @param mat matrice 3x3
 *
 *  @todo rajouter les test isinf() pour detecter une megacase où ya une égalité
 */
uint8_t check_win(float (*mat)[3]) 
{
    uint8_t CR;
    float lin = 0;
    float col = 0;
    float diag1 = 0;
    float diag2 = 0;
    
    // Calcul des valeurs
    for (int i=0; i<3; i++) {
        lin = 0;
        col = 0;
        for (int j=0; j<3; j++) {
            lin += mat[i][j];
            col += mat[j][i];
        }
        diag1 += mat[i][i];
        diag2 += mat[i][2-i];

        if ((CR = checksub(lin)) != 0) return CR;
        if ((CR = checksub(col)) != 0) return CR;
    }
    if ((CR = checksub(diag1)) != 0) return CR;
    if ((CR = checksub(diag2)) != 0) return CR;

    return 0;   // revient à renvoyer "la somme de la ligne/colonne/diagonales est un NaN"
}

/** 
 *  @brief Cette fonction vérifie quelle est la valeur qu'elle reçoit
 *          NaN => return 0
 *          0 => return J1
 *          3 => return J2
 *  @param val 
 */
uint8_t checksub(float val)
{
    if (isnan(val) == false) {
        if (val == 0) return PLAYER1;
        if (val == 3) return PLAYER2;
    }
    return 0;
}

