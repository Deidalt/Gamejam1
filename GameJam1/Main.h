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
#define LCASE 50
#define W4K 3840
#define H4K 2160
#define W2K 1920
#define H2K 1080
#define LCASE 158 // @TODO: choisir l'un des 2 LCASE
#define HCASE 79
#define TIMETURN 2000

extern SDL_Window* Screen;
extern int EndMain;
extern SDL_Renderer *Renderer;
extern SDL_Point posMap;
extern SDL_Point LastMove;
extern int Eras;
extern float Zoom;
extern SDL_Point ScreenL;

typedef enum { EMPTY_CASE, MOUNTAIN, RIVER, SEA, FOREST, HUT, HOUSE, APPART, MILL, SHIP } CaseType;

struct Case {
	CaseType Object;//0 = Vide, 1= montagne,2=riviere,3=mer,4=arbre, 5=hut, 6 =house, 7 =appart ,8 = Moulin, 9=Bateau
	int State; //0= empty, 1=const 2=destru, (sert aussi à indiquer le nombre d'arbre détruit dans la case)
};
extern Case Grid[LMAP][HMAP];

struct Ressources {
	int Food; //useless
	int Animals;
	int Trees;
	int Pop;
	int Huts; //1pop
	int Houses; //2pop
	int Apparts; //4pop
	int Hunt;
	int Fish;
	int Harvest;
};
extern Ressources Ress;

#define COL_FOREST 20
#define LINE_FOREST 10
#define FOREST_W 10
#define FOREST_H 10

#endif