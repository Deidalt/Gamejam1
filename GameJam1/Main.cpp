#define SDL_MAIN_HANDLED

#include <signal.h>
#include <SDL_image.h>
#include "Main.h"
#include "Affichage.h"
#include "Evenements.h"
#include "Pixel.h"

//Globals
SDL_Window* Screen = NULL;
SDL_Renderer *Renderer;
int EndMain = 1;
Case Grid[LMAP][HMAP];
Ressources Ress = { 0 };
int Year = -1;
int timegame = 0;
SDL_Point posxy = { 0,0 };
SDL_Point LastMove = { 0,0 }; //1=Droite , 2 =Gauche, 3=Bas, 4=Haut
Eras era = TRIBAL; //1=tribal, 2=medieval, 3=contemporain
float Zoom = 1; //same camera for all resolutions
float Zoom7K = 1;
SDL_Point ScreenL = { 0,0 }; //my Reso
Actions lastAction;
int Revenge = 0; //Humans passif
int triggerCold = 0; //Cold action
int riverDryness = 0;
int rain = 0; //rain action
int Menu = 0; //1 = Menu IG

static inline void TribalEra();
static inline void MedievalEra();
static inline void ContemporaryEra();

static bool IsType(int i, int j, CaseType type);
static bool IsRiverLocation(SDL_Surface* surf, int i, int j);
static bool IsSeaLocation(int i, int j);
static bool IsForestLocation(int i, int j);
static bool IsNearHouse(int i, int j);

void Plant();
void Rain();
void Cold();
void Meteor();
void Devour();
void Drown();

static inline bool IsGlacialEpoch();
static inline bool IsDryEpoch();
static inline int GetColdResistance();

static inline void ManageSeasons();

int main(int argc, char* argv[])
{
	SDL_SetMainReady();
	InitAffichage();

	srand((int)time(NULL));
	int i = 0, j = 0;
	int PastYear = 0; //Check when year changes
	void (*actions[])() = { Plant, Rain, Cold, Meteor, Devour, Drown };
	lastAction = PLANT;
	SDL_Surface* HitboxRiverS = IMG_Load("Assets/Map/MapHitbox.png");

	for (i = 0; i < LMAP; i++) { //Init Grille //init map
		for (j = 0; j < HMAP; j++) {
			if (j < 3)
				Grid[i][j].Object = MOUNTAIN;
			else if (IsSeaLocation(i, j))
				Grid[i][j].Object = SEA;
			else if (IsRiverLocation(HitboxRiverS, i, j))
				Grid[i][j].Object = RIVER;
			else if (IsForestLocation(i, j))
				Grid[i][j].Object = FOREST;
		}
		Grid[i][j].State = 0;
	}
	SDL_FreeSurface(HitboxRiverS);
	Grid[10][10].Object = HUT;
	
	Year = 0; //Game Starts here
	timegame = SDL_GetTicks();
	Ress.River = 1;
	Ress.Harvest = 0;
	Ress.Fish = 0;
	Ress.Animals = 10;
	Ress.Trees = MAXTREES;
	triggerCold = 0;
	rain = 0;
	while (EndMain) {
		if (Year >= 0 && Menu==0) {
			Year = (SDL_GetTicks() - timegame) / TIMETURN; //+1 Year every 2 sec

			if (PastYear != Year) {
				//Year changed, Turn calculs
				if(Ress.Animals)
					Ress.Animals++;
				Ress.Hunt = 0;
				
				// Repeat last action each turn
				actions[lastAction]();

				ManageSeasons();
				
				//Human turn
				if (era == TRIBAL) {
					TribalEra();
				}
				else if (era == MEDIEVAL) {
					MedievalEra();
				}
				else if (era == CONTEMPORARY) {
					ContemporaryEra();
				}
			}
			
			PastYear = Year;
		}
		if (Ress.Trees <= 0) {
			Menu = 1; //Game Lost
		}
		SDL_Delay(1);
		Evenement();
		Afficher();
	}
	return EXIT_SUCCESS;

}

static inline int GetColdResistance() {
	if (era == TRIBAL) {
		return 5;
	}
	else {
		return 10;
		// @TODO: gérer hopital
	}
}

static inline float GetFieldProductivity() {
	return (30.f - (riverDryness < 30 ? riverDryness : 30)) / 30.f;
}

static inline void ManageSeasons() {
	if (triggerCold > 0) {
		if (IsGlacialEpoch()) {
			if (triggerCold >= GetColdResistance()) { // Note : pas de game over possible par le froid
				int dead = (int)(0.05 * Ress.Pop);
				Ress.Pop -= dead;
			}
			else {
				++triggerCold;
			}
		}
		else {
			triggerCold = 0; // Only active during the glacial epoch
		}
	}

	if (riverDryness < 30)
		++riverDryness;
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

static bool IsRiverLocation(SDL_Surface *surf, int i, int j) {
	if(surf==NULL)
		printf("isRiver error : %s\n",SDL_GetError());
	Uint8 r, g, b;
	GetRGBPixel(surf, i, j, &r, &g, &b);
	(void)j;
	return r==0 && g==0 && b==255;
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

static bool IsHabitation(int i, int j) {
	if (0 <= i && i < LMAP && 0 <= j && j < HMAP) {
		const CaseType caseType = Grid[i][j].Object;
		return caseType == HUT || caseType == HOUSE || caseType == APPART;
	}

	return false;
}

static bool IsNearHouse(int i, int j) {
	return IsHabitation(i + 1, j) || IsHabitation(i + 1, j + 1) || IsHabitation(i - 1, j) || IsHabitation(i - 1, j - 1);
}

void Hunt() {
	Ress.Food -= 5;
	Ress.Hunt++;
	Ress.Animals--;
}

static inline bool IsGlacialEpoch() {
	return Year / YEARS_PER_SEASON == 1;
}

static inline bool IsDryEpoch() {
	return Year / YEARS_PER_SEASON == 3;
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
	Ress.Trees -= 4;
	int found = 0;
	Ress.Treecut += 4;
	while (!found) {
		int Rcase = rand() % 900;
		int CaseI = Rcase % LMAP;
		int CaseJ = Rcase / LMAP;
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
	int Built = 0;
	while (!Built) {
		int Rcase = rand() % ((LMAP - 2) * 2) + 1; //cases au centre de la mer
		int CaseJ = Rcase / (LMAP - 2);
		int CaseI = Rcase % (LMAP - 2);
		if (Grid[1+CaseI][25+CaseJ].Object == SEA) {
			Built = 1;
			Grid[1+CaseI][25+CaseJ].Object = SHIP;
		}
	}
}

void BuildMill() {
	Grid[13][20].Object = MILL; 
	Ress.Treecut++;
}

void BuildFields() {
	//int cptHunt = Ress.Hunt + (Ress.Fish - 1) * 2; //Les bateaux de peche en trop seront remplacés par des champs
	//Ress.Fish = 1;
	while (Ress.Food + 10 > Ress.Harvest * 5) {
		//cptHunt--;
		Ress.Harvest++;
		Ress.Treecut++;
		for (int i = 14;i > 0;i--) {
			for (int j = 20;j < 27;j++) {
				if (Grid[i][j].Object == EMPTY_CASE) {
					if (Grid[i + 1][j].Object == FIELD || Grid[i - 1][j].Object == FIELD || Grid[i][j - 1].Object == FIELD || Grid[i][j + 1].Object == FIELD) {
						Grid[i][j].Object = FIELD;
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
		era = MEDIEVAL;
}

void MedievalEra() {
	Ress.Pop += 2;
	int found = 0;
	Ress.Treecut = 2;
	Ress.Food = Ress.Pop; //total Ress.Food to search per turn
	Ress.Food -= 5;
	if (Revenge) {
		Revenge = 0;
		Ress.Food -= 10;
		Ress.Animals -= 3;
		Ress.Hunt += 2;
	}
	if ( Ress.River) {
		if (Grid[13][20].Object != MILL) {
			printf("AAB\n");
			BuildMill();

		}
		BuildFields();
	}
	while (Ress.Food > 0) {
		if (Ress.Fish) {
			//1 fisher
			Ress.Food -= 10;

		}
		if (Ress.River > 0) { /// Pour gérer sécheresse ?
			//Harvest
			Ress.Food -= (int)(Ress.Harvest * 5 * GetFieldProductivity());
		}
		if (Ress.Food > 0 && Ress.Animals > 0) {
			Hunt();
		}
		else while (Ress.Food > 0) {
			//build new temporary Ship
			Ress.Food -= 10;
			BuildShip();
		}
	} //end Ress.Food
	
	if (Ress.Fish == 0) {
		BuildShip();
	}
	
	if (Ress.Pop > Ress.Huts + Ress.Houses + Ress.Apparts) {
		BuildHouse();
	}
	RemoveRandomTrees();

	if (Ress.Pop >= 100)
		era = CONTEMPORARY;
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

void Plant() {
	int nbTreesAdded = 2;
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

static void FloodCase(int i, int j) {
	if (IsHabitation(i, j)) {
		Grid[i][j].Object = EMPTY_CASE;
		Ress.Pop -= 1;
	}
	else if (IsType(i, j, MILL)) {
		Grid[i][j].Object = EMPTY_CASE;
	}
}

void Rain() {
	if (rain == 0) {
		if (riverDryness == 0) { // Flood
			for (int i = 0; i < LMAP; i++) {
				for (int j = 0; j < HMAP; j++) {
					if (IsType(i, j, RIVER)) {
						FloodCase(i - 1, j);
						FloodCase(i + 1, j);

						FloodCase(i - 1, j - 1);
						FloodCase(i - 1, j + 1);
						FloodCase(i + 1, j - 1);
						FloodCase(i + 1, j + 1);

						FloodCase(i, j - 1);
						FloodCase(i, j + 1);
					}
				}
			}
		}

		riverDryness = 0;
		rain = 1;
	}

	++rain;

	if (rain >= 7) {
		// Reset rain
		lastAction = PLANT;
		rain = 0;
	}
}

void Cold() {
	if (IsGlacialEpoch()) {
		triggerCold = 1;
	}
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
		--Ress.Pop;
		if (era == MEDIEVAL) {
			//Revenge //battue au prochain tour
			Revenge = 1;
		}
	}
}

void Drown() {
	// @TODO: commencer à une case random pour être moins linéaire ?
	for (int i = 0; i < LMAP; ++i) {
		for (int j = 0; j < HMAP; ++j) {
			if (Grid[i][j].Object == SHIP) {
				Grid[i][j].Object = SEA;
				return;
			}
		}
	}
}

void SetAsAction(Actions action) {
	if ((action == METEOR && era == TRIBAL) || (action == RAIN && IsDryEpoch()) || (action == COLD && !IsGlacialEpoch())) {
		return;
	}
	else if (rain > 0) {
		//return; // No actions possible during the rain
	}

	lastAction = action;
}

Actions getCurrentAction() {
	return lastAction;
}
