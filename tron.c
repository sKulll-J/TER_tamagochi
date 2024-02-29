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

void newPos(uint8_t * pos, uint8_t * dir, uint8_t input);
void changeBodyPos(uint8_t bodyPos[40], uint8_t size);
uint8_t getBitVal(uint8_t mot , uint8_t bit_position); //récupere la valeur d'un bit dans un mot

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
}

game_t tron(game_t game_data, uint8_t input) 
{
    static uint8_t pos_1 = 0x34;
    static uint8_t pos_2 = 0x54; 
    static uint8_t prepos_1 = 0x34;
    static uint8_t prepos_2 = 0x54; 
    static uint8_t flags =0x08;
    static unsigned long pastTime = 0;
    static uint16_t loopDelay = 250;

    static uint8_t dir_P1 = 0 ;
    static uint8_t dir_P2 = 0 ;
    static uint8_t size_P1 = 1;
    static uint8_t size_P2 = 1;

    static uint8_t count_01 = 0;
    static uint8_t count_input1 = 0;
    static uint8_t saved_input1 = 0;
    static uint8_t count_02 = 0;
    static uint8_t count_input2 = 0;
    static uint8_t saved_input2 = 0;

    static Node* head1;
    static Node* head2;

    for (uint8_t i=0; i<9; i++) {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[i][j] = COL_NOIR;
        }
    }

    /*if(getBitVal(flags, 3)) //flag d'init
    {
        head1 = newNode(pos_1);
        head2 = newNode(pos_2);
        Node* temp1 = head1;
        Node* temp2 = head2;
        for(int i=0; i<size_P1; i++)
        {
            Node* tempTemp1 = newNode(pos_1);
            temp1->next = tempTemp1;
            tempTemp1->prev = temp1;
            temp1 = tempTemp1;

            Node* tempTemp2 = newNode(pos_1);
            temp2->next = tempTemp2;
            tempTemp2->prev = temp2;
            temp2 = tempTemp2;

            game_data.printmatrix[0][0] = COL_OPPS;
        }

        setBitVal(&flags, 3, 0);
    }*/

    if(game_data.current_player==PLAYER1) //patch les mauvaises récéptions
    {
        if(input != 0)
        {
            saved_input1 = input;
            count_input1 ++;
            
            game_data.printmatrix[0][9] = COL_OWN_CLAIR;
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
        game_data.printmatrix[0][0] = COL_BLANC;


        
        //setBitVal(&flags, 0, 0);
        //setBitVal(&flags, 1, 1);
        //pastTime=game_data.game_time;
    }

    if(game_data.current_player==PLAYER1 && (getBitVal(flags, 2)==1) && (getBitVal(flags, 0)==0))
    {
        if(count_input1>1)
        {
            
            game_data.printmatrix[0][1] = COL_BLANC; 
        }
        
        newPos(&prepos_1,&dir_P1, saved_input1);

        setBitVal(&flags, 0,1);
    }
    else if(game_data.current_player==PLAYER2 && (getBitVal(flags, 2)==1)  && (getBitVal(flags, 1)==0))
    {
        if(count_input2>1)
        {
            game_data.printmatrix[0][2] = COL_BLANC;
        }
        
        newPos(&prepos_2,&dir_P2, saved_input2);

        setBitVal(&flags, 1,1); 
    }

    if((getBitVal(flags, 0)==1) && (getBitVal(flags, 1)==1)) //si les deux joueurs on bougés
    {
        setBitVal(&flags, 0,0); 
        setBitVal(&flags, 1,0);
        setBitVal(&flags, 2,0);
       
        pastTime = game_data.game_time;

        pos_1 = prepos_1;
        pos_2 = prepos_2;

        //head1->pos = pos_1;
        //head2->pos = pos_2;

        count_01 = 0;
        count_02 = 0;
        count_input1 = 0;
        count_input2 = 0;

        game_data.printmatrix[1][0] = COL_OPPS;
    }

    /*for(int i =0; i<size_P1; i++)
    {
        game_data.printmatrix[body_P1[i]>>4][body_P1[i]&0x0F] = COL_OWN;
    }
    for(int i =0; i<size_P2; i++)
    {
        game_data.printmatrix[body_P2[i]>>4][body_P2[i]&0x0F] = COL_OPPS;
    }*/

    game_data.printmatrix[pos_1>>4][pos_1&0x0F] = COL_OWN;
    game_data.printmatrix[pos_2>>4][pos_2&0x0F] = COL_OPPS;

    // DEBUG avec matrice 
    /*if(getBitVal(flags, 0)==1) //si les deux joueurs on bougés
    {
        game_data.printmatrix[1][1] = COL_OPPS;
    }
    else{
        game_data.printmatrix[2][1] = COL_OPPS;
        
    }
    if(getBitVal(flags, 1)==1) //si les deux joueurs on bougés
    {
        game_data.printmatrix[1][2] = COL_OWN;
    }
    else{
        game_data.printmatrix[2][2] = COL_OWN;
        
    }
    if(getBitVal(flags, 2)==1) //si les deux joueurs on bougés
    {
        game_data.printmatrix[1][0] = COL_BLANC;
    }
    else{
        game_data.printmatrix[2][0] = COL_BLANC;
        
    }*/

    


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

void changeBodyPos(uint8_t bodyPos[40], uint8_t size)
{
    for(int i=size; i>1; i--)
    {
        bodyPos[i]=bodyPos[i-1];
    }
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