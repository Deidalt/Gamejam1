#define SDL_MAIN_HANDLED

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
Eras era = TRIBAL; //1=tribal, 2=medieval, 3=contemporain
float Zoom = 1; //same camera for all resolutions
SDL_Point ScreenL = { 0,0 }; //my Reso
Actions lastAction;

static inline void TribalEra();

static bool IsType(int i, int j, CaseType type);
static bool IsRiverLocation(int i, int j);
static bool IsSeaLocation(int i, int j);
static bool IsForestLocation(int i, int j);

static bool IsNearHouse(int i, int j);

void Plant();
void Rain();
void Cold();
void Meteor();
void Devour();
void Drown();

int main(int argc, char* argv[])
{
	SDL_SetMainReady();

	InitAffichage();

	srand((int)time(NULL));
	int i = 0, j = 0;
	int PastYear = 0; //Check when year changes

	void (*actions[])() = { Plant, Rain, Cold, Meteor, Devour, Drown };
	lastAction = PLANT;
	
	for (i = 0;i < LMAP;i++) { //Init Grille //init map
		for (j = 0;j < HMAP;j++) {
			if (j < 2)
				Grid[i][j].Object = MOUNTAIN;
			else if (IsSeaLocation(i, j))
				Grid[i][j].Object = SEA;
			else if (IsRiverLocation(i, j))
				Grid[i][j].Object = RIVER;
			else if (IsForestLocation(i, j))
				Grid[i][j].Object = FOREST;
		}
		Grid[i][j].State = 0;
	}
	Grid[10][10].Object = HUT;
	
	Year = 0; //Game Starts here
	timegame = SDL_GetTicks();
	Ress.River = 1;
	Ress.Harvest = 0;
	Ress.Fish = 0;
	Ress.Animals = 10;
	while (EndMain) {
		if (Year >= 0) {
			Year = (SDL_GetTicks() - timegame) / TIMETURN; //+1 Year every 2 sec

			if (PastYear != Year) {
				//Year changed, Turn calculs
				if(Ress.Animals)
					Ress.Animals++;
				Ress.Hunt = 0;
				
				// Repeat last action each turn
				actions[lastAction]();
				
				//Human turn
				if (era == TRIBAL) {
					TribalEra();
				}
				else if (era == MEDIEVAL) {
					//medieval
					Ress.Pop+=2;
					Ress.Trees-=5;
					int found = 0;
					int Treecut = 5;
					int food = Ress.Pop; //total food to search per turn
					food -= 5;
					while (food > 0) {
						if (Ress.Fish) {
							//1 fisher
							food -= 10;
							
						}
						if (Ress.River > 0) {
							//Harvest
							int cptHarvest = Ress.Harvest;
							while (cptHarvest) {
								food -= 5;
								cptHarvest--;
							}
						}
						if (food >0 && Ress.Animals > 0) {
							//Hunt
							food -= 5;
							Ress.Hunt++;
							Ress.Animals--;
						}
						else while (food > 0) {
							//build new temporary boat
							food -= 10;
							Ress.Fish++;
							Treecut++;
						}
					} //end food
					if (Grid[14][20].Object != MILL) {
						//build Mill
						Grid[14][20].Object = MILL;
						Grid[14][20].State = 1;
						Treecut++;
					}
					if (Ress.Fish == 0) {
						//Build boat
						Ress.Fish = 1;
						Treecut += 1;
					}
					if (Ress.River) {
						//build fields
						int cptHunt = Ress.Hunt + (Ress.Fish-1)*2; //Les bateaux de peche en trop seront remplacés par des champs
						Ress.Fish = 1;
						while (cptHunt) {
							cptHunt--;
							Ress.Harvest++;
							Treecut++;
							for (i = 14;i > 0;i--) {
								for (j = 20;j < 27;j++) {
									if (Grid[i][j].Object == EMPTY_CASE) {
										if (Grid[i+1][j].Object == FIELD || Grid[i - 1][j].Object == FIELD || Grid[i][j-1].Object == FIELD || Grid[i][j+1].Object == FIELD) {
											Grid[i][j].Object = FIELD;
											Grid[i][j].State = 2;
											i = 0;
											j = 30;
										}
									}
								}
							}
						}
					}
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
							if (Grid[CaseI][CaseJ].Object == EMPTY_CASE) {
								//check sides == house
								if (IsNearHouse(CaseI, CaseJ)) {
									found = 1;
									Grid[CaseI][CaseJ].Object = HOUSE;
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
						era = CONTEMPORARY;
				}
				else if (era == CONTEMPORARY) {
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
							if (Grid[CaseI][CaseJ].Object == EMPTY_CASE) {
								//check near house
								if (IsNearHouse(CaseI, CaseJ)) {
									found = 1;
									Grid[CaseI][CaseJ].Object = APPART;
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
		SDL_Delay(1);
		Evenement();
		Afficher();
	}
	return EXIT_SUCCESS;

}

static inline void HuntTribal() {
	int food = Ress.Pop; //total food to search per turn

	food -= 5;

	while (food > 0 && Ress.Animals > 0) {
		//Hunt
		food -= 5;
		Ress.Hunt++;
		Ress.Animals--;
	}
}

static inline void RemoveRandomTrees(int nbTreesCut) {
	while (nbTreesCut > 0 && Ress.Trees > 0) {
		int Rtree = rand() % 101; //destroy random tree
		int i = Rtree % 10; //10col for 10line
		int j = Rtree / 10;

		if (Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
			while (nbTreesCut > 0 && Grid[20 + i][10 + j].State < 4) {
				Grid[COL_FOREST + i][LINE_FOREST + j].State++;
				--nbTreesCut;
				--Ress.Trees; //trees consumed per turn
			}
		}
	}
}

static bool IsRiverLocation(int i, int j) {
	(void)j;
	return 14 < i && i < 17;
}

static bool IsSeaLocation(int i, int j) {
	(void)i;
	return j > 27;
}

static bool IsForestLocation(int i, int j) {
	return i >= 20 && i < 30 && j >= 10 && j < 20;
}

static bool IsType(int i, int j, CaseType type) {
	if (0 <= i && i < LMAP && 0 <= j && j < HMAP) {
		const CaseType caseType = Grid[i][j].Object;
		return caseType == type;
	}

	return false;
}

static bool IsBuilding(int i, int j) {
	if (0 <= i && i < LMAP && 0 <= j && j < HMAP) {
		const CaseType caseType = Grid[i][j].Object;
		return caseType == HUT || caseType == HOUSE || caseType == APPART;
	}

	return false;
}

static bool IsNearHouse(int i, int j) {
	return IsBuilding(i + 1, j) || IsBuilding(i + 1, j + 1) || IsBuilding(i - 1, j) || IsBuilding(i - 1, j - 1);
}

static inline void BuildHut() {
	bool found = false;

	Ress.Huts++;

	while (!found) {
		int Rcase = rand() % 900;
		int i = Rcase % 30;
		int j = Rcase / 30;

		if (Grid[i][j].Object == EMPTY_CASE) {
			//check near house
			if (IsNearHouse(i, j)) {
				
				found = true;
				Grid[i][j].Object = HUT;
			}
		}
	}
}

static inline void TribalEra() {
	Ress.Pop++;
	
	int found = 0;

	HuntTribal();
	RemoveRandomTrees(2);

	if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
		BuildHut();
	}

	if (Ress.Pop >= 20)
		era = MEDIEVAL;
}

void Plant() {
	int nbTreesAdded = 1;
	int Rtree = rand() % 101; //destroy random tree
	int i = Rtree % 10; //10col for 10line
	int j = Rtree / 10;
	int count = 0; // Avoid looping endlessly if the forest cannot take any more trees

	for (; count < FOREST_W * FOREST_H && nbTreesAdded > 0 && Ress.Trees > 0; j = (j + 1) % FOREST_H, ++count) {
		if (Grid[COL_FOREST + i][LINE_FOREST + j].State > 0) {
			while (nbTreesAdded > 0 && Grid[COL_FOREST + i][LINE_FOREST + j].State > 0) {
				Grid[COL_FOREST + i][LINE_FOREST + j].State--;
				--nbTreesAdded;
				++Ress.Trees;
			}
		}

		if (j == FOREST_H - 1) {
			i = (i + 1) % FOREST_W;
		}
	}
}

void Rain() {
	// @TODO
}

void Cold() {
	// @TODO
}

void Meteor() {
	Ress.Pop -= 10;
	if (Ress.Pop < 0) {
		// @TODO: Game over -- hunter compté comme population ?
	}
}

void Devour() {
	if (Ress.Animals > 0 && Ress.Hunt > 0) {
		--Ress.Hunt;
	}
}

void Drown() {
	// @TODO: commencer à une case random pour être moins linéaire ?
	for (int i = 0; i < LMAP; ++i) {
		for (int j = 0; j < HMAP; ++j) {
			if (Grid[i][j].Object == SHIP) {
				if (IsRiverLocation(i, j)) {
					Grid[i][j].Object = RIVER;
				}
				else {
					Grid[i][j].Object = SEA;
				}

				return;
			}
		}
	}
}

void SetAsAction(Actions action) {
	if (action == METEOR && era == TRIBAL) {
		return;
	}

	lastAction = action;
}

Actions getCurrentAction() {
	return lastAction;
}
