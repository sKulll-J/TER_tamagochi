#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "terlib.h"
#include "color.h"

#define OWN 1
#define OPP 2
#define NOT_SELECTED 0
#define SELECTED 1


typedef struct 
{
    uint8_t x; // Coordonnée x du pion
    uint8_t y; // Coordonnée y du pion
} Pion;

typedef struct 
{
    int dx; // Composante x du vecteur
    int dy; // Composante y du vecteur
} Vecteur;

// ---------------------------------------------------------------- 

void plateau_init(uint8_t plateau[5][9]);
bool move_pion(uint8_t plateau[5][9], Pion pion_depart, Pion pion_arrivee, Vecteur deplacement);
uint8_t getPlayerCol(struct game_s game_data);
struct game_s refreshScr(struct game_s game_data,uint8_t plateau[5][9], uint8_t flag_pion, uint8_t x, uint8_t y);
void kill(uint8_t plateau[5][9], uint8_t entourage[5][5], Pion pion_arrivee, Pion pion_depart, struct game_s game_data);

uint8_t check_win(uint8_t plateau[5][9]);
/*
    FONCTIONS NECESSAIRES
        - initialiser le plateau de jeu : ok
        - vérifier victoire : ok
        - vérifier entourage d'un pion : ok

        - selectionner/déselectionner un pion : en cours
            - déplacement : à tester

        - éliminations : à tester
            - calculer vecteur déplacement : ok

    TO DO : 
    - coder la gameloop 
    - tests
*/

struct game_s fanorona(struct game_s game_data, uint8_t input) 
{
    // DECLARATIONS --------------------------------------------
    static uint8_t plateau[5][9] = {0};    // grille de jeu
    static uint8_t entourage[3][3] = {0};
    static uint8_t x = 4;
    static uint8_t y = 2;
    static Pion start = {0,0};
    static Pion end = {0,0};
    static Vecteur vector_d = {0,0};
    static uint8_t flag_pion = NOT_SELECTED;
    static uint8_t flag_init = 1;
    static uint8_t win = 0;

    //init plateau
    if(win==0 && game_data.state==RUN)
    {
    if(flag_init==1)
    {    
        plateau_init(plateau);
        flag_init = 0;
    }

    // Clear
    for (uint8_t i=0; i<9; i++) {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[i][j] = COL_NOIR;
        }
    }
    //affiche plateau
    game_data = refreshScr(game_data,plateau, flag_pion, x,y);


    // GAME LOOP -----------------------------------------------
    if (flag_pion == NOT_SELECTED) { // mode selection du pion à bouger
        switch (input) { //inverse les direction pour le joueur opposant (car a l'envers)
            case INPUT_R:  if (x > 0 && game_data.current_player==PLAYER1) x--; if (x < 8 && game_data.current_player==PLAYER2) x++; break;
            case INPUT_L: if (x < 8&& game_data.current_player==PLAYER1) x++;if (x > 0 && game_data.current_player==PLAYER2) x--; break;
            case INPUT_U:  if (y > 0&& game_data.current_player==PLAYER1) y--;if (y < 4 && game_data.current_player==PLAYER2) y++; break;
            case INPUT_D:    if (y < 4&& game_data.current_player==PLAYER1) y++;if (y > 0 && game_data.current_player==PLAYER2) y--; break;
            case INPUT_A: 
                if ((game_data.current_player==PLAYER1 && COL_OWN== plateau[y][x]) || (game_data.current_player==PLAYER2 && COL_OPP== plateau[y][x])) {   // si le joueur a selectionné un de ses pions
                    
                    start.x = x;
                    start.y=y;
                    if(check_entourage(plateau, entourage, start)) //vérifie qu'une case libre est adjacente
                    {    
                        flag_pion = SELECTED;
                    }
                } 
                else {
                    // joue un petit "bip" (si on ajoute un système audio)
                    
                }
                break;
            case INPUT_B: break; // rien

            
        }
        game_data=refreshScr(game_data,plateau, flag_pion, x,y);
         // affichage du curseur (blanc sur du vide)
            
    }
    if (flag_pion == SELECTED) { // mode déplacement du pion sélectionné
    if(game_data.current_player==PLAYER1)
    {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[j][7] = COL_OWN_CLAIR;
        }
    }
    if(game_data.current_player==PLAYER2)
    {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[j][1] = COL_OPP_CLAIR;
        }
    }

        switch (input) {
            case INPUT_R:  if (x > 0 && game_data.current_player==PLAYER1) x--; if (x < 8 && game_data.current_player==PLAYER2) x++; break;
            case INPUT_L: if (x < 8&& game_data.current_player==PLAYER1) x++;if (x > 0 && game_data.current_player==PLAYER2) x--; break;
            case INPUT_U:  if (y > 0&& game_data.current_player==PLAYER1) y--; if (y < 4 && game_data.current_player==PLAYER2) y++; break;
            case INPUT_D:    if (y < 4&& game_data.current_player==PLAYER1) y++; if (y > 0 && game_data.current_player==PLAYER2) y--; break;
            case INPUT_A:
                if(plateau[y][x]==COL_NOIR)
                {
                    end.x=x;
                    end.y=y;
                    compute_vector(&vector_d, start, end);

                        if (move_pion(plateau, start, end , vector_d))
                        { 
                            compute_vector(&vector_d, start, end);
                            kill(plateau, entourage,start, end, game_data);
                            game_data.current_player = ! game_data.current_player;
                            
                            x=4;
                            y=2;
                            flag_pion = NOT_SELECTED;
                        }
                        else
                        {
                            flag_pion = NOT_SELECTED;
                            
                        }
                        //game_data=refreshScr(game_data,plateau, flag_pion);
                        win = check_win(plateau);

                        
                        
                }
                break; // rien
            case INPUT_B: 
                flag_pion = NOT_SELECTED;
            break;

            
        }
        game_data=refreshScr(game_data,plateau, flag_pion, x,y);
        if(win == 1 || win == 2)
        {   
            for (uint8_t i=0; i<9; i++) {
                for (uint8_t j=0; j<9; j++) {   
                    if(win==1)
                        game_data.printmatrix[i][j] = COL_OWN;
                    if(win==2)
                        game_data.printmatrix[i][j] = COL_OPP;
                }
            }
            game_data.winlose=!(win-1);
        }
        
    }

        

    
    }
    return game_data;

}


// FONCTIONS ---------------------------------------------------
/*  
    Fonction pour initialiser le plateau : pour allumer les leds de chaque joueur.
*/
void plateau_init(uint8_t plateau[5][9])
{
    for(int i = 0 ; i<5 ; i++)
    {
        for(int j = 0 ; j<9 ; j++)
        {
            if(i == 0 | i == 1) plateau[i][j] = COL_OPP ;
            else if(i == 3 | i == 4) plateau[i][j] = COL_OWN ;

            else if(i==2)
            {
                if(j==0|j==2|j==5|j==7) plateau[i][j] = COL_OPP ;
                if(j==1|j==3|j==6|j==8) plateau[i][j] = COL_OWN ;
            }
            else plateau[i][j] = COL_NOIR ;
        }
    }
}

struct game_s refreshScr(struct game_s game_data,uint8_t plateau[5][9], uint8_t flag_pion, uint8_t x,  uint8_t y)
{
    for (uint8_t i=0; i<9; i++) {
        for (uint8_t j=0; j<5; j++) { 
            switch(plateau[j][i]){
                case COL_OWN : game_data.printmatrix[i][j+2] = COL_OWN; break;
                case COL_OPP : game_data.printmatrix[i][j+2] = COL_OPP; break;
                default : game_data.printmatrix[i][j+2] = COL_NOIR; break;
            }
        }
    }

    if(game_data.current_player == PLAYER1) //barre lumineuse d'information en bas de l'écran
    {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[j][8] = COL_OWN;
            game_data.printmatrix[j][0] = COL_NOIR;   
            game_data.printmatrix[j][1] = COL_NOIR;
            game_data.printmatrix[j][7] = COL_NOIR;
            if(flag_pion==SELECTED)
                game_data.printmatrix[j][7] = COL_OWN_CLAIR;
        }
    }
    if(game_data.current_player == PLAYER2)
    {
        for (uint8_t j=0; j<9; j++) {   
            game_data.printmatrix[j][0] = COL_OPP;
            game_data.printmatrix[j][7] = COL_NOIR;   
            game_data.printmatrix[j][8] = COL_NOIR;  
            game_data.printmatrix[j][1] = COL_NOIR;
            if(flag_pion==SELECTED)
                game_data.printmatrix[j][1] = COL_OPP_CLAIR;
        }
    }

    switch (plateau[y][x]) {
                case COL_OWN:   game_data.printmatrix[x][y+2] = COL_OWN_CLAIR;   break;
                case COL_OPP:   game_data.printmatrix[x][y+2] = COL_OPP_CLAIR;   break;
                case COL_NOIR: if(flag_pion==SELECTED) game_data.printmatrix[x][y+2] = getPlayerCol(game_data);else game_data.printmatrix[x][y+2] = COL_BLANC; break; 
                default :       game_data.printmatrix[x][y+2] = COL_NOIR ;       break;
                    //! pas sur du [y][x]
            }

    return game_data;
}

/*Permet de récupérer la couleur du player en cours*/
uint8_t getPlayerCol(struct game_s game_data)
{
    if(game_data.current_player == PLAYER1)
        return COL_OWN;
    if(game_data.current_player == PLAYER2)
        return COL_OPP;
}

/*  
    Cette fonction scrute le plateau de jeu, vérifie les pions restants et détermine le vainqueur.
    Victoire par anihilation. 
    Immobilisation non traitée ...
*/
uint8_t check_win(uint8_t plateau[5][9])
{   
    int pion_J1 = 0; // Variable pour détecter un pion du J1
    int pion_J2 = 0; // Variable pour détecter un pion du J2
    
    for(int i = 0; i<5 ; i++)
    {
        for(int j = 0 ; j<9 ; j++)
        {
            if      (plateau[i][j] == COL_OWN)    pion_J1 = 1 ;
            else if (plateau[i][j] == COL_OPP)    pion_J2 = 1 ;
        }
    }

    // Vérification
    if (pion_J1 && !pion_J2) 
    {
        return 1; // Joueur 1 gagne
    } 
    
    else if (!pion_J1 && pion_J2) 
    {
        return 2; // Joueur 2 gagne
    } 
    
    else
    {
        return 0; // La partie continue
    } 
    
}

/*  
    Cette fonction vérifie l'entourage du pion sélectionné.
*/
int check_entourage(uint8_t plateau[5][9], uint8_t entourage[3][3], Pion pion) 
{
    // Parcours de l'entourage 5x5 autour du pion sélectionné
    for (int i = pion.x - 1, x_entourage = 0; i <= pion.x + 1; i++, x_entourage++) 
    {   
        for (int j = pion.y - 1, y_entourage = 0; j <= pion.y + 1; j++, y_entourage++) 
        {
            // Vérification des limites du plateau
            if (i >= 0 && i < 5 && j >= 0 && j < 9) 
            {
                // Si la case correspond au pion sélectionné, copier sa valeur depuis le plateau
                if (i == pion.y && j == pion.x) 
                {
                    entourage[x_entourage][y_entourage] = plateau[pion.y][pion.x];
                }
                // Sinon, copier la valeur de la case du plateau dans l'entourage
                else 
                {
                    entourage[x_entourage][y_entourage] = plateau[i][j];
                }
            }

            // Si la case est en dehors du plateau, la considérer comme vide (0)
            else 
            {
                entourage[x_entourage][y_entourage] = COL_OWN;
            }
        }
    }

    //Check vide dans entourage
    for(int i =0; i<5; i++)
    {
        for(int  j=0; j<5; j++)
        {
            if(entourage[i][j]==COL_NOIR)
                return 1;
        }
    }
    return 0;
}

/*  
    Cette fonction calcule un vecteur déplacement.
*/
void compute_vector(Vecteur* deplacement, Pion pion_avant, Pion pion_apres)
{
    // Calcul des composantes du vecteur de déplacement
    deplacement->dx = pion_apres.x - pion_avant.x;
    deplacement->dy = pion_apres.y - pion_avant.y;

}

/*  
    Fonction pour valider un déplacement.
*/
bool move_pion(uint8_t plateau[5][9], Pion pion_depart, Pion pion_arrivee, Vecteur deplacement)
{
    // Vérification si les coordonnées de départ et d'arrivée sont valides
    if ( pion_depart.x < 0 || pion_depart.y >= 5  || pion_depart.y < 0  || pion_depart.x >= 9 ||
        pion_arrivee.x < 0 || pion_arrivee.y >= 5 || pion_arrivee.y < 0 || pion_arrivee.x >= 9   ) 
    {
        //plateau[0][1]=COL_OWN;
        return false; // Déplacement invalide
    }

    // Vérification si la case d'arrivée est vide
    if (plateau[pion_arrivee.y][pion_arrivee.x] != COL_NOIR) 
    {
        //plateau[0][2]=COL_OPP;
        return false; // Déplacement invalide - case occupée
    }

    // Calcul du vecteur de déplacement
    compute_vector(&deplacement, pion_depart, pion_arrivee);

    // Vérification du mouvement
    if ((deplacement.dx == 1 || deplacement.dx == -1 || deplacement.dx == 0) &&
        (deplacement.dy == 1 || deplacement.dy == -1 || deplacement.dy == 0)) 
    {
        // Déplacement autorisé, maj du plateau
        plateau[pion_arrivee.y][pion_arrivee.x] = plateau[pion_depart.y][pion_depart.x];
        plateau[pion_depart.y][pion_depart.x] = COL_NOIR; // Vide la case de départ

        return true ;
    }
    else 
    {
        //plateau[0][0]=COL_NOIR;
        return false; // Déplacement invalide
    }
}


/*
    Fonction pour éliminations
*/
void kill(uint8_t plateau[5][9], uint8_t entourage[5][5], Pion pion_arrivee, Pion pion_depart, struct game_s game_data)
{   
    
    Vecteur deplacement = {0,0};
    compute_vector(&deplacement, pion_depart, pion_arrivee);

    uint8_t kil1_x = pion_arrivee.x + deplacement.dx ;
    uint8_t kil1_y = pion_arrivee.y + deplacement.dy ;

    uint8_t kil2_x = pion_depart.x - deplacement.dx ;
    uint8_t kil2_y = pion_depart.y - deplacement.dy ;
    uint8_t aim = 0;

    if(game_data.current_player==PLAYER2)
        aim = COL_OWN;
    else
        aim = COL_OPP;

    while  (((plateau[kil1_y][kil1_x] == aim )) && (kil1_x<=8 && kil1_x>=0) && (kil1_y<=4 && kil1_y>=0) )
    {
        if ((kil1_x <=8 && kil1_x >= 0) && (kil1_y <=4 && kil1_y >=0))
        {
            plateau[kil1_y][kil1_x] = COL_NOIR ; // éliminations par percussion
            kil1_x = kil1_x + deplacement.dx ;
            kil1_y = kil1_y + deplacement.dy ;            
        }
    }

    while ( ((plateau[kil2_y][kil2_x] == aim ))&& (kil2_x<=8 && kil2_x>=0) && (kil2_y<=4 && kil2_y>=0)  )
    {    
        if ((kil2_x <9 && kil2_x >= 0) && (kil2_y <5 && kil2_y >=0))
        {
            plateau[kil2_y][kil2_x] = COL_NOIR ; // éliminations par aspiration
            kil2_x = kil2_x - deplacement.dx ;
            kil2_y = kil2_y - deplacement.dy ;
        }

    } 
    
}