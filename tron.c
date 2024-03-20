#include <stdint.h> // uint8_t sinon il hurle
#include <stddef.h>

#include "terlib.h"

#define UP 4
#define DOWN  16 
#define RIGHT 8
#define LEFT 2

#define flagP1 0x01
#define flagP2 0x02
#define flagTime 0x04
#define FLAGp_START 3 // position du flag de start

#define CHANGE_DEL_MAX 20000 //temps max avant que la vitesse augmente : 1000= 1min
#define SPD_MIN 50 //vitesse minimum
#define SPD_STRT 170 //vitesse de départ

void newPos(uint8_t * pos, uint8_t * dir, uint8_t input);
void changeBodyPos(uint8_t *_bodyPos, uint8_t _size);
uint8_t getBitVal(uint8_t mot , uint8_t bit_position); //récupere la valeur d'un bit dans un mot
uint8_t checkEndGame(uint8_t *_bodyPos1,uint8_t sizeP1, uint8_t *_bodyPos2,uint8_t sizeP2); //vérifie si un joueur a perdu (ou les deux)
game_t endGame(game_t game_data, uint8_t res); //set l'etat de game_data lors de la fin du jeu

/*
typedef struct Node {
    uint8_t pos;           // Donnée du nœud
    struct Node* next;  // Pointeur vers le prochain nœud
    struct Node* prev;  // Pointeur vers le prochain nœud
} Node;


Node* newNode(uint8_t pos) {
    Node* newNode = (Node*)malloc(sizeof(Node)); // Allocation de mémoire pour le nouveau nœud
    
    newNode->pos = pos; // Initialisation de la donnée du nœud
    newNode->next = NULL; // Initialisation du pointeur vers le prochain nœud à NULL
    newNode->prev = NULL; // Initialisation du pointeur vers le précédent nœud à NULL
    return newNode;
}*/

game_t tron(game_t game_data, uint8_t input) 
{
    static uint8_t pos_1 = 0x34;
    static uint8_t pos_2 = 0x54; 
    static uint8_t prepos_1 = 0x34;
    static uint8_t prepos_2 = 0x54; 
    static uint8_t flags =0x08;
    static unsigned long pastTime = 0;
    static uint16_t loopDelay = SPD_STRT;
    static uint8_t iteration_del = 0; //compte ne nombre de deplacement avant d'augmenter la vitesse

    static uint8_t dir_P1 = 0 ;
    static uint8_t dir_P2 = 0 ;
    static uint8_t size_P1 = 8;
    static uint8_t size_P2 = 8;
    static uint8_t bodypos1[8];
    static uint8_t bodypos2[8];

    static uint8_t count_01 = 0;
    static uint8_t count_input1 = 0;
    static uint8_t saved_input1 = 0;
    static uint8_t count_02 = 0;
    static uint8_t count_input2 = 0;
    static uint8_t saved_input2 = 0;

    static int idx=0;


    if(flags==0x08)
    {
        for(int i=0;i<size_P1; i++)
        {
            bodypos1[i] = pos_1;
        }
        for(int i=0;i<size_P2; i++)
        {
            bodypos2[i] = pos_2;
        }
        setBitVal(&flags, FLAGp_START, 0); 
    }

    // Clear screen
    for (uint8_t i=0; i<9; i++) {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[i][j] = COL_NOIR;
        }
    }

    if(game_data.current_player==PLAYER1) //patch les mauvaises récéptions
    {
        if(input != 0)
        {
            saved_input1 = input;
            count_input1 ++;
        }
        else
        {
            count_01 ++;
        }
    }
    else
    {
       if(input != 0)
        {
            saved_input2 = input;
            count_input2 ++;
        }
        else
        {
            count_02 ++;
        } 
    }
    

    if((game_data.game_time-pastTime>loopDelay) && (getBitVal(flags, 2)==0))
    {
        setBitVal(&flags, 2, 1);
    }

    if(game_data.current_player==PLAYER1 && (getBitVal(flags, 2)==1) && (getBitVal(flags, 0)==0)) //déplacement du player1
    {        
        newPos(&prepos_1,&dir_P1, saved_input1);

        setBitVal(&flags, 0,1);
    }
    else if(game_data.current_player==PLAYER2 && (getBitVal(flags, 2)==1)  && (getBitVal(flags, 1)==0)) //deplacement du player2
    {
        newPos(&prepos_2,&dir_P2, saved_input2);

        setBitVal(&flags, 1,1); 
    }

    if((getBitVal(flags, 0)==1) && (getBitVal(flags, 1)==1)) //si les deux joueurs on bougés
    {
        // Remove bits
        setBitVal(&flags, 0,0); 
        setBitVal(&flags, 1,0);
        setBitVal(&flags, 2,0);
       
        pastTime = game_data.game_time; //met le temps au temps actuel

        //Set la position a la position temporaire
        //prepos sont set le temps que les deux joueurs aient bougées => synchro
        pos_1 = prepos_1;
        pos_2 = prepos_2;

        //
        count_01 = 0;
        count_02 = 0;
        count_input1 = 0;
        count_input2 = 0;

        // Change body position
        changeBodyPos(&bodypos1, size_P1);
        changeBodyPos(&bodypos2, size_P2);
        bodypos1[0]=pos_1;
        bodypos2[0]=pos_2;

        //Change speed
        iteration_del++; //augmente la vitesse toutes les minutes
        if(iteration_del*(uint16_t)loopDelay>CHANGE_DEL_MAX)
        {
            iteration_del=0;
            if(loopDelay>SPD_MIN) //vitesse minimum
                loopDelay-=2;
        }

        //Detect end of game
        if(checkEndGame(bodypos1,size_P1, bodypos2,size_P2)>0)
        {
           game_data.state=STOP; 
        }
        
        if(game_data.state==STOP)
        {
            for (uint8_t i=0; i<9; i++) {
                for (uint8_t j=0; j<9; j++) {   
                    game_data.printmatrix[i][j] = COL_OPPS;
                }
            }
        }
        
    }

    //--Affichage
    for(idx=1;idx<size_P1; idx++)
    {
        game_data.printmatrix[(bodypos1[idx]&0xF0)>>4][bodypos1[idx]&0x0F] = COL_OWN_CLAIR;
    }
    game_data.printmatrix[(bodypos1[0]&0xF0)>>4][bodypos1[0]&0x0F] = COL_OWN;

    for(idx=1;idx<size_P2; idx++)
    {
        game_data.printmatrix[(bodypos2[idx]&0xF0)>>4][bodypos2[idx]&0x0F] = COL_OPPS_CLAIR;
    }
    game_data.printmatrix[(bodypos2[0]&0xF0)>>4][bodypos2[0]&0x0F] = COL_OPPS;
    


    return game_data;
}


void newPos(uint8_t * pos, uint8_t* dir, uint8_t input)
{
    int x,y;
    x= ((*pos)>>4);
    y= ((*pos)&0x0F);

    switch(input)
    {
        case UP : (*dir) = 0;break;
        case DOWN : (*dir) = 1;break;
        case LEFT : (*dir) = 2;break;
        case RIGHT : (*dir) = 3;break;
        default : break;
    }

    switch(*dir)
    {
        case 0 : (y)+=1;break;
        case 1 :(y)-=1; break;
        case 2 :(x)+=1; break;
        case 3 : (x)-=1;break;
        default : break;
    }
    
    if(x>=9)
        x=0;
    if(x<0)
        x=8;
    if(y>=9)
        y=0;
    if(y<0)
        y=8;

    (*pos) = y+(x<<4); 
}

void changeBodyPos(uint8_t *_bodyPos, uint8_t _size)
{
    for (int i=0; i<_size-1; i++) {
            _bodyPos[_size-1-i] = _bodyPos[_size-2-i];
    }
}

uint8_t checkEndGame(uint8_t *_bodyPos1,uint8_t sizeP1, uint8_t *_bodyPos2,uint8_t sizeP2)
{
    uint8_t res = 0; //res=0 : nothing / res=1 : P1 win / res=2 : P2 win / res=3 : No winner
    for(int i=0; i<sizeP1; i++)
    {
        if(_bodyPos2[0]==_bodyPos1[i])
            res+=1;
    } 

    for(int i=0; i<sizeP2; i++)
    {
        if(_bodyPos1[0]==_bodyPos2[i])
            res+=2;
    } 

    return res;
}

game_t endGame(game_t game_data, uint8_t res)
{/*
    if(res==1)
    {
        game_data.state = RUN;
        game_data.winlose = LOSE;

    }
    if(res==2)
    {
        game_data.state = STOP;
        game_data.winlose = WIN;

    }
    if(res==3)
    {
        game_data.state = STOP;
    }*/
    return game_data;
}

uint8_t getBitVal(uint8_t mot , uint8_t bit_position)
{
    return (mot>>bit_position)&0x01;
}

void setBitVal(uint8_t* mot , uint8_t bit_position, uint8_t bit_value)
{
    *mot =(*mot) & (0xFF-(1<<bit_position)); //pour mettre a 0 
    *mot |= (bit_value<<bit_position); //pour mettre a 1   
}