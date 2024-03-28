#include <stdint.h> // uint8_t sinon il hurle
#include <stdlib.h>
#include <Arduino.h> // millis

#include "terlib.h"
#include "color.h"

#define FLAG_INIT   0x01
#define FLAG_POMME  0x02
#define FLAG_OVER   0x04

#define FILTRE_X 0xF0 // 4 bits pour pos X
#define FILTRE_Y 0x0F // 4 bits pour pos Y

#define DIR_HV 0x01 // bits selection horizontal, vertical
#define DIR_PM 0x02 // bit selection plus, moins


struct game_s snake(struct game_s game_data, uint8_t input)
{
    // DECLARATIONS / INITIALISATIONS --------------------------
    // static ch_lst *corpschaine = NULL;    // init ma liste chainée
    static uint8_t flag = 0x03;             // set les flags utilisés dans le projet
    static uint8_t headpos = 0x55;          // position au milieu
    static uint8_t pmpos;                   // position de la pomme
    static uint8_t dir = 0x03;              // un bit pour VERTICAL/HORIZONTAL un bit pour +/-
    static unsigned long pasttime = 1000.0;
    static uint8_t snakesize = 3;
    static uint8_t bodypos[18];

    switch (flag) {
        case FLAG_INIT:      //initialise le corps du snake
            //Push(&corpschaine, 0x55);//position de la tete
            //Push(&corpschaine, 0x56);
            //Push(&corpschaine, 0x57);

            bodypos[0] = 0x55;
            bodypos[1] = 0x56;
            bodypos[2] = 0x57;
            flag &= 0xFE; // set le bit de flag d'init a 0;
            break;

        case FLAG_POMME:     //placement d'une nouvelle pomme
            pmpos = ((random() % 9 + 1) << 4) | (random() % 9 + 1);
            game_data.printmatrix[(pmpos & 0x0F) - 1][(pmpos >> 4) - 1] = COL_OPP; 
            flag &= 0xFD; //set bit de flag à 0
            break;

        case FLAG_OVER:  //detecte si partie est finie
            headpos = 0x55;
            pmpos = 99; // bien en dehors de la grille de jeu 
            dir = 0x03;
            snakesize = 3;
            flag = 0x03;
            break;

        default: break;
    }
    

    switch (input) { 
        case INPUT_L:  dir = 1; break;
        case INPUT_R: dir = 3; break;
        case INPUT_D:  dir = 0; break;
        case INPUT_U:    dir = 2; break;
        default: break; // pas besoin de A ni de B
    }

    // exécute le main code lorsque'un certain temps s'est écoulé => lisibilité
    if (true) {
        //ch_lst *p=corpschaine; //def la chaine a modifier

        //refresh position de la tete
        headpos = ((headpos & FILTRE_X)                // explication
                + (((1 - (dir & DIR_HV))               // ça sert à ...
                * ((1) - 2 * ((dir & DIR_PM) >> 1)))   // là on fait 1 - la valeur du trou noir supermassif
                << 4))                                 // on bitshift ta mere
                | ((headpos & FILTRE_Y)                // PITIE ERWANN DONNE DES EXPLICATIONS
                + ((dir & DIR_HV)                      // blabla bla
                * ((1) - 2 * ((dir & DIR_PM) >> 1)))); // et voilà comment construire une bombe nucélaire

        if ((headpos & 0x0F) > 9)  headpos = (headpos & FILTRE_X) | 0x01;
        if ((headpos >> 4) > 9)    headpos = (headpos & FILTRE_Y) | 0x10;
        if ((headpos & 0x0F) == 0) headpos = (headpos & FILTRE_X) | 0x09;
        if ((headpos >> 4) == 0)   headpos = (headpos & FILTRE_Y) | 0x90; 


        //mange la pomme
        if (headpos == pmpos) {
            snakesize++; // ajoute 1 à la taille
            // Push(&corpschaine, 0x00);//ajoute une case a ma liste chainee
            flag |= FLAG_POMME; // libere le flag de pm pour replacer une nouvelle pomme
        }


        // deplace chaques parties du corps(sauf la tete) a la précédente------------
        // p=corpschaine;
        for (int i=0; i<snakesize-1; i++) {
            bodypos[snakesize-1-i] = bodypos[snakesize-2-i];

            //detecte si la tete est dans le corps
            if (bodypos[snakesize-1-i] == headpos) {
                game_data.state = ter_STOP;
                game_data.printmatrix[0][0] = COL_BLANC;

                flag |= FLAG_OVER; // set le flag d'arret
            }
        }
        // p->pos = headpos;//attribue la position de la tete a l'elem 0 du corps
        bodypos[0] = headpos;


        // Affiche la pomme
        game_data.printmatrix[(pmpos & 0x0F) - 1][(pmpos >> 4) - 1] = COL_OPP; 

        // Affiche toutes les part du corps
        //p=corpschaine;
        for (int i=0; i<snakesize; i++) {            
            game_data.printmatrix[(bodypos[i] & FILTRE_Y) - 1][(bodypos[i] >> 4) - 1] = COL_OPP; 
            //p=p->prec;
        }
        
    }
/*
    switch case droite gauche haut bas
*/

    return game_data;
}