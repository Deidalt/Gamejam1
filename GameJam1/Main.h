#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define LMAP 30
#define HMAP 30
#define MAXTREES 400
#define W4K 3840
#define H4K 2160
#define W2K 1920
#define H2K 1080
#define LCASE 158
#define HCASE 79
#define TIMETURN 1000 //2000

extern SDL_Window* Screen;
extern int EndMain;
extern SDL_Renderer *Renderer;
extern SDL_Point posMap;
extern SDL_Point LastMove;
enum Eras { TRIBAL, MEDIEVAL, CONTEMPORARY };
extern Eras era;
extern float Zoom;
extern SDL_Point ScreenL;
extern int Revenge;

typedef enum { EMPTY_CASE, MOUNTAIN, RIVER, SEA, FOREST, HUT, HOUSE, APPART, MILL, FIELD = MILL, SHIP } CaseType;

struct Case {
	CaseType Object;//0 = empty, 1= mount,2=river,3=sea,4=tree, 5=hut, 6 =house, 7 =appart ,8 = Mill or Field, 9=Ship
	int State; //0= Building, 1=Built, 2=Destroyed ; OR number of tree destroyed
};
extern Case Grid[LMAP][HMAP];

struct Ressources {
	int Food; //useless
	int Treecut; //Faire tableau d'affichage pour connaitre les raisons de chaque arbre coupé
	int Animals;
	int Trees;
	int Pop;
	int Huts; //1pop
	int Houses; //2pop
	int Apparts; //4pop
	int Hunt;
	int Fish;
	int Harvest;
	int River;
};
extern Ressources Ress;

#define COL_FOREST 20
#define LINE_FOREST 10
#define FOREST_W 10
#define FOREST_H 10

enum Actions { PLANT, RAIN, COLD, METEOR, DEVOUR, DROWN, NB_ACTIONS };
extern Actions lastAction;

void SetAsAction(Actions action);
Actions getCurrentAction();

#endif