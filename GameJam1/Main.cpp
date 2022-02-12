//#define SDL_MAIN_HANDLED

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
Eras era = TRIBAL; //0
float Zoom = 1; //same camera for all resolutions
float Zoom7K = 1;
float Zoom2K = 1;
SDL_Point ScreenL = { 0,0 }; //my Reso
Actions lastAction;
int Revenge = 0; //Humans passif
int triggerCold = 0; //Cold action
int riverDryness = 0;
int rain = 0; //rain action
int fire = 0; //fire event
int avalanche = 0;//avalanche event
int tsunami = 0;//tsunami event
int meteor = 0;//meteor event *CaseI*CaseJ
int hospitalCount = 0;
Menus Menu = NONE; 
int SousMenu = 0;
int ActionAuto = 0; //if not 0, action is save for when it is possible
int timeTurn = TIMETURN;

static inline void TribalEra();
static inline void MedievalEra();
static inline void ContemporaryEra();

static bool IsType(int i, int j, CaseType type);
static bool IsRiverLocation(SDL_Surface* surf, int i, int j);
static bool IsSeaLocation(int i, int j);
static bool initForestLocation(int i, int j);
static bool IsNearHouse(int i, int j);

void Plant();
void Rain();
void Cold();
void Meteor();
void Devour();
void Drown();
void NoAction();

void Fire();
void Avalanche();
void Tsunami();

void initGame();

bool IsGlacialSeason();
bool IsDrySeason();
static inline int GetColdResistance();

static inline void ManageSeasons();

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char* argv[])
{
	//SDL_SetMainReady();
	InitAffichage();
	srand((int)time(NULL));
	int i = 0, j = 0;
	int PastYear = 0; //Check when year changes
	void (*actions[])() = { Plant, Rain, Cold, Meteor, Devour, Drown, NoAction };
	
	SDL_Surface* HitboxRiverS = IMG_Load("Assets/Map/MapHitbox.png");

	Mix_Music* gMusic = NULL;
	//Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}
	Mix_VolumeMusic(8);
	initGame();
	
	Afficher(); //init game
	gMusic = Mix_LoadMUS("Sound/Music.wav");
	if (gMusic == NULL)
	{
		printf("Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
	}
	if (Mix_PlayingMusic() == 0)
	{
		//Play the music
		Mix_PlayMusic(gMusic, -1);
	}
	timegame = SDL_GetTicks();

	while (EndMain) {
		if (Year >= 0 && Menu!=ESCAPE) {
			PastYear = Year;
			if ((SDL_GetTicks() - timegame) / timeTurn >= 1) {
				Year += 1; //+1 Year every 2 sec
				timegame = SDL_GetTicks();
			}
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
			
			

		}
		if (Ress.Trees <= 0) {
			Menu = ESCAPE; //Game Lost
		}
		else if (Ress.Pop <= 0) {
			Menu = ESCAPE;
		}
		else if (Year >= YEARMAX) {
			Menu = ESCAPE;
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
	return (30.f - (riverDryness / era < 30 ? (float)(riverDryness / era) : 30)) / 30.f;
}

static inline void ManageSeasons() {
	if (triggerCold > 0) {
		if (IsGlacialSeason()) {
			if (triggerCold >= GetColdResistance() && hospitalCount < 1) { // Note : pas de game over possible par le froid
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
		if (rain == 8) { // 8 is 7 because rain skip 1
			rain = 0;
		}
	}

	if (IsDrySeason()) {
		ActionAuto = 1;
		if (fire > 0 || rand() % 100 < 5 ) { //5%
			Fire(); //incendie
		}
		
	}
	else {
		if (rand() % 100 < 3) { //3%
			Avalanche(); 
		}
		if (ActionAuto == 1) {
			SetAsAction(RAIN);
			ActionAuto = 0;
		}
	}
	if (avalanche) {
		avalanche--;
	}
	
	if (rand() % 100 < 2) { //2%
		Tsunami();
	}
	if (tsunami) {
		tsunami--;
	}
	if (era == CONTEMPORARY) {
		//Barrage upgrade
		if (riverDryness < 60)
			++riverDryness;
		if (riverDryness >= 60)
			Ress.River = 0;
	}
	else {
		if (riverDryness < 30)
			++riverDryness;
		if (riverDryness >= 30)
			Ress.River = 0;
	}

	if (riverDryness > 5 && Ress.River == 2) {
		riverDryness = 0;
		Ress.River = 1;
	}
	
}

static inline void RemoveRandomTrees() {
	int nbTreeCut = Ress.Treecut;
	int cptdebug = 1000;
	while (nbTreeCut > 0 && Ress.Trees > 0 && cptdebug>0) {

		int Rtree = rand() % (FOREST_H* FOREST_W+1); //destroy random tree
		int i = Rtree % FOREST_W; //10col for 10line
		int j = Rtree / FOREST_W;
		if (Grid[COL_FOREST + i][LINE_FOREST + j].Object == FOREST && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
			while (nbTreeCut > 0 && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
				Grid[COL_FOREST + i][LINE_FOREST + j].State++;
				--nbTreeCut;
				--Ress.Trees; //trees consumed per turn
			}
			i = 30;
			j = 30;//delete
		}
		cptdebug--;
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

static bool initForestLocation(int i, int j) {
	//init randomly forest locations
	if (i < 19 && j>15)
		return 0; 
	if (i >= COL_FOREST && i < COL_FOREST + FOREST_W && j >= LINE_FOREST && j < LINE_FOREST + FOREST_H && Grid[i][j].Object == EMPTY_CASE) {
		if (rand() % 100 < 70) { //Change random for trees number
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
		if(era==CONTEMPORARY)
			return caseType == HUT || caseType == HOUSE || caseType == APPART || caseType == RIVER;
		return caseType == HUT || caseType == HOUSE || caseType == APPART;
	}

	return false;
}

static bool IsNearHouse(int i, int j) {
	return IsHabitation(i + 1, j) || IsHabitation(i + 1, j + 1) || IsHabitation(i - 1, j) || IsHabitation(i - 1, j - 1);
}


void Hunt() {
	Ress.Food -= 10;
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
	int cptdebug = 1000;
	while (!found && cptdebug>0) {
		int Rcase = rand() % (LMAP*HMAP);
		int i = Rcase % 30;
		int j = Rcase / 30;
		if (Grid[i][j].Object == EMPTY_CASE || (Grid[i][j].Object == FOREST && Grid[i][j].State == 4)) {
			//check near house
			if (IsNearHouse(i, j)) {
				
				found = true;
				Grid[i][j].Object = HUT;
				Grid[i][j].id = rand() % 4;
				Grid[i][j].State = SDL_GetTicks() + 2000;
			}
		}
		cptdebug--;
	}
}

void BuildHouse() {
	Ress.Houses++;
	int found = 0;
	Ress.Treecut += 4;
	int cptdebug = 1000;
	while (!found && cptdebug > 0) {
		int Rcase = rand() % (LMAP*HMAP);
		int CaseI = Rcase % LMAP;
		int CaseJ = Rcase / LMAP;
		if (Grid[CaseI][CaseJ].Object == EMPTY_CASE || (Grid[CaseI][CaseJ].Object == FOREST && Grid[CaseI][CaseJ].State == 4)) {
			//check sides == house
			if (IsNearHouse(CaseI, CaseJ)) {
				found = 1;
				Grid[CaseI][CaseJ].Object = HOUSE;
				Grid[CaseI][CaseJ].id = rand() % 4;
				Grid[CaseI][CaseJ].State = SDL_GetTicks() + 2000;
			}
		}
		cptdebug--;
	}
}

void BuildAppart() {
	Ress.Apparts++;
	int found = 0;
	Ress.Treecut += 10;
	int cptdebug = 1000;
	while (!found && cptdebug > 0) {
		int Rcase = rand() % (LMAP*HMAP);
		int CaseI = Rcase % LMAP;
		int CaseJ = Rcase / LMAP;
		if (Grid[CaseI][CaseJ].Object == EMPTY_CASE || (Grid[CaseI][CaseJ].Object == FOREST && Grid[CaseI][CaseJ].State == 4)) {
			//check sides == house
			if (IsNearHouse(CaseI, CaseJ)) {
				found = 1;
				Grid[CaseI][CaseJ].Object = APPART;
				Grid[CaseI][CaseJ].id = rand() % 4;
				if (Grid[CaseI][CaseJ].id == 1)
					Grid[CaseI][CaseJ].id = 0;
				Grid[CaseI][CaseJ].State = SDL_GetTicks() + 2000;
			}
		}
		cptdebug--;
	}
}

void BuildShip() {
	Ress.Treecut++;
	int Built = 0;
	while (Ress.Trees > 0 && Ress.Fish<30 && Built<1000) {
		int Rcase = rand() % ((LMAP - 2) * 2); //cases au centre de la mer
		int CaseJ = 28 + Rcase / (LMAP - 2);
		int CaseI = 1 + Rcase % (LMAP - 2);
		CaseJ = MIN(CaseJ, HMAP - 1);
		CaseI = MIN(CaseI, LMAP - 1);
		if (Grid[CaseI][CaseJ].Object == SEA) {
			Built = 1000;
			Grid[CaseI][CaseJ].Object = SHIP;
			Grid[CaseI][CaseJ].State = SDL_GetTicks() + 2000;
		}
		Built++ ;
	}
}

void BuildMill() {
	Grid[13][20].Object = MILL; 
	Ress.Treecut++;
	Grid[13][20].State = SDL_GetTicks() + 2000;
}

void BuildFields() {
	//harvest
	while (Ress.Food + 10 > Ress.Harvest * 10) { //+10 de la peche
		//cptHunt--;
		Ress.Harvest++;
		Ress.Treecut++;
		if (era == CONTEMPORARY) {
			for (int i = 28;i > 0;i--) {
				for (int j = 10;j < 24;j++) {
					if (Grid[i][j].Object == EMPTY_CASE) {
						if (Grid[i + 1][j].Object == RIVER || Grid[i - 1][j].Object == RIVER || Grid[i][j - 1].Object == RIVER || Grid[i][j + 1].Object == RIVER) {
							Grid[i][j].Object = FIELD;
							Grid[i][j].State = SDL_GetTicks() + 2000;
							i = 0;
							j = 30;
						}
						else if (Grid[i + 1][j].Object == FIELD || Grid[i - 1][j].Object == FIELD || Grid[i][j - 1].Object == FIELD || Grid[i][j + 1].Object == FIELD) {
							Grid[i][j].Object = FIELD;
							Grid[i][j].State = SDL_GetTicks() + 2000;
							i = 0;
							j = 30;
						}
					}
				}
			}
		}
		for (int i = 14;i > 0;i--) {
			for (int j = 18;j < 24;j++) {
				if (Grid[i][j].Object == EMPTY_CASE) {
					if (Grid[i + 1][j].Object == FIELD || Grid[i - 1][j].Object == FIELD || Grid[i][j - 1].Object == FIELD || Grid[i][j + 1].Object == FIELD) {
						Grid[i][j].Object = FIELD;
						Grid[i][j].State = SDL_GetTicks() + 2000;
						i = 0;
						j = 30;
					}
				}
			}
		}
	}
}

void BuildHospi() {
	int found = 0;
	int cptdebug = 1000;
	while (!found && cptdebug > 0) {
		int Rcase = rand() % (LMAP*HMAP);
		int CaseI = Rcase % LMAP;
		int CaseJ = Rcase / LMAP;
		if (Grid[CaseI][CaseJ].Object == EMPTY_CASE || (Grid[CaseI][CaseJ].Object == FOREST && Grid[CaseI][CaseJ].State == 4)) {
			//check sides == house
			if (IsNearHouse(CaseI, CaseJ)) {
				found = 1;
				Grid[CaseI][CaseJ].Object = HOSPI;
				++hospitalCount;
				Grid[CaseI][CaseJ].State = SDL_GetTicks() + 2000;
			}
		}
		cptdebug--;
	}
}
void BuildFireStation() {
	int found = 0;
	int cptdebug = 1000;
	while (!found && cptdebug>0) {
		int Rcase = rand() % (LMAP*HMAP);
		int CaseI = Rcase % LMAP;
		int CaseJ = Rcase / LMAP;
		
		if (Grid[CaseI][CaseJ].Object == EMPTY_CASE || (Grid[CaseI][CaseJ].Object == FOREST && Grid[CaseI][CaseJ].State == 4)) {
			//check sides == house
			if (IsNearHouse(CaseI, CaseJ)) {
				found = 1;
				Grid[CaseI][CaseJ].Object = FIRESTATION;
				Grid[CaseI][CaseJ].State = SDL_GetTicks() + 2000;
			}
		}
		cptdebug--;
	}
}
void BuildBarrage() {
	Grid[15][15].Object = BARRAGE;
}
void BuildSecourist() {
	Grid[1][15].Object = SECOURIST;
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

	if (Ress.Pop > Ress.Huts + Ress.Houses*2 + Ress.Apparts*4) {
		BuildHut();
	}

	if (Ress.Pop >= ERAMED)
		era = MEDIEVAL;
}

void MedievalEra() {
	Ress.Pop += 2;
	int found = 0;
	Ress.Treecut = 2;
	Ress.Food = Ress.Pop; //total Ress.Food to search per turn
	Ress.Food -= 5;
	
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
			fishing--;
		}
		while (Ress.Food > 0 && Ress.River > 0) { /// Pour gérer sécheresse
			//Harvest
			Ress.Food -= (int)(Ress.Harvest * 10 * GetFieldProductivity());
		}
		while (Ress.Food > 0 && Ress.Animals > 0 && Revenge==0) {
			Hunt();
		}
		if (Revenge) {
			Revenge = 0;
			Ress.Food -= 10;
			Ress.Animals -= 3;
			Ress.Hunt += 2;
		}
		while (Ress.Food > 0) {
			//build new temporary Ship
			Ress.Food -= 10;
			BuildShip();
		}
	} //end Ress.Food

	if (Ress.Fish == 0 && tsunami==0) {
		BuildShip();
	}
	
	if (Ress.Pop > Ress.Huts + Ress.Houses*2 + Ress.Apparts*4) {
		BuildHouse();
	}
	RemoveRandomTrees();

	if (Ress.Pop >= ERACONT)
		era = CONTEMPORARY;
}

void ContemporaryEra() {

	static int initEra = 0;
	if (initEra==0) {
		BuildFireStation();
		BuildBarrage();
		initEra++;
	}
	Ress.Pop += 4;
	int found = 0;
	Ress.Treecut = 4;
	if (Ress.Trees < 100)
		Ress.Treecut -= 4;//collective consciousness
	Ress.Food = Ress.Pop; //total Ress.Food to search per turn
	Ress.Food -= 5;
	if (Ress.River) { //works while flood
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
			fishing--;
		}
		while (Ress.Food > 0 && Ress.River > 0) { /// Pour gérer sécheresse
			//Harvest
			Ress.Food -= (int)(Ress.Harvest * 10 * GetFieldProductivity());
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
	if (Ress.Fish == 0 && tsunami == 0) {
		BuildShip();
	}


	if (Ress.Pop > Ress.Huts + Ress.Houses*2 + Ress.Apparts*4) {
		BuildAppart();
	}
	RemoveRandomTrees();
}

void NoAction() {}

void Plant() {
	if (fire > 0) {
		SetAsAction(NO_ACTION);
		return;
	}
	int nbTreesAdded = 2+5*era;
	int Rtree = rand() % (FOREST_H * FOREST_W + 1); //random tree
	int i = Rtree % FOREST_W; //10col for 10line
	int j = Rtree / FOREST_W;
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
	if (IsType(i, j, HUT)) {
		Grid[i][j].Object = EMPTY_CASE;
		Ress.Pop -= 1;
	}
	else if (IsType(i, j, HOUSE)) {
		Grid[i][j].Object = EMPTY_CASE;
		Ress.Pop -= 2;
	}
	else if (IsType(i, j, APPART)) {
		Grid[i][j].Object = EMPTY_CASE;
		Ress.Pop -= 4;
	}
	else if (IsType(i, j, MILL)) {
		Grid[i][j].Object = EMPTY_CASE;
	}
	else if (IsType(i, j, HOSPI)) {
		Grid[i][j].Object = EMPTY_CASE;
		--hospitalCount;
	}
}

void Fire() {
	int found = 0;
	int cptdebug = 1000;
	if (fire == 0) {
		//first fire
		while (Ress.Trees > 0 && found == 0 && cptdebug > 0) {
			int Rtree = rand() % (FOREST_H * FOREST_W + 1); //fire random tree
			int i = Rtree % FOREST_W; //10col for 10line
			int j = Rtree / FOREST_W;
			if (Grid[COL_FOREST + i][LINE_FOREST + j].Object == FOREST && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
				Ress.Trees -= 4 - Grid[COL_FOREST + i][LINE_FOREST + j].State ;

				Grid[COL_FOREST + i][LINE_FOREST + j].State += 5;
				fire++;
				found = 1;
			}
			cptdebug--;
		}
	}
	else {
		int cptfire = fire + 30; //fire exponential
		//if (era == CONTEMPORARY) //FireStation -50% effect
		//	cptfire = cptfire/2;
		while (Ress.Trees > 0 && cptfire > 0) {
			int Rtree = rand() % (FOREST_H * FOREST_W + 1); //fire random tree
			int i = Rtree % FOREST_W; //10col for 10line
			int j = Rtree / FOREST_W;
			if (Grid[COL_FOREST + i][LINE_FOREST + j].Object == FOREST && Grid[COL_FOREST + i][LINE_FOREST + j].State < 4) {
				if (Grid[COL_FOREST + i+1][LINE_FOREST + j].State > 4 || Grid[COL_FOREST + i - 1][LINE_FOREST + j].State > 4 || Grid[COL_FOREST + i][LINE_FOREST + j+1].State > 4 || Grid[COL_FOREST +i][LINE_FOREST + j-1].State > 4) {
					Ress.Trees -= 4 - Grid[COL_FOREST + i][LINE_FOREST + j].State;
					Grid[COL_FOREST + i][LINE_FOREST + j].State += 5;
					fire++;
					break;
				}
			}
			cptfire--;
		}
		if (cptfire == 0) {
			//Fire isolated extinguished
			for (int i = COL_FOREST; i < COL_FOREST + FOREST_W; i++) {
				for (int j = LINE_FOREST; j < LINE_FOREST + FOREST_H; j++) {
					if (IsType(i, j, FOREST) && Grid[i][j].State > 4) {
						if ((era == CONTEMPORARY && Grid[i + 1][j].State <= 4 && Grid[i - 1][j].State <= 4 && Grid[i][j + 1].State <= 4 && Grid[i][j - 1].State <= 4)
							|| (era != CONTEMPORARY && Grid[i + 1][j].State == 4 && Grid[i - 1][j].State == 4 && Grid[i][j + 1].State == 4 && Grid[i][j - 1].State == 4)) {
							//extinguished fire
							fire--;
							Grid[i][j].State = 4;
							i = LMAP;
							j = LMAP;
						}
					}
				}
			}
		}
	}
}

void Avalanche() {
	avalanche = 3;
	for (int i = 0;i < LMAP;i++) {
		if (Grid[i][3].Object != EMPTY_CASE && Grid[i][3].Object != RIVER) {
			if (Grid[i][3].Object == FOREST) {
				if (Grid[i][3].State < 4) 
					Ress.Trees -= 4 - Grid[i][3].State;
				Grid[i][3].State = 4;
			}
			else
				Grid[i][3].Object = EMPTY_CASE;
		}
	}
}
void Tsunami() {
	tsunami = 3;
	Ress.River = 1;
	riverDryness = 0;
	Ress.Pop -= Ress.Fish;
	Ress.Fish = 0;
	for (int i = 0;i < LMAP;i++) {
		for (int j = 24;j < HMAP;j++) {
			if (Grid[i][j].Object != EMPTY_CASE && Grid[i][j].Object != RIVER && Grid[i][j].Object != SEA) {
				if (Grid[i][j].Object == FOREST) {
					if (Grid[i][j].State < 4)
						Ress.Trees -= 4 - Grid[i][j].State;
					Grid[i][j].State = 4;
				}
				else
					Grid[i][j].Object = EMPTY_CASE;
			}
		}
	}
}

void Rain() {
	if (rain == 0) {
		if (riverDryness < 10 && era != CONTEMPORARY) { // Flood
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
	int CaseI = meteor % LMAP;
	int CaseJ = meteor / LMAP;
	if (Grid[CaseI][CaseJ].Object == FOREST && Grid[CaseI][CaseJ].State < 4) {
		Ress.Trees -= 4 - Grid[CaseI][CaseJ].State;
		Grid[CaseI][CaseJ].State += 5;
		fire++;
	}
	for (int i = CaseI - 1;i < CaseI + 2;i++) {
		if (Grid[i][CaseJ].Object == HUT || Grid[i][CaseJ].Object == HOUSE || Grid[i][CaseJ].Object == APPART || Grid[i][CaseJ].Object == FIELD) {
			Grid[i][CaseJ].State = -int(SDL_GetTicks()+2000);
		}	
	}
	for (int j = CaseJ - 1;j < CaseJ + 2;j++) {
		if (Grid[CaseI][j].Object == HUT || Grid[CaseI][j].Object == HOUSE || Grid[CaseI][j].Object == APPART || Grid[CaseI][j].Object == FIELD) {
			Grid[CaseI][j].State = -int(SDL_GetTicks()+2000);
		}	
	}
	meteor = rand() % (LMAP * HMAP);

}

void Devour() {
	if (Ress.Animals > 0) {
		Ress.Pop-=1;
		if (era == MEDIEVAL) {
			//Revenge //battue au prochain tour
			Revenge = 1;
		}
	}
}

void Drown() {
	// @TODO: commencer à une case random pour être moins linéaire ?
	if (Ress.Fish) {
		for (int i = 0; i < LMAP; ++i) {
			for (int j = 27; j < HMAP; ++j) {
				if (Grid[i][j].Object == SHIP) {
					Grid[i][j].Object = SEA;
					--Ress.Pop;
					Ress.Fish--;
					return;
				}
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
	if (action == METEOR) {
		meteor = rand() % (LMAP * HMAP);
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

const char* GetPeriodName() {
	static const char* names[] = { "Printemps", "Été", "Automne", "Hiver" };
	return names[(Year / YEARS_PER_SEASON) % 4];
}
const char* GetEraName() {
	static const char* names[] = { "Tribal", "Medieval", "Contemporary"};
	return names[era];
}

void initGame() {
	int i = 0, j = 0;
	lastAction = PLANT;
	Ress.River = 1;
	Ress.Harvest = 0;
	Ress.Fish = 0;
	Ress.Animals = 15;
	Ress.Pop = 1;
	Ress.Trees = 0;
	triggerCold = 0;
	Ress.Huts = 1;
	Ress.Houses = 0;
	Ress.Apparts = 0;
	rain = 0;
	fire = 0;
	era = TRIBAL;
	SDL_Surface* HitboxRiverS = IMG_Load("Assets/Map/MapHitbox.png");

	for (i = 0; i < LMAP; i++) { //Init Grille //init map
		for (j = 0; j < HMAP; j++) {
			Grid[i][j].Object = EMPTY_CASE;
			Grid[i][j].State = 0;
			if (j < 3)
				Grid[i][j].Object = MOUNTAIN;
			else if (IsSeaLocation(i, j))
				Grid[i][j].Object = SEA;
			else if (IsRiverLocation(HitboxRiverS, i, j))
				Grid[i][j].Object = RIVER;
			else if (initForestLocation(i, j)) {
				Grid[i][j].Object = FOREST;
				Grid[i][j].id = rand() % 4;
				if (Grid[i][j].id == 1)
					Grid[i][j].id += rand() % 3;
			}
		}
	}
	SDL_FreeSurface(HitboxRiverS);
	Grid[5][10].Object = HUT;
	Grid[5][10].id = rand() % 4;
	timeTurn = TIMETURN;
}