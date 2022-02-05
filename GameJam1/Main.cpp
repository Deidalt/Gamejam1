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
Ressources Ress = { 0 };
int Year = -1;
int timegame = 0;
SDL_Point posMap = { 0,0 };
SDL_Point LastMove = { 0,0 }; //1=Droite , 2 =Gauche, 3=Bas, 4=Haut
int Eras = 1; //1=tribal, 2=medieval, 3=contemporain
float Zoom = 1; //same camera for all resolutions
SDL_Point ScreenL = { 0,0 }; //my Reso
int Revenge = 0;

static inline void TribalEra();
static inline void MedievalEra();
static inline void ContemporaryEra();

static bool IsType(int i, int j, CaseType type);
static bool IsRiverLocation(int i, int j);
static bool IsSeaLocation(int i, int j);
static bool IsForestLocation(int i, int j);
static bool IsNearHouse(int i, int j);

int main(int argc, char* argv[])
{
	SDL_SetMainReady();
	InitAffichage();

	srand((int)time(NULL));
	int i = 0, j = 0;
	int PastYear = 0; //Check when year changes
	
	for (i = 0;i < LMAP;i++) { //Init Grille //init map
		for (j = 0;j < HMAP;j++) {
			if (j < 3)
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
	Ress.Trees = MAXTREES;
	while (EndMain) {
		if (Year >= 0) {
			Year = (SDL_GetTicks() - timegame) / TIMETURN; //+1 Year every 2 sec 
			if (PastYear != Year) {
				//Year changed, Turn calculs
				if(Ress.Animals)
					Ress.Animals++;
				Ress.Hunt = 0;
				//Human turn
				if (Eras == 1) {
					TribalEra();
				}
				else if (Eras == 2) {
					MedievalEra();
				}
				else if (Eras == 3) {
					ContemporaryEra();
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

static inline void RemoveRandomTrees() {
	int nbTreeCut = Ress.Treecut;
	while (nbTreeCut > 0 && Ress.Trees > 0) {

		int Rtree = rand() % (FOREST_H* FOREST_W+1); //destroy random tree
		int i = Rtree % FOREST_H; //10col for 10line
		int j = Rtree / FOREST_H;
		if (Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
			while (nbTreeCut > 0 && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
				Grid[COL_FOREST + i][LINE_FOREST + j].State++;
				--nbTreeCut;
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
	return j > 24;
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

static bool IsHouse(int i, int j) {
	if (0 <= i && i < LMAP && 0 <= j && j < HMAP) {
		const CaseType caseType = Grid[i][j].Object;
		return caseType == HUT || caseType == HOUSE || caseType == APPART;
	}

	return false;
}

static bool IsNearHouse(int i, int j) {
	return IsHouse(i + 1, j) || IsHouse(i + 1, j + 1) || IsHouse(i - 1, j) || IsHouse(i - 1, j - 1);
}

void Hunt() {
	Ress.Food -= 5;
	Ress.Hunt++;
	Ress.Animals--;
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

void BuildHouse() {
	Ress.Houses++;
	Ress.Trees -= 5;
	int found = 0;
	Ress.Treecut += 5;
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

void BuildShip() {
	Ress.Fish++;
	Ress.Treecut++;
}

void BuildMill() {
	Grid[14][20].Object = MILL;
	Grid[14][20].State = 1;
	Ress.Treecut++;
}

void BuildFields() {
	int cptHunt = Ress.Hunt + (Ress.Fish - 1) * 2; //Les bateaux de peche en trop seront remplac�s par des champs
	Ress.Fish = 1;
	while (cptHunt) {
		cptHunt--;
		Ress.Harvest++;
		Ress.Treecut++;
		for (int i = 14;i > 0;i--) {
			for (int j = 20;j < 27;j++) {
				if (Grid[i][j].Object == EMPTY_CASE) {
					if (Grid[i + 1][j].Object == FIELD || Grid[i - 1][j].Object == FIELD || Grid[i][j - 1].Object == FIELD || Grid[i][j + 1].Object == FIELD) {
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

static inline void TribalEra() {
	Ress.Pop++;
	
	Ress.Food = Ress.Pop; //total Ress.Food to search per turn
	Ress.Food -= 5; //gathering
	while (Ress.Food > 0 && Ress.Animals > 0) {
		Hunt();
	}
	Ress.Treecut = 2;
	RemoveRandomTrees();

	if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
		BuildHut();
	}

	if (Ress.Pop >= 20)
		Eras++;
}

void MedievalEra() {
	Ress.Pop += 2;
	int found = 0;
	Ress.Treecut = 5;
	Ress.Food = Ress.Pop; //total Ress.Food to search per turn
	Ress.Food -= 5;
	if (Revenge) {
		Revenge = 0;
		Ress.Food -= 10;
		Ress.Animals -= 3;
		Ress.Hunt += 2;
	}
	while (Ress.Food > 0) {
		if (Ress.Fish) {
			//1 fisher
			Ress.Food -= 10;

		}
		if (Ress.River > 0) {
			//Harvest
			Ress.Food -= Ress.Harvest * 5;
		}
		if (Ress.Food > 0 && Ress.Animals > 0) {
			Hunt();
		}
		else while (Ress.Food > 0) {
			//build new temporary boat
			Ress.Food -= 10;
			BuildShip();
		}
	} //end Ress.Food
	if (Grid[14][20].Object != MILL) {
		BuildMill();

	}
	if (Ress.Fish == 0) {
		BuildShip();
	}
	if (Ress.River) {
		BuildFields();
	}
	if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
		BuildHouse();
	}
	RemoveRandomTrees();

	if (Ress.Pop >= 100)
		Eras++;
}

void ContemporaryEra() {
	Ress.Pop += 4;
	int found = 0;
	Ress.Treecut = 10;
	if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
		//Build Appart
		Ress.Apparts++;
		Ress.Trees -= 10;
		Ress.Treecut += 10;
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
	RemoveRandomTrees();
}

void Plant(int nbTreesAdded) {
	int Rtree = rand() % 101; //destroy random tree
	int i = Rtree % FOREST_H; //10col for 10line
	int j = Rtree / FOREST_H;
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
		// @TODO: Game over -- hunter compt� comme population ?
	}
}

void Devour() {
	if (Ress.Animals > 0 && Ress.Hunt > 0) {
		--Ress.Hunt;
		if (Eras == 2) {
			//Revenge //battue au prochain tour
			Revenge = 1;
		}
	}
}

void Drown() {
	// @TODO: commencer � une case random pour �tre moins lin�aire ?
	for (int i = 0; i < LMAP; ++i) {
		for (int j = 25; j < HMAP; ++j) {
			if (Grid[i][j].Object == SHIP) {
				Grid[i][j].Object = SEA;
				

				return;
			}
		}
	}
}
