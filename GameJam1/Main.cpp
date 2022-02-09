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
int fire = 0; //fire event
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
void NoAction();

void Fire();

bool IsGlacialSeason();
bool IsDrySeason();
static inline int GetColdResistance();

static inline void ManageSeasons();

int main(int argc, char* argv[])
{
	SDL_SetMainReady();
	InitAffichage();

	srand((int)time(NULL));
	int i = 0, j = 0;
	int PastYear = 0; //Check when year changes
	void (*actions[])() = { Plant, Rain, Cold, Meteor, Devour, Drown, NoAction };
	lastAction = PLANT;
	SDL_Surface* HitboxRiverS = IMG_Load("Assets/Map/MapHitbox.png");

	Ress.River = 1;
	Ress.Harvest = 0;
	Ress.Fish = 0;
	Ress.Animals = 10;
	Ress.Trees = 0;
	triggerCold = 0;
	rain = 0;
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
	Grid[5][10].Object = HUT;
	
	Year = 0; //Game Starts here
	timegame = SDL_GetTicks();
	
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
		if (IsGlacialSeason()) {
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
	if (rain != 0) {
		++rain;
		if (rain == 7) {
			rain = 0;
		}
	}

	if (IsDrySeason()) {
		if (fire > 0 || rand() % 100 < 50 ) { //3%
			Fire(); //incendie
		}
	}
	
	if (riverDryness < 30)
		++riverDryness;
	if (riverDryness > 5 && Ress.River == 2) {
		riverDryness = 0;
		Ress.River = 1;
	}
	if(riverDryness>=30)
		Ress.River = 0;
}

static inline void RemoveRandomTrees() {
	int nbTreeCut = Ress.Treecut;
	while (nbTreeCut > 0 && Ress.Trees > 0) {

		int Rtree = rand() % (FOREST_H* FOREST_W+1); //destroy random tree
		int i = Rtree % FOREST_H; //10col for 10line
		int j = Rtree / FOREST_H;
		if (Grid[COL_FOREST + i][LINE_FOREST + j].Object == FOREST && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
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
	if (i < 19 && j>15)
		return 0; 
	if (i >= COL_FOREST && i < COL_FOREST + FOREST_W && j >= LINE_FOREST && j < LINE_FOREST + FOREST_H && Grid[i][j].Object == EMPTY_CASE) {
		if (rand() % 2) {
			Ress.Trees += 4;
			return 1;
		}
	}
	return 0;
	//return i >= COL_FOREST && i < COL_FOREST+FOREST_W && j >= LINE_FOREST && j < LINE_FOREST+FOREST_H;
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

bool IsGlacialSeason() {
	return (Year / YEARS_PER_SEASON) % 4 == 3;
}

bool IsDrySeason() {
	return (Year / YEARS_PER_SEASON) % 4 == 1;
}

static inline void BuildHut() {
	bool found = false;

	Ress.Huts++;

	while (!found) {
		int Rcase = rand() % 900;
		int i = Rcase % 30;
		int j = Rcase / 30;

		if (Grid[i][j].Object == EMPTY_CASE || (Grid[i][j].Object == FOREST && Grid[i][j].State == 4)) {
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
		if (Grid[CaseI][CaseJ].Object == EMPTY_CASE || (Grid[CaseI][CaseJ].Object == FOREST && Grid[CaseI][CaseJ].State == 4)) {
			//check sides == house
			if (IsNearHouse(CaseI, CaseJ)) {
				found = 1;
				Grid[CaseI][CaseJ].Object = HOUSE;
			}
		}
	}
}

void BuildAppart() {
	Ress.Apparts++;
	Ress.Trees -= 10;
	int found = 0;
	Ress.Treecut += 10;
	while (!found) {
		int Rcase = rand() % 900;
		int CaseI = Rcase % LMAP;
		int CaseJ = Rcase / LMAP;
		if (Grid[CaseI][CaseJ].Object == EMPTY_CASE || (Grid[CaseI][CaseJ].Object == FOREST && Grid[CaseI][CaseJ].State == 4)) {
			//check sides == house
			if (IsNearHouse(CaseI, CaseJ)) {
				found = 1;
				Grid[CaseI][CaseJ].Object = APPART;
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
		if (Grid[1+CaseI][28+CaseJ].Object == SEA) {
			Built = 1;
			Grid[1+CaseI][28+CaseJ].Object = SHIP;
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
	if ( Ress.River) { //works while flood
		if (Grid[13][20].Object != MILL) {
			BuildMill();
		}
		BuildFields();
	}
	while (Ress.Food > 0) {
		int fishing = Ress.Fish;
		while (Ress.Food > 0 && fishing) {
			//1 fisher
			Ress.Food -= 10;

		}
		while (Ress.Food > 0 && Ress.River > 0) { /// Pour gérer sécheresse ?
			//Harvest
			Ress.Food -= (int)(Ress.Harvest * 5 * GetFieldProductivity());
		}
		while (Ress.Food > 0 && Ress.Animals > 0) {
			Hunt();
		}
		while (Ress.Food > 0) {
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

void NoAction() {}

void Plant() {
	if (fire > 0)
		return;
	int nbTreesAdded = 2;
	int Rtree = rand() % 101; //destroy random tree
	int i = Rtree % FOREST_H; //10col for 10line
	int j = Rtree / FOREST_H;
	int count = 0; // Avoid looping endlessly if the forest cannot take any more trees

	for (; count < FOREST_W * FOREST_H && nbTreesAdded > 0 && Ress.Trees > 0; j = (j + 1) % FOREST_H, ++count) {
		if (Grid[COL_FOREST + i][LINE_FOREST + j].Object == FOREST && Grid[COL_FOREST + i][LINE_FOREST + j].State > 0) {
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

void Fire() {
	int found = 0;
	if (fire == 0) {
		//first fire
		while (Ress.Trees > 0 && found == 0) {
			int Rtree = rand() % (FOREST_H * FOREST_W + 1); //fire random tree
			int i = Rtree % FOREST_H; //10col for 10line
			int j = Rtree / FOREST_H;
			if (Grid[COL_FOREST + i][LINE_FOREST + j].Object == FOREST && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
				Grid[COL_FOREST + i][LINE_FOREST + j].State += 5;
				Ress.Trees -= Grid[COL_FOREST + i][LINE_FOREST + j].State - 4;
				fire++;
				found = 1;
			}
		}
	}
	else {
		int cptfire = fire + 30; //fire exponential
		while (Ress.Trees > 0 && cptfire > 0) {
			int Rtree = rand() % (FOREST_H * FOREST_W + 1); //fire random tree
			int i = Rtree % FOREST_H; //10col for 10line
			int j = Rtree / FOREST_H;
			if (Grid[COL_FOREST + i][LINE_FOREST + j].Object == FOREST && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
				if (Grid[COL_FOREST + i+1][LINE_FOREST + j].State > 4 || Grid[COL_FOREST + i - 1][LINE_FOREST + j].State > 4 || Grid[COL_FOREST + i][LINE_FOREST + j+1].State > 4 || Grid[COL_FOREST +i][LINE_FOREST + j-1].State > 4) {
					Grid[COL_FOREST + i][LINE_FOREST + j].State += 5;
					Ress.Trees -= Grid[COL_FOREST + i][LINE_FOREST + j].State - 4;
					fire++;
				}
			}
			cptfire--;
		}
	}
}

void Rain() {
	if (rain == 0) {
		if (Ress.River) { // Flood
			Ress.River = 2;
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
		else
			Ress.River = 1;
		if (fire > 0) {
			for (int i = COL_FOREST; i < COL_FOREST+FOREST_W; i++) {
				for (int j = LINE_FOREST; j < LINE_FOREST+FOREST_H; j++) {
					if (IsType(i, j, FOREST) && Grid[i][j].State > 4) {
						//extinguished fire
						fire = 0;
						Grid[i][j].State = 4;
					}
				}
			}
		}
		
		riverDryness = 0;
		rain = 1;
	}
}

void Cold() {
	if (IsGlacialSeason()) {
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
	bool incendie = fire > 0;
	if ((action == METEOR && era == TRIBAL) || (action == RAIN && IsDrySeason()) || (action == COLD && !IsGlacialSeason()) || (incendie && action == PLANT)) {
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

bool IsColdOn() {
	return triggerCold > 0;
}

int GetSickNumber() {
	return (triggerCold == 0 ? 0 : (int)(Ress.Pop * 0.05));
}

const char* GetEraName() {
	static const char* names[] = { "Automne", "Été", "Printemps", "Hiver" };
	return names[(Year / YEARS_PER_SEASON) % 4];
}