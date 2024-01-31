#include <stdint.h> // uint8_t sinon il hurle
#include <stdlib.h>

#include "terlib.h"

#define FLAG_INIT   0x01
#define FLAG_POMME  0x02
#define FLAG_OVER   0x04

game_t snake(game_t game_data, uint8_t input)
{
    // DECLARATIONS / INITIALISATIONS --------------------------
    // static ch_lst *corpschaine = NULL;    // init ma liste chainée
    static uint8_t flag = 0x03;             // set les flags utilisés dans le projet
    static uint8_t headpos = 0x55;          // position au milieu
    static uint8_t pmpos;                   // position de la pomme
    static uint8_t dir = 0x03;              // un bit pour VERTICAL/HORIZONTAL un bit pour +/-
    static unsigned long pasttime = 1000.0;
    static uint8_t snakesize = 3;
    static uint8_t bodypos[30];

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
            game_data.printmatrix[0][0] = LED_BLANC;
            flag &= 0xFD; //set bit de flag à 0
            break;

        case FLAG_OVER:  //detecte si partie est finie
            headpos = 0x55;
            pmpos = NULL; 
            dir = 0x03;
            snakesize = 3;
            flag = 0x03;
            break;

        default: break;
    }
    

    //set la matrice en noir -> pas nécessaire si clrscrn() dans le main
    for (uint8_t i=0; i<9; i++) {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[i][j] = LED_NOIR;
        }
    }


    //set dir en fonction des input comme definie dans ma documentation
    switch (input & 0x3C) { //prend uniquement les touches HBGD et pas A ni B
        case 4:  dir = 1; break; // left
        case 8:  dir = 3; break; // right
        case 16: dir = 0; break; // down
        case 32: dir = 2; break; // up
        default: break;
    }

    //éxécute le main code lorsque'un certain temps s'est écoulé => lissibilité
    if ((millis() - (unsigned long)pasttime) >= 10.0) {
        //ch_lst *p=corpschaine; //def la chaine a modifier

        //refresh position de la tete
        headpos = ((headpos&0xF0) + (((1-(dir&0x01))*((1)-2*((dir&0x02)>>1))) <<4)) | ((headpos&0x0F) + ((dir&0x01)*((1)-2*((dir&0x02)>>1)))); //definit la nouvelle position de la tete > cf docs

        if ((headpos & 0x0F) > 9)  headpos = headpos & 0xF0 | 0x01;
        if ((headpos >> 4) > 9)    headpos = headpos & 0x0F | 0x10;
        if ((headpos & 0x0F) == 0) headpos = headpos & 0xF0 | 0x09;
        if ((headpos >> 4) == 0)   headpos = headpos & 0x0F | 0x90; 


        //mange la pomme
        if (headpos == pmpos) {
            snakesize++; // ajoute 1 à la taille
            // Push(&corpschaine, 0x00);//ajoute une case a ma liste chainee
            flag |= 0x02; // libere le flag de pm pour replacer une nouvelle pomme
        }


        // deplace chaques parties du corps(sauf la tete) a la précédente------------
        // p=corpschaine;
        for (int i=0; i<snakesize-1; i++) {
            bodypos[snakesize-1-i] = bodypos[snakesize-2-i];

            //detecte si la tete est dans le corps
            if (bodypos[snakesize-1-i] == headpos) {
                game_data.state = STOP;
                game_data.printmatrix[0][0] = LED_BLANC;

                flag |= 0x04; // set le flag d'arret
            }
        }
        // p->pos = headpos;//attribue la position de la tete a l'elem 0 du corps
        bodypos[0] = headpos;


        // Affiche la pomme
        game_data.printmatrix[(pmpos & 0x0F)-1][(pmpos >> 4)-1] = PLAYER2; 


        // Affiche toutes les part du corps
        //p=corpschaine;
        for (int i=0; i<snakesize; i++) {            
            game_data.printmatrix[(bodypos[i] & 0x0F) - 1][(bodypos[i] >> 4) - 1] = PLAYER1; 
            //p=p->prec;
        }
    }
/*
    switch case droite gauche haut bas
*/

    return game_data;
}