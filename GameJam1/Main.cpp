


#include <signal.h>

#include "Main.h"
#include "Affichage.h"
#include "Evenements.h"

//Globals
SDL_Window* Screen = NULL;
SDL_Renderer *Renderer;
int EndMain = 1;
Case Grid[LMAP][HMAP];
Ressources Ress = { 0,10,MAXTREES,0,0,0,0 };
int Year = -1;
int timegame = 0;
SDL_Point posMap = { 0,0 };
SDL_Point LastMove = { 0,0 }; //1=Droite , 2 =Gauche, 3=Bas, 4=Haut
int Eras = 1; //1=tribal, 2=medieval, 3=contemporain
float Zoom = 1; //same camera for all resolutions
SDL_Point ScreenL = { 0,0 }; //my Reso

int main(int argc, char* argv[])
{
	srand(time(NULL));
	int i = 0, j = 0;
	int PastYear = 0; //Check when year changes
	
	for (i = 0;i < LMAP;i++) { //Init Grille //init map
		for (j = 0;j < HMAP;j++) {
			if (j < 2)
				Grid[i][j].Object = 1;
			else if (j>27)
				Grid[i][j].Object = 3;
			else if (i > 14 && i < 17)
				Grid[i][j].Object = 2;
			else if (i >= 20 && i<30 && j >= 10 && j<20)
				Grid[i][j].Object = 4;
		}
		Grid[i][j].State = 0;
	}
	Grid[10][10].Object = 5;
	
	Year = 0; //Game Starts
	timegame = SDL_GetTicks();
	while (EndMain) {
		if (Year >= 0) {
			Year = (SDL_GetTicks() - timegame) / 2000; //+1 Year every 2 sec
			if (PastYear != Year) {
				//Year changed, Turn calcul
				printf("Tree=%d Pop=%d Food=%d\n", Ress.Trees,Ress.Pop,Ress.Food);
				Ress.Animals++;
				Ress.Hunt = 0;
				Ress.Fish = 0;
				Ress.Harvest = 0;
				//Human turn
				if (Eras == 1) {
					//tribal
					int food = Ress.Pop; //total food to search per turn
					Ress.Pop++;
					Ress.Trees-=2; //trees consumed per turn
					food -= 5;
					int found = 0;
					int Treecut = 2; //Total trees to cut
					while (food > 0 && Ress.Animals > 0) {
						//Hunt
						food -= 5;
						Ress.Hunt ++;
						Ress.Animals--;
					}
					while (Treecut > 0 && Ress.Trees>0) {
						int Rtree = rand() % 101; //destroy random tree
						int TreeI = Rtree % 10; //10col for 10line
						int TreeJ = Rtree / 10;
						if (Grid[20 + TreeI][10 + TreeJ].State < 4) {
							while (Treecut > 0 && Grid[20 + TreeI][10 + TreeJ].State<4) {
								Grid[20 + TreeI][10 + TreeJ].State++;
								Treecut--;
							}
						}
					}
					if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
						//Build Hut
						Ress.Huts++;
						found = 0;
						while (!found) {
							int Rcase = rand() % 900;
							int CaseI = Rcase % 30;
							int CaseJ = Rcase / 30;
							if (Grid[CaseI][CaseJ].Object == 0) {
								//check near house
								if ((Grid[CaseI + 1][CaseJ].Object >= 5 && Grid[CaseI + 1][CaseJ].Object <= 7) || (Grid[CaseI + 1][CaseJ + 1].Object >= 5 && Grid[CaseI + 1][CaseJ + 1].Object <= 7) ||
									(Grid[CaseI - 1][CaseJ].Object >= 5 && Grid[CaseI - 1][CaseJ].Object <= 7) || (Grid[CaseI - 1][CaseJ - 1].Object >= 5 && Grid[CaseI - 1][CaseJ - 1].Object <= 7)) {
									found = 1;
									Grid[CaseI][CaseJ].Object = 5;
								}
							}
						}
					}
					if (Ress.Pop >= 20)
						Eras++;
				}
				else if (Eras == 2) {
					//medieval
					Ress.Pop+=2;
					Ress.Trees-=5;
					int found = 0;
					int Treecut = 5;
					
					if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
						//Build House
						Ress.Houses++;
						Ress.Trees -= 5;
						found = 0;
						Treecut += 5;
						while (!found) {
							int Rcase = rand() % 900;
							int CaseI = Rcase % 30;
							int CaseJ = Rcase / 30;
							if (Grid[CaseI][CaseJ].Object == 0) {
								//check sides == house
								if ((Grid[CaseI + 1][CaseJ].Object >= 5 && Grid[CaseI + 1][CaseJ].Object <= 7) || (Grid[CaseI + 1][CaseJ + 1].Object >= 5 && Grid[CaseI + 1][CaseJ + 1].Object <= 7) ||
									(Grid[CaseI - 1][CaseJ].Object >= 5 && Grid[CaseI - 1][CaseJ].Object <= 7) || (Grid[CaseI - 1][CaseJ - 1].Object >= 5 && Grid[CaseI - 1][CaseJ - 1].Object <= 7)) {
									found = 1;
									Grid[CaseI][CaseJ].Object = 6;
								}
							}
						}
					}
					while (Treecut > 0 && Ress.Trees>0) {
						int Rtree = rand() % 101; //destroy random tree
						int TreeI = Rtree % 10; //10col for 10line
						int TreeJ = Rtree / 10;
						if (Grid[20 + TreeI][10 + TreeJ].State < 4) {
							while (Treecut > 0 && Grid[20 + TreeI][10 + TreeJ].State < 4) {
								Grid[20 + TreeI][10 + TreeJ].State++;
								Treecut--;
							}
						}
					}
					if (Ress.Pop >= 100)
						Eras++;
				}
				else if (Eras == 3) {
					//Contemporary
					Ress.Pop += 4;
					Ress.Trees -= 10;
					int found = 0;
					int Treecut = 10;
					if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
						//Build Appart
						Ress.Apparts++;
						Ress.Trees -= 10;
						Treecut += 10;
						found = 0;
						while (!found) {
							int Rcase = rand() % 900;
							int CaseI = Rcase % 30;
							int CaseJ = Rcase / 30;
							if (Grid[CaseI][CaseJ].Object == 0) {
								//check near house
								if ((Grid[CaseI + 1][CaseJ].Object >= 5 && Grid[CaseI + 1][CaseJ].Object <= 7) || (Grid[CaseI + 1][CaseJ + 1].Object >= 5 && Grid[CaseI + 1][CaseJ + 1].Object <= 7) ||
									(Grid[CaseI - 1][CaseJ].Object >= 5 && Grid[CaseI - 1][CaseJ].Object <= 7) || (Grid[CaseI - 1][CaseJ - 1].Object >= 5 && Grid[CaseI - 1][CaseJ - 1].Object <= 7)) {
									found = 1;
									Grid[CaseI][CaseJ].Object = 7;
								}
							}
						}
					}
					while (Treecut > 0 && Ress.Trees>0) {
						int Rtree = rand() % 101; //destroy random tree
						int TreeI = Rtree % 10; //10col for 10line
						int TreeJ = Rtree / 10;
						if (Grid[20 + TreeI][10 + TreeJ].State < 4) {
							while (Treecut > 0 && Grid[20 + TreeI][10 + TreeJ].State < 4) {
								Grid[20 + TreeI][10 + TreeJ].State++;
								Treecut--;
							}
						}
					}
				}
			}
			
			PastYear = Year;
		}
		if (Ress.Trees <= 0) {
			EndMain = 0;
		}
		SDL_Delay(10);
		Evenement();
		Afficher();
	}
	return EXIT_SUCCESS;

}