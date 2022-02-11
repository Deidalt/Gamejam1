
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <math.h>

#include "Main.h"
#include "Fonctions.h"
#include "Pixel.h"


#define OMAPY 1446
#define DECALAGE 0 //Manque un bout à gauche de la map donc on décale tout

struct sTree {
    int init = 0;
    SDL_Point posArb[4]; 
};

static inline void DisplayTexts(TTF_Font* ArialNarrowB77);
static inline void DisplayTexts2(TTF_Font* ArialNarrowB77);
int TTFrender(const char* chaine, TTF_Font* ft, SDL_Color color, SDL_Point posft, bool solid=false);

void TTFprerender(const char* chaine, TTF_Font* ft, SDL_Color color, SDL_Texture** texture, int* w, int* h) {
    SDL_Surface* HudRessS = TTF_RenderText_Blended(ft, chaine, color);
    *texture = SDL_CreateTextureFromSurface(Renderer, HudRessS);
    *w = HudRessS->w, *h = HudRessS->h;
    SDL_FreeSurface(HudRessS);
}

void TTFRenderWithTextBefore(SDL_Texture* prerender, int w, int h, const char* chaine, TTF_Font* ft, SDL_Color color, SDL_Point posft) {
    SDL_Rect posT = { posft.x, posft.y, w, h };

    SDL_RenderCopy(Renderer, prerender, NULL, &posT);

    posft.x += w;
    TTFrender(chaine, ft, color, posft, true);
}

int TTFrender(const char *chaine, TTF_Font *ft, SDL_Color color, SDL_Point posft, bool solid) {
    //Text rendering
    //can write one line
    SDL_Surface* HudRessS;
    if (!solid) {
        HudRessS = TTF_RenderText_Blended(ft, chaine, color);
    }
    else {
        HudRessS = TTF_RenderText_Solid(ft, chaine, color);
    }
    SDL_Texture* HudRessT = SDL_CreateTextureFromSurface(Renderer, HudRessS);
    SDL_Rect posT = { posft.x,posft.y,HudRessS->w,HudRessS->h };

    SDL_RenderCopy(Renderer, HudRessT, NULL, &posT);
    SDL_FreeSurface(HudRessS);
    SDL_DestroyTexture(HudRessT);

    return posft.x + HudRessS->w;
}

void InitAffichage() {
    //Libs init
    Screen = SDL_CreateWindow("Hello", 0, 0, 640, 480, SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED); //SDL_WINDOW_FULLSCREEN_DESKTOP //
    if (Screen == NULL)
        printf("NULL Window\n");
    Renderer = SDL_CreateRenderer(Screen, -1, SDL_RENDERER_ACCELERATED); //SDL_RENDERER_PRESENTVSYNC //SDL_RENDERER_TARGETTEXTURE
    if (Renderer == NULL)
        printf("NULL renderer\n");
    if (TTF_Init() == -1)
        printf("TTF %s\n", TTF_GetError());

    SDL_Surface* IconeExe = IMG_Load_RW(SDL_RWFromFile("Assets/UI/Logos/Int0.png", "rb"), 1);

    if (IconeExe == NULL)
        printf("ICONULL %s\n", SDL_GetError());

    SDL_SetWindowIcon(Screen, IconeExe);
    SDL_FreeSurface(IconeExe);
    //SDL_RenderSetLogicalSize(Renderer,Wecran/Zoom4K,Hecran/Zoom4K); //resize tout l'écran

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best"); //utile pour les rotations de Sdl_renderCopyEx
    //SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "0");
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1"); //0 par defaut = pas de minimize
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    ScreenL.x = DM.w;
    ScreenL.y = DM.h;
    Zoom7K = ((float)ScreenL.y) / H6K;
    Zoom = ((float)ScreenL.y) / H4K;
    Zoom2K = ((float)ScreenL.y) / H2K;

    printf("Zoom %.2f\n", Zoom);
}

static inline const char* const ActionName() {
    static const char* names[NB_ACTIONS] = {
        "Plant", "Rain", "Cold", "Meteor", "Devour", "Drown", "Do nothing"
    };

    return names[getCurrentAction()];
}

enum { PRINTEMPS, ETE, HIVER, AUTOMNE = PRINTEMPS };

void Afficher() {
    SDL_FPoint CaseL = { LCASE + 0.7f,HCASE + 0.1f };
    SDL_SetRenderDrawColor(Renderer, 16, 8, 21, 255);
    SDL_RenderClear(Renderer);
    static int Initialised = 0;
    static int timerVictory = 0;

    int i = 0, j = 0; //loop
    int wText = 0, hText = 0, wText2, hText2; //dimensions des textures récupérées
    char buff1[100]; 
    char buff2[50] = {};
    static sTree posTree[30][30];
    static int cptFire = 0;
    static unsigned int timerFire = SDL_GetTicks() + 83;

    static TTF_Font* ArialNarrowB77 = TTF_OpenFont("ttf/Arial-Narrow-Bold.ttf", 77*Zoom);
    static TTF_Font* ArialNarrowB200 = TTF_OpenFont("ttf/Arial-Narrow-Bold.ttf", 160*Zoom);
    //Textures init
    static SDL_Texture* MapBase1T[3];
    static SDL_Texture* MapBase2T[3];
    static SDL_Texture* MapRe1T[3];
    static SDL_Texture* MapRe2T[3];
    static SDL_Texture* BackgroundT = IMG_LoadTexture(Renderer, "Assets/Map/Background_tempere.png");
    static SDL_Texture* LogoTreeT = IMG_LoadTexture(Renderer, "Assets/Tiles/Trees/LogoTree.png");
    static SDL_Texture* TreeAT[4];
    static SDL_Texture* TreeBT[4];
    static SDL_Texture* TreeCT[4];
    static SDL_Texture* TreeDT[4];
    static SDL_Texture* HouseAT[4];
    static SDL_Texture* HouseBT[4];
    static SDL_Texture* HouseCT[4];
    static SDL_Texture* MillT;
    static SDL_Texture* FieldT[3];
    static SDL_Texture* ShipT;
    static SDL_Texture* HospitalT;
    static SDL_Texture* FireStationT;
    static SDL_Texture* RiverT[3];
    static SDL_Texture* RainT[7];
    static SDL_Texture* SnowT[80];
    static SDL_Texture* FireT[6];
    static SDL_Texture* BuildingT;
    static SDL_Texture* BlackBgT;
    static SDL_Texture* BlueFilterT;
    static SDL_Texture* FrameT[4];
    static SDL_Texture* SpeedT[2];
    static SDL_Texture* InteractT[8];
    static SDL_Texture* AppleT;
    static SDL_Texture* PopT;
    static SDL_Texture* AnimalsT;
    static SDL_Texture* MeteorT = IMG_LoadTexture(Renderer, "Assets/Fx/meteorite.png");
    static SDL_Texture* AvalancheT = IMG_LoadTexture(Renderer, "Assets/Fx/avalanche.png");
    static SDL_Texture* TsunamiT = IMG_LoadTexture(Renderer, "Assets/Fx/tsunami.png");

    
    static SDL_Texture* RestartT;
    static SDL_Texture* ResumeT;
    static SDL_Texture* LeaveT;
    static SDL_Texture* WinT;
    static SDL_Texture* LoseT;
    static SDL_Texture* EraT[3];

    if (Initialised == 0) {
        //Vars init
        Initialised = 1;
        for (i = 0;i < 4;i++) {
#pragma warning(suppress : 4996)
            sprintf(buff1, "Assets/Tiles/Trees/tree_%d_tempere.png", i + 1);
            TreeAT[i] = IMG_LoadTexture(Renderer, buff1);
#pragma warning(suppress : 4996)
            sprintf(buff1, "Assets/Tiles/Trees/tree_%d_snowy.png", i + 1);
            TreeBT[i] = IMG_LoadTexture(Renderer, buff1);
#pragma warning(suppress : 4996)
            sprintf(buff1, "Assets/Tiles/Trees/tree_%d_glacial.png", i + 1);
            TreeCT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/Trees/tree_%d_secheresse.png", i + 1);
            TreeDT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/House/tribal_house_%d.png", i + 1);
            HouseAT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/House/medieval_house_%d.png", i + 1);
            HouseBT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/House/contemporary_house_%d.png", i + 1);
            HouseCT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/UI/Frame%d.png", i);
            FrameT[i] = IMG_LoadTexture(Renderer, buff1);

        }

        for (i = 0; i < 3;i++) {
            sprintf(buff1, "Assets/Map/Relief%c.png", 'A' + i);
            MapRe1T[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Map/MAP_base%c.png", 'A' + i);
            MapBase1T[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/River/river%d.png", i);
            RiverT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 8;i++) {
            if (i < 7) {
                sprintf(buff1, "Assets/Fx/Pluie/Pluie_%05d.png", i);
                RainT[i] = IMG_LoadTexture(Renderer, buff1);
            }
            sprintf(buff1, "Assets/UI/Logos/Int%d.png", i);
            InteractT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 80;i++) {
            sprintf(buff1, "Assets/Fx//Neige/Neige 2_%05d.png", i);
            SnowT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 6;i++) {
            sprintf(buff1, "Assets/Fx/Feu/Feu_1_%05d.png", i);
            FireT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 3;i++) {
            sprintf(buff1, "Assets/Tiles/Other/Field%d.png", i);
            FieldT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 2;i++) {
            sprintf(buff1, "Assets/UI/Speedup%d.png", i);
            SpeedT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        sprintf(buff1, "Assets/Tiles/Other/Ship.png");
        ShipT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/Tiles/Other/FireStation.png");
        FireStationT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/Tiles/Other/Hospital.png");
        HospitalT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/Tiles/construction.png");
        BuildingT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/UI/FondNoir.png");
        BlackBgT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/UI/BlueFilter.png");
        BlueFilterT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/Tiles/Other/moulin.png");
        MillT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/UI/Logos/apple.png");
        AppleT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/UI/Logos/population.png");
        PopT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/UI/Logos/animals.png");
        AnimalsT = IMG_LoadTexture(Renderer, buff1);

        //Texts

        SDL_Surface* RestartS;
        sprintf(buff1, "RESTART");
        RestartS = TTF_RenderText_Blended(ArialNarrowB200, buff1, { 255,255,255 });
        RestartT = SDL_CreateTextureFromSurface(Renderer, RestartS);
        SDL_FreeSurface(RestartS);
        SDL_Surface* ReturnS;
        sprintf(buff1, "RESUME");
        ReturnS = TTF_RenderText_Blended(ArialNarrowB200, buff1, { 255,255,255 });
        ResumeT = SDL_CreateTextureFromSurface(Renderer, ReturnS);
        SDL_FreeSurface(ReturnS);
        SDL_Surface* LeaveS;
        sprintf(buff1, "LEAVE");
        LeaveS = TTF_RenderText_Blended(ArialNarrowB200, buff1, { 255,255,255 });
        LeaveT = SDL_CreateTextureFromSurface(Renderer, LeaveS);
        SDL_FreeSurface(LeaveS);
        sprintf(buff1, "WIN");
        SDL_Surface* WinS = TTF_RenderText_Blended(ArialNarrowB200, buff1, { 255,255,255 });
        WinT = SDL_CreateTextureFromSurface(Renderer, WinS);
        SDL_FreeSurface(WinS);
        sprintf(buff1, "LOSE");
        SDL_Surface* LoseS = TTF_RenderText_Blended(ArialNarrowB200, buff1, { 255,255,255 });
        LoseT = SDL_CreateTextureFromSurface(Renderer, LoseS);
        SDL_FreeSurface(LoseS);
        sprintf(buff1, "Tribal era");
        SDL_Surface* EraS = TTF_RenderText_Blended(ArialNarrowB77, buff1, { 200,200,200 });
        EraT[0] = SDL_CreateTextureFromSurface(Renderer, EraS);
        SDL_FreeSurface(EraS);
        sprintf(buff1, "Medieval era");
        EraS = TTF_RenderText_Blended(ArialNarrowB77, buff1, { 200,200,200 });
        EraT[1] = SDL_CreateTextureFromSurface(Renderer, EraS);
        SDL_FreeSurface(EraS);
        sprintf(buff1, "Contemporary era");
        EraS = TTF_RenderText_Blended(ArialNarrowB77, buff1, { 200,200,200 });
        EraT[2] = SDL_CreateTextureFromSurface(Renderer, EraS);
        SDL_FreeSurface(EraS);

        Year = 1; //Game Starts here
        timegame = SDL_GetTicks();

    }



    int period = PRINTEMPS;
    if (IsDrySeason()) {
        period = ETE;
    }
    else if (IsGlacialSeason()) {
        period = HIVER;
    }

    //Background and base Map
    QueryText4(BackgroundT, &wText, &hText);
    QueryText4(MapBase1T[period], &wText2, &hText2);
    //SDL_Rect posBackground = { -posxy.x + wText2-wText/2,-posxy.y+hText2/2-hText/2,wText,hText };
    SDL_Rect posBackground = { 0,0,wText,hText };
    SDL_RenderCopy(Renderer, BackgroundT, NULL, &posBackground);
    SDL_Rect posMap = { -posxy.x,-posxy.y,wText2,hText2 };
    SDL_RenderCopy(Renderer, MapBase1T[period], NULL, &posMap);
    /*QueryText(MapBase2T[period], &wText2, &hText2);
    SDL_Rect posMap2 = { posMap.x+posMap.w,posMap.y,wText2,hText2 };
    SDL_RenderCopy(Renderer, MapBase2T[period], NULL, &posMap2);*/
    QueryText4(MapRe1T[period], &wText, &hText);
    SDL_Rect posMapRe1 = { posMap.x, posMap.y - hText + posMap.h, wText,hText };
    SDL_RenderCopy(Renderer, MapRe1T[period], NULL, &posMapRe1);
    /*QueryText(MapRe2T[period], &wText, &hText);
    SDL_Rect posMapRe2 = { arrond(posMap.x+4895*Zoom7K),arrond(posMap.y - hText + posMap.h),wText,hText };
    SDL_RenderCopy(Renderer, MapRe2T[period], NULL, &posMapRe2);*/
    QueryText4(RiverT[period], &wText, &hText);
    SDL_Rect posRiver = { posMap.x, posMap.y + posMap.h - hText, wText, hText };
    if (Ress.River) {
        SDL_RenderCopy(Renderer, RiverT[Ress.River - 1], NULL, &posRiver);
    }

    if (1) { //refaire
        //Render Tiles one by one
        //Tiles
        //SDL_Texture* CaseT = IMG_LoadTexture(Renderer, "Images/Case.png"); //delete test
        //i et j sont inversés à l'affichage à cause de l'isometrie
        //reverse i and j for isometric
        //FX2
        if (avalanche) {
            QueryText4(AvalancheT, &wText, &hText);
            SDL_Rect posAvalanche = { posMap.x, posMap.y - hText + posMap.h, wText,hText };
            SDL_RenderCopy(Renderer, AvalancheT, NULL, &posAvalanche);
        }
        if (tsunami) {
            QueryText4(TsunamiT, &wText, &hText);
            SDL_Rect posTsunami = { posMap.x, posMap.y - hText + posMap.h, wText,hText };
            SDL_RenderCopy(Renderer, TsunamiT, NULL, &posTsunami);
        }
        for (i = 0;i < LMAP;i++) {
            for (j = 0;j < HMAP;j++) {
                //QueryText(CaseT, &wText, &hText);
                SDL_Point ObjectP = { i * wText / 2  ,j * hText };
                SDL_Point ObjectIsoP = ToIso(ObjectP);
                SDL_Rect posObject = { ObjectIsoP.x - posxy.x - arrond(300 * Zoom),ObjectIsoP.y - posxy.y,wText,hText };
                if (Grid[j][i].Object == MOUNTAIN) {
                }
                else if (Grid[j][i].Object == RIVER) {
                }
                else if (Grid[j][i].Object == SEA) {
                }
                else if (Grid[j][i].Object == FOREST) {
                    if (Grid[j][i].State == 4) {
                        //SDL_SetTextureColorMod(CaseT, 100, 150, 100);
                    }
                    else {
                        //SDL_SetTextureColorMod(CaseT, 100, 255, 100);
                    }
                    if (Grid[j][i].State > 4) { //fire
                        
                        if (timerFire < SDL_GetTicks()) {
                            cptFire++;
                            if (cptFire > 5)
                                cptFire = 0;
                            timerFire = SDL_GetTicks() - timerFire + SDL_GetTicks() + 83;
                            timerFire = SDL_GetTicks() + 83;
                        }

                        QueryText4(FireT[cptFire], &wText, &hText);
                        SDL_Rect posFire = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                        SDL_RenderCopy(Renderer, FireT[cptFire], NULL, &posFire);
                    }
                    int idtree = Grid[j][i].id;
                    if (Grid[j][i].State < 4) {
                        for (unsigned int arb = 0; arb < 4 - Grid[j][i].State % 5; arb++) {
                            QueryText(TreeAT[Grid[j][i].id], &wText, &hText);
                            //init positions of each tree randomly in his case
                            if (arb == 0) {
                                idtree = Grid[j - 1][i].id;
                            }
                            else if (arb == 1) {
                                idtree = Grid[j + 1][i].id;
                            }
                            else if (arb == 2) {
                                idtree = Grid[j][i + 1].id;
                            }
                            else if (arb == 3) {
                                idtree = Grid[j][i - 1].id;
                            }
                            if (posTree[j][i].init < 4) {
                                if (arb == 0) {
                                    posTree[j][i].posArb[arb].x = arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) + arrond(Zoom * CaseL.x);
                                    posTree[j][i].posArb[arb].y = arrond((OMAPY + (CaseL.y * (i + 0)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - hText - arrond(Zoom * CaseL.y / 2);
                                }
                                else if (arb == 1) {
                                    posTree[j][i].posArb[arb].x = arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) + arrond(Zoom * CaseL.x / 2);
                                    posTree[j][i].posArb[arb].y = arrond((OMAPY + (CaseL.y * (i + 0)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - hText;
                                }
                                else if (arb == 2) {
                                    posTree[j][i].posArb[arb].x = arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) + arrond(Zoom * CaseL.x * 1.5f);
                                    posTree[j][i].posArb[arb].y = arrond((OMAPY + (CaseL.y * (i + 0)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - hText;
                                }
                                else if (arb == 3) {
                                    posTree[j][i].posArb[arb].x = arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) + arrond(Zoom * CaseL.x);
                                    posTree[j][i].posArb[arb].y = arrond((OMAPY + (CaseL.y * (i + 0)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - hText + arrond(Zoom * CaseL.y / 2);
                                }
                                if (idtree == 0) {
                                    posTree[j][i].posArb[arb].x -= arrond((45 + rand() % 40) * Zoom7K);
                                    posTree[j][i].posArb[arb].y += arrond((55 + rand() % 40) * Zoom7K);
                                }
                                else if (idtree == 1) {
                                    posTree[j][i].posArb[arb].x -= arrond((140 + rand() % 40) * Zoom7K);
                                    posTree[j][i].posArb[arb].y += arrond((95 + rand() % 40) * Zoom7K);
                                }
                                else if (idtree == 2) {
                                    posTree[j][i].posArb[arb].x -= arrond((172 + rand() % 40) * Zoom7K);
                                    posTree[j][i].posArb[arb].y += arrond((50 + rand() % 40) * Zoom7K);
                                }
                                else if (idtree == 3) {
                                    posTree[j][i].posArb[arb].x -= arrond((110 + rand() % 40) * Zoom7K);
                                    posTree[j][i].posArb[arb].y += arrond((21 + rand() % 40) * Zoom7K);
                                }
                                posTree[j][i].init++;
                            }

                            SDL_Rect posFinal = { posTree[j][i].posArb[arb].x - posxy.x,posTree[j][i].posArb[arb].y - posxy.y,wText,hText };
                            if (period == ETE) {
                                SDL_RenderCopy(Renderer, TreeDT[idtree], NULL, &posFinal);
                            }
                            else if (triggerCold) {
                                SDL_RenderCopy(Renderer, TreeBT[idtree], NULL, &posFinal);
                            }
                            else if (period == HIVER) {
                                SDL_RenderCopy(Renderer, TreeCT[idtree], NULL, &posFinal);
                            }
                            else {
                                SDL_RenderCopy(Renderer, TreeAT[idtree], NULL, &posFinal);
                            }
                        }
                    }
                    else if (Grid[j][i].State > 4) {
                        for (unsigned int arb = 0; arb < 4 - Grid[j][i].State % 5; arb++) {
                            if (arb == 0) {
                                idtree = Grid[j - 1][i].id;
                            }
                            else if (arb == 1) {
                                idtree = Grid[j + 1][i].id;
                            }
                            else if (arb == 2) {
                                idtree = Grid[j][i + 1].id;
                            }
                            else if (arb == 3) {
                                idtree = Grid[j][i - 1].id;
                            }
                            QueryText(TreeAT[idtree], &wText, &hText);
                            SDL_Rect posFinal = { posTree[j][i].posArb[arb].x - posxy.x,posTree[j][i].posArb[arb].y - posxy.y,wText,hText };
                            if (period == ETE) {
                                SDL_SetTextureColorMod(TreeDT[idtree], 255, 10, 50);
                                SDL_RenderCopy(Renderer, TreeDT[idtree], NULL, &posFinal);
                                SDL_SetTextureColorMod(TreeDT[idtree], 255, 255, 255);
                            }
                            else if (triggerCold) {
                                SDL_SetTextureColorMod(TreeBT[idtree], 255, 10, 50);
                                SDL_RenderCopy(Renderer, TreeBT[idtree], NULL, &posFinal);
                                SDL_SetTextureColorMod(TreeBT[idtree], 255, 255, 255);
                            }
                            else if (period == HIVER) {
                                SDL_SetTextureColorMod(TreeCT[idtree], 255, 10, 50);
                                SDL_RenderCopy(Renderer, TreeCT[idtree], NULL, &posFinal);
                                SDL_SetTextureColorMod(TreeCT[idtree], 255, 255, 255);
                            }
                            else{
                                SDL_SetTextureColorMod(TreeAT[idtree], 255, 10, 50);
                                SDL_RenderCopy(Renderer, TreeAT[idtree], NULL, &posFinal);
                                SDL_SetTextureColorMod(TreeAT[idtree], 255, 255, 255);
                            }
                        }
                    }
                }
                else if (Grid[j][i].Object == HUT) {
                    if (Grid[j][i].State < 0) {
                        if (fabsf(Grid[j][i].State) > SDL_GetTicks()) {
                            //destroyed
                            if (timerFire < SDL_GetTicks()) {
                                cptFire++;
                                if (cptFire > 5)
                                    cptFire = 0;
                                timerFire = SDL_GetTicks() - timerFire + SDL_GetTicks() + 83;
                                timerFire = SDL_GetTicks() + 83;
                            }

                            QueryText4(FireT[cptFire], &wText, &hText);
                            SDL_Rect posFire = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, FireT[cptFire], NULL, &posFire);
                        }
                        else {
                            Grid[j][i].State = 0;
                            Grid[j][i].Object = EMPTY_CASE;
                            Ress.Pop -= 1;
                        }
                    }
                    else {
                        if (Grid[j][i].State < SDL_GetTicks())
                            Grid[j][i].State = 1;
                        QueryText(HouseAT[Grid[j][i].id], &wText, &hText);
                        SDL_Rect posHouseA = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                        SDL_RenderCopy(Renderer, HouseAT[Grid[j][i].id], NULL, &posHouseA);
                    }
                }
                else if (Grid[j][i].Object == HOUSE) {
                    if (Grid[j][i].State < 0) {
                        if (fabsf(Grid[j][i].State) > SDL_GetTicks()) {
                            //destroyed
                            if (timerFire < SDL_GetTicks()) {
                                cptFire++;
                                if (cptFire > 5)
                                    cptFire = 0;
                                timerFire = SDL_GetTicks() - timerFire + SDL_GetTicks() + 83;
                                timerFire = SDL_GetTicks() + 83;
                            }

                            QueryText4(FireT[cptFire], &wText, &hText);
                            SDL_Rect posFire = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, FireT[cptFire], NULL, &posFire);
                        }
                        else {
                            Grid[j][i].State = 0;
                            Grid[j][i].Object = EMPTY_CASE;
                            Ress.Pop -= 2;
                        }
                    }
                    else {
                        if (Grid[j][i].State > SDL_GetTicks()) {
                            //Building
                            QueryText4(BuildingT, &wText, &hText);
                            SDL_Rect posBuilding = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, BuildingT, NULL, &posBuilding);

                        }
                        else {
                            if (Grid[j][i].State < SDL_GetTicks())
                                Grid[j][i].State = 1;
                            //SDL_SetTextureColorMod(CaseT, 204, 0, 204);
                            QueryText4(HouseBT[Grid[j][i].id], &wText, &hText);
                            SDL_Rect posHouseB = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, HouseBT[Grid[j][i].id], NULL, &posHouseB);
                        }
                    }
                }
                else if (Grid[j][i].Object == APPART) {
                    if (Grid[j][i].State < 0) {
                        if (fabsf(Grid[j][i].State) > SDL_GetTicks()) {
                            //destroyed
                            if (timerFire < SDL_GetTicks()) {
                                cptFire++;
                                if (cptFire > 5)
                                    cptFire = 0;
                                timerFire = SDL_GetTicks() - timerFire + SDL_GetTicks() + 83;
                                timerFire = SDL_GetTicks() + 83;
                            }

                            QueryText4(FireT[cptFire], &wText, &hText);
                            SDL_Rect posFire = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, FireT[cptFire], NULL, &posFire);
                        }
                        else {
                            Grid[j][i].State = 0;
                            Grid[j][i].Object = EMPTY_CASE;
                            Ress.Pop -= 4;
                        }
                    }
                    else {
                        if (Grid[j][i].State > SDL_GetTicks()) {
                            //Building
                            QueryText4(BuildingT, &wText, &hText);
                            SDL_Rect posBuilding = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, BuildingT, NULL, &posBuilding);

                        }
                        else {
                            QueryText4(HouseCT[Grid[j][i].id], &wText, &hText);
                            SDL_Rect posHouseC = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, HouseCT[Grid[j][i].id], NULL, &posHouseC);
                        }
                    }
                }

                else if (Grid[j][i].Object == FIELD) {
                    if (i == 20 && j == 13) {
                        if (Grid[j][i].State > SDL_GetTicks()) {
                            //Building mill
                            QueryText4(BuildingT, &wText, &hText);
                            SDL_Rect posBuilding = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, BuildingT, NULL, &posBuilding);

                        }
                        else {
                            if (Grid[j][i].State < SDL_GetTicks())
                                Grid[j][i].State = 1;
                           // SDL_SetTextureColorMod(CaseT, 230, 153, 0);
                            QueryText4(MillT, &wText, &hText);
                            SDL_Rect posMill = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, MillT, NULL, &posMill);
                        }
                    }
                    else {
                        if (Grid[j][i].State < 0) {
                            if (fabsf(Grid[j][i].State) > SDL_GetTicks()) {
                                //destroyed
                                if (timerFire < SDL_GetTicks()) {
                                    cptFire++;
                                    if (cptFire > 5)
                                        cptFire = 0;
                                    timerFire = SDL_GetTicks() - timerFire + SDL_GetTicks() + 83;
                                    timerFire = SDL_GetTicks() + 83;
                                }

                                QueryText4(FireT[cptFire], &wText, &hText);
                                SDL_Rect posFire = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                                SDL_RenderCopy(Renderer, FireT[cptFire], NULL, &posFire);
                            }
                            else {
                                Grid[j][i].State = 0;
                                Grid[j][i].Object = EMPTY_CASE;
                                Ress.Pop -= 1;
                            }
                        }
                        else {
                            if (Grid[j][i].State > SDL_GetTicks()) {
                                //Building field
                                QueryText4(FieldT[0], &wText, &hText);
                                SDL_Rect posField = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                                SDL_RenderCopy(Renderer, FieldT[0], NULL, &posField);

                            }
                            else {
                                if (Grid[j][i].State < SDL_GetTicks())
                                    Grid[j][i].State = 1;
                                int idField = 1;
                                if (Ress.River == 0)
                                    idField = 2;
                                //SDL_SetTextureColorMod(CaseT, 255, 255, 153);
                                QueryText4(FieldT[idField], &wText, &hText);
                                SDL_Rect posField = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                                SDL_RenderCopy(Renderer, FieldT[idField], NULL, &posField);
                            }
                        }
                    }
                }
                else if (Grid[j][i].Object == SHIP) {
                    //SDL_SetTextureColorMod(CaseT, 255, 255, 255);
                    QueryText4(ShipT, &wText, &hText);
                    SDL_Rect posShip = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                    SDL_RenderCopy(Renderer, ShipT, NULL, &posShip);
                }
                else if (Grid[j][i].Object == HOSPI) {
                    QueryText4(HospitalT, &wText, &hText);
                    SDL_Rect posHospital = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                    SDL_RenderCopy(Renderer, HospitalT, NULL, &posHospital);
                }
                else if (Grid[j][i].Object == FIRESTATION) {
                    QueryText4(FireStationT, &wText, &hText);
                    SDL_Rect posFireStation = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                    SDL_RenderCopy(Renderer, FireStationT, NULL, &posFireStation);
                }
                else if (Grid[j][i].Object == BARRAGE) {
                    //SDL_SetTextureColorMod(CaseT, 200, 200, 200);
                }
                else if (Grid[j][i].Object == SECOURIST) {
                    //SDL_SetTextureColorMod(CaseT, 0, 0, 0);
                }
                else {
                    //SDL_SetTextureColorMod(CaseT, 100, 200, 100);
                }
                //SDL_RenderCopy(Renderer, CaseT, NULL, &posObject);

            }
        }
        //SDL_DestroyTexture(CaseT);
    }
    //FX
    if (rain) {
        static int cptRain = 0;
        static unsigned int timerRain = SDL_GetTicks() + 83;
        if (timerRain < SDL_GetTicks()) {
            cptRain++;
            if (cptRain > 6) 
                cptRain = 0;
            timerRain = SDL_GetTicks() - timerRain + SDL_GetTicks() + 83;
            timerRain = SDL_GetTicks() + 83;
        }
        QueryText4(RainT[0], &wText, &hText);
        SDL_Rect posMeteo = { 0,0,wText,hText };
        SDL_RenderCopy(Renderer, RainT[cptRain], NULL, &posMeteo);

    }
    else if (triggerCold) {
        static int cptSnow = 0;
        static unsigned int timerSnow = SDL_GetTicks() + 83;
        if (timerSnow < SDL_GetTicks()) {
            cptSnow++;
            if (cptSnow > 79)
                cptSnow = 0;
            timerSnow = SDL_GetTicks() - timerSnow + SDL_GetTicks() + 83;
            timerSnow = SDL_GetTicks() + 83;
        }
        QueryText2(SnowT[0], &wText, &hText);
        SDL_Rect posMeteo = { 0,0,wText,hText };
        SDL_RenderCopy(Renderer, SnowT[cptSnow], NULL, &posMeteo);
    }
    if (lastAction == METEOR) {
        int CDMeteor = (SDL_GetTicks() - timegame) % timeTurn ;
        QueryText4(MeteorT, &wText, &hText);
        SDL_Rect posMeteor = { arrond((CaseL.x * (LMAP - (meteor / HMAP)) + CaseL.x * (meteor % LMAP)) * Zoom - posxy.x - wText / 2),
            arrond((OMAPY + (CaseL.y * (meteor / HMAP + 1)) - (CaseL.y * (LMAP - meteor % LMAP - 1))) * Zoom) - posxy.y + (-timeTurn + CDMeteor) - hText,wText,hText };
        SDL_RenderCopy(Renderer, MeteorT, NULL, &posMeteor);
    }


    //HUD MENU & UPSCREEN
    if (Menu == ESCAPE) {
        SDL_Rect posNoir = { arrond(1000 * Zoom),arrond(450 * Zoom),arrond(ScreenL.x - 2000 * Zoom), arrond(ScreenL.y - 900 * Zoom) };
        SDL_RenderCopy(Renderer, BlackBgT, NULL, &posNoir);
        for (i = 0;i < (ScreenL.x - 2000 * Zoom) / 60 + 1;i++) {
            SDL_Rect posFrame = { posNoir.x + i * 60,posNoir.y,64,8 };
            if (posFrame.x + posFrame.w > arrond(ScreenL.x - 1000 * Zoom))
                posFrame.x = arrond(ScreenL.x - 1000 * Zoom) - 64;
            SDL_RenderCopy(Renderer, FrameT[0], NULL, &posFrame);
            posFrame.y = arrond(ScreenL.y - 450 * Zoom) - 8;
            SDL_RenderCopy(Renderer, FrameT[1], NULL, &posFrame);
        }
        for (i = 0;i <(ScreenL.y - 900 * Zoom) / 60 + 1;i++) {
            SDL_Rect posFrame = { posNoir.x,posNoir.y+i * 60,8,64 };
            if (posFrame.y + posFrame.h > arrond(ScreenL.y - 450 * Zoom))
                posFrame.y = arrond(ScreenL.y - 450 * Zoom) - 64;
            SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            posFrame.x = arrond(ScreenL.x - 1000 * Zoom) - 8;
            SDL_RenderCopy(Renderer, FrameT[3], NULL, &posFrame);
        }
        SDL_QueryTexture(ResumeT, NULL, NULL, &wText, &hText);
        SDL_Rect posResume = { arrond(ScreenL.x/2 - wText / 2), arrond(ScreenL.y/2 - hText/2 - 300*Zoom),wText,hText };
        SDL_RenderCopy(Renderer, ResumeT, NULL, &posResume);
        SDL_QueryTexture(RestartT, NULL, NULL, &wText, &hText);
        SDL_Rect posRestart = { arrond(ScreenL.x/2 - wText / 2), arrond(ScreenL.y/2 - hText/2 - 0*Zoom),wText,hText };
        SDL_RenderCopy(Renderer, RestartT, NULL, &posRestart);
        SDL_QueryTexture(LeaveT, NULL, NULL, &wText, &hText);
        SDL_Rect posLeave = { arrond(ScreenL.x/2 - wText / 2), arrond(ScreenL.y/2 - hText/2 +300*Zoom),wText,hText };
        SDL_RenderCopy(Renderer, LeaveT, NULL, &posLeave);
        QueryText2(SpeedT[0], &wText, &hText);
        SDL_Rect posSpeed = { arrond(1500 * Zoom), arrond(ScreenL.y / 2 - hText / 2 -300*Zoom + SousMenu*300*Zoom),wText,hText };
        SDL_RenderCopy(Renderer, SpeedT[0], NULL, &posSpeed);
        posSpeed.x += arrond(20 * Zoom);
        SDL_RenderCopy(Renderer, SpeedT[0], NULL, &posSpeed);
        posSpeed.x += arrond(70 * Zoom);

        if ((Ress.Trees <= 0 || Ress.Pop <= 0  || Year >= YEARMAX ) && timerVictory == 0) {
            timerVictory = SDL_GetTicks();
        }
        
        else if (SDL_GetTicks() - timerVictory < 5000) {
            wText = 100 * Zoom;
            hText = 100 * Zoom;
            SDL_Rect posFin = { 0,0,wText,hText };
            int nbFin = (SDL_GetTicks() - timerVictory) / 10;
            if (Year >= YEARMAX) {
                //win
                
                posFin.w = posFin.w + 1 * (nbFin/2);
                posFin.h = posFin.w + 1 * (nbFin/2);
                posFin.x = arrond(300*Zoom-posFin.w/2);
                posFin.y = ScreenL.y / 2 - posFin.h/2;
                SDL_RenderCopy(Renderer, InteractT[0], NULL, &posFin);
                if (Ress.Animals) {
                    posFin.x = ScreenL.x + posFin.x-arrond(600*Zoom) ;
                    SDL_RenderCopy(Renderer, AnimalsT, NULL, &posFin);
                }
                SDL_QueryTexture(WinT, NULL, NULL, &wText, &hText);
                SDL_SetTextureColorMod(WinT, 100, 200, 100);
                SDL_Rect posWin = { arrond(ScreenL.x / 2 - wText / 2), arrond(ScreenL.y / 2 - hText / 2 - 500 * Zoom),wText,hText };
                SDL_RenderCopy(Renderer, WinT, NULL, &posWin);
            }
            else {
                while (nbFin >= 0) {
                    posFin.x = rand() % ScreenL.x - wText;
                    posFin.y = rand() % ScreenL.y - hText;
                    if (Ress.Trees <= 0)
                        SDL_RenderCopy(Renderer, PopT, NULL, &posFin);
                    else
                        SDL_RenderCopy(Renderer, LogoTreeT, NULL, &posFin);
                    
                    nbFin--;
                }
            
                SDL_QueryTexture(LoseT, NULL, NULL, &wText, &hText);
                SDL_SetTextureColorMod(LoseT, 255, 100, 100);
                SDL_Rect posLose = { arrond(ScreenL.x / 2 - wText / 2), arrond(ScreenL.y / 2 - hText / 2 - 500 * Zoom),wText,hText };
                SDL_RenderCopy(Renderer, LoseT, NULL, &posLose);
            }
        }

    }
    else {
        timerVictory = 0;
        if (Menu == NONE) {
            SDL_Rect posNoir = { 0,0,ScreenL.x, arrond(100 * Menu * Zoom) };
            SDL_RenderCopy(Renderer, BlackBgT, NULL, &posNoir);
            for (i = 0;i < ScreenL.x / 60 + 1;i++) {
                SDL_Rect posFrame = { i * 60,0,64,8 };
                SDL_RenderCopy(Renderer, FrameT[0], NULL, &posFrame);
                posFrame.y = arrond(100 * Menu * Zoom) - 8;
                SDL_RenderCopy(Renderer, FrameT[1], NULL, &posFrame);
            }
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { 0,i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
                posFrame.x = ScreenL.x - 8;
                SDL_RenderCopy(Renderer, FrameT[3], NULL, &posFrame);
            }
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(800 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posApple = { arrond(1000 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, AppleT, NULL, &posApple);
            posApple.x += 100 * Zoom;
            if (Ress.River && era != TRIBAL)
                SDL_RenderCopy(Renderer, FieldT[1], NULL, &posApple);
            else if (Ress.Animals)
                SDL_RenderCopy(Renderer, AnimalsT, NULL, &posApple);
            else {
                posApple.w = arrond(192 * Zoom);
                posApple.h = arrond(122 * Zoom);
                posApple.x -= arrond(60 * Zoom);
                posApple.y -= arrond(10 * Zoom);
                SDL_RenderCopy(Renderer, ShipT, NULL, &posApple);
            }
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(1400 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posPop = { arrond(1600 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, PopT, NULL, &posPop);
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(2000 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posLogoTree = { arrond(2200 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, LogoTreeT, NULL, &posLogoTree);
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(2600 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posAnimals = { arrond(2800 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, AnimalsT, NULL, &posAnimals);

            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(3200 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            DisplayTexts2(ArialNarrowB77);

        }
        else if (Menu == UIUP) {
            SDL_Rect posNoir = { 0,0,ScreenL.x,arrond(100 * Menu * Zoom) };
            SDL_RenderCopy(Renderer, BlackBgT, NULL, &posNoir);
            for (i = 0;i < ScreenL.x / 60 + 1;i++) {
                SDL_Rect posFrame = { i * 60,0,64,8 };
                SDL_RenderCopy(Renderer, FrameT[0], NULL, &posFrame);
                posFrame.y = arrond(100 * Menu * Zoom) - 8;
                SDL_RenderCopy(Renderer, FrameT[1], NULL, &posFrame);
            }
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { 0,i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
                posFrame.x = ScreenL.x - 8;
                SDL_RenderCopy(Renderer, FrameT[3], NULL, &posFrame);
            }

            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(800 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posApple = { arrond(1000 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, AppleT, NULL, &posApple);
            posApple.x += 100 * Zoom;
            if (Ress.River && era != TRIBAL)
                SDL_RenderCopy(Renderer, FieldT[1], NULL, &posApple);
            else if (Ress.Animals)
                SDL_RenderCopy(Renderer, AnimalsT, NULL, &posApple);
            else {
                posApple.w = arrond(192 * Zoom);
                posApple.h = arrond(122 * Zoom);
                posApple.x -= arrond(60 * Zoom);
                posApple.y -= arrond(10 * Zoom);
                SDL_RenderCopy(Renderer, ShipT, NULL, &posApple);
            }
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(1400 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posPop = { arrond(1600 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, PopT, NULL, &posPop);
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(2000 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posLogoTree = { arrond(2200 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, LogoTreeT, NULL, &posLogoTree);
            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(2600 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }
            SDL_Rect posAnimals = { arrond(2800 * Zoom), arrond(10 * Zoom),arrond(80 * Zoom),arrond(80 * Zoom) };
            SDL_RenderCopy(Renderer, AnimalsT, NULL, &posAnimals);

            for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
                SDL_Rect posFrame = { arrond(3200 * Zoom),i * 60,8,64 };
                if (posFrame.y + posFrame.h > arrond(100 * Menu * Zoom))
                    posFrame.y = arrond(100 * Menu * Zoom) - 64;
                SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            }


            DisplayTexts(ArialNarrowB77);
            DisplayTexts2(ArialNarrowB77);

        }
        QueryText2(SpeedT[0], &wText, &hText);
        SDL_Rect posSpeed = { arrond(20 * Zoom),arrond(20 * Zoom),wText,hText };
        SDL_RenderCopy(Renderer, SpeedT[1], NULL, &posSpeed);
        posSpeed.x += arrond(20 * Zoom);
        SDL_RenderCopy(Renderer, SpeedT[1], NULL, &posSpeed);
        posSpeed.x += arrond(70 * Zoom);
        sprintf(buff1, "x%d", 2000 / timeTurn);
        SDL_Point posXtime = { posSpeed.x, arrond(8 * Zoom) };
        TTFrender(buff1, ArialNarrowB77, { 150, 150, 150 }, posXtime);
        posSpeed.x += arrond(80 * Zoom);
        SDL_RenderCopy(Renderer, SpeedT[0], NULL, &posSpeed);
        posSpeed.x += arrond(20 * Zoom);
        SDL_RenderCopy(Renderer, SpeedT[0], NULL, &posSpeed);
        sprintf(buff1, "Year %d", Year);
        SDL_Point posYear = { arrond(400 * Zoom), arrond(5 * Zoom) };
        TTFrender(buff1, ArialNarrowB77, { 200, 200, 200 }, posYear);
        SDL_QueryTexture(EraT[era], NULL, NULL, &wText, &hText);
        SDL_Rect posEra = { arrond(3520 * Zoom - wText/2), arrond(5 * Zoom),wText,hText };
        SDL_RenderCopy(Renderer, EraT[era], NULL, &posEra);

        for (j = 0; j < 8;j++) {
            float CDaction = ((SDL_GetTicks() - timegame) % timeTurn) / float(timeTurn);
            if (j == 1) {
                if (period == ETE) {
                    CDaction = float(Year % YEARS_PER_SEASON) / YEARS_PER_SEASON;
                }
                else if (rain)
                    CDaction = (CDaction + rain - 1) / 7.0f;
                else
                    CDaction = (CDaction + rain) / 7.0f;
            }
            else if (j == 2) {
                CDaction = float((Year % (YEARS_PER_SEASON * 4)) % (YEARS_PER_SEASON * 3)) / (YEARS_PER_SEASON * 3);
                if (IsColdOn())
                    CDaction = 0;
            }
            else if (j == 3 && era == 0)
                CDaction = float(Ress.Pop) / ERAMED;
            else if (j == 0 && fire > 0)
                CDaction = 0;

            if (CDaction < 0)
                CDaction = 0;
            //Actions
            SDL_Rect posNoir = { arrond(200 * Zoom + j * 220 * Zoom),arrond(1900 * Zoom),arrond(200 * Zoom),arrond(200 * Zoom) };
            SDL_RenderCopy(Renderer, BlackBgT, NULL, &posNoir);
            if (j != 7 || (Mix_PausedMusic() == 0))
             SDL_RenderCopy(Renderer, InteractT[j], NULL, &posNoir);
            if (getCurrentAction() == j) {
                SDL_SetTextureColorMod(FrameT[0], 0, 0, 0);
                SDL_SetTextureColorMod(FrameT[1], 0, 0, 255);
                SDL_SetTextureColorMod(FrameT[2], 255, 0, 0);
                SDL_SetTextureColorMod(FrameT[3], 0, 255, 0);
            }
            SDL_Rect posFrame = { arrond(200 * Zoom + j * 220 * Zoom),arrond(1900 * Zoom),arrond(200 * Zoom),8 };
            SDL_RenderCopy(Renderer, FrameT[0], NULL, &posFrame);
            posFrame.y += arrond(200 * Zoom) - 8;
            SDL_RenderCopy(Renderer, FrameT[1], NULL, &posFrame);

            SDL_Rect posFrame2 = { arrond(200 * Zoom + j * 220 * Zoom),arrond(1900 * Zoom),8,arrond(200 * Zoom) };
            SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame2);
            posFrame2.x += arrond(200 * Zoom) - 8;
            SDL_RenderCopy(Renderer, FrameT[3], NULL, &posFrame2);
            if (getCurrentAction() == j) {
                SDL_SetTextureColorMod(FrameT[0], 255,255, 255);
                SDL_SetTextureColorMod(FrameT[1], 255,255, 255);
                SDL_SetTextureColorMod(FrameT[2], 255,255, 255);
                SDL_SetTextureColorMod(FrameT[3], 255,255, 255);
            }
            if (getCurrentAction() == j || (j == 1 && (rain > 0 || period == ETE)) || (j == 2 && (period != HIVER || IsColdOn())) || (j == 3 && era == 0) || (j==0 && fire>0)) {
                //CD
                SDL_Rect posBlue = { arrond(200 * Zoom + j * 220 * Zoom),arrond(1900 * Zoom),arrond(200 * Zoom),arrond((200 - 200 * CDaction) * Zoom) };
                posBlue.y += arrond(200 * Zoom - posBlue.h);
                SDL_RenderCopy(Renderer, BlueFilterT, NULL, &posBlue);
            }
            sprintf(buff1, "%d", j);
            SDL_Point posAction = { arrond(200 * Zoom + j * 220 * Zoom + posNoir.w / 2 - 18 * Zoom),arrond(1850 * Zoom) + posNoir.h };
            TTFrender(buff1, ArialNarrowB77, { 255, 250, 250 }, posAction);


        }
    }
    SDL_RenderPresent(Renderer);
}


static inline void DisplayTexts(TTF_Font* ArialNarrowB77) {
    char buff1[100];
    static SDL_Texture* startText[7] = { NULL };
    static int widths[7], heights[7];
    int i = 0;

    if (startText[0] == NULL) {
        TTFprerender("Gathering +", ArialNarrowB77, {150, 200, 150}, &startText[i], &widths[i], &heights[i]);
        ++i;
        TTFprerender("Hunt +", ArialNarrowB77, { 150, 200, 150 }, &startText[i], &widths[i], &heights[i]);
        ++i;
        TTFprerender("Fish +", ArialNarrowB77, { 150, 200, 150 }, &startText[i], &widths[i], &heights[i]);
        ++i;
        TTFprerender("Harvest +", ArialNarrowB77, { 150, 200, 150 }, &startText[i], &widths[i], &heights[i]);
        ++i;
        TTFprerender("Sick +", ArialNarrowB77, { 150, 200, 150 }, &startText[i], &widths[i], &heights[i]);
        ++i;
        TTFprerender("Bud +", ArialNarrowB77, { 150, 200, 150 }, &startText[i], &widths[i], &heights[i]);
        ++i;
        TTFprerender("Wood -", ArialNarrowB77, { 200, 150, 150 }, &startText[i], &widths[i], &heights[i]);
    }

    i = 0;
    //Food
    SDL_Point posRess = { arrond(910 * Zoom), arrond(10 * Zoom) };
    sprintf(buff1, "5");
    posRess.y += arrond(70 * Zoom);
    TTFRenderWithTextBefore(startText[i], widths[i], heights[i], buff1, ArialNarrowB77, { 150, 200, 150 }, posRess);
    ++i;
    
    if (Ress.Hunt) {
        sprintf(buff1, "%d", Ress.Hunt * 10);
        posRess.y += arrond(70 * Zoom);
        TTFRenderWithTextBefore(startText[i], widths[i], heights[i], buff1, ArialNarrowB77, { 150, 200, 150 }, posRess);
    }
    ++i;
    if (Ress.Fish) {
        sprintf(buff1, "%d", Ress.Fish * 10);
        posRess.y += arrond(70 * Zoom);
        TTFRenderWithTextBefore(startText[i], widths[i], heights[i], buff1, ArialNarrowB77, { 150, 200, 150 }, posRess);
    }
    ++i;
    if (Ress.Harvest && Ress.River) {
        sprintf(buff1, "%d", Ress.Harvest * 10);
        posRess.y += arrond(70 * Zoom);
        TTFRenderWithTextBefore(startText[i], widths[i], heights[i], buff1, ArialNarrowB77, { 150, 200, 150 }, posRess);
    }
    ++i;
    posRess.y = arrond(80 * Zoom);
    posRess.x += arrond(600 * Zoom);
    if (IsColdOn()) {
        sprintf(buff1, "%d", GetSickNumber());
        TTFRenderWithTextBefore(startText[i], widths[i], heights[i], buff1, ArialNarrowB77, { 150, 200, 150 }, posRess);
    }
    ++i;
    posRess.y = arrond(80 * Zoom);
    posRess.x += arrond(600 * Zoom);
    if (lastAction == PLANT || Ress.Trees < 100) {
        int cptplant = 0;
        if (Ress.Trees < 100)
            cptplant += 4;
        if (lastAction == PLANT)
            cptplant += 2 + 5 * era;
        sprintf(buff1, "%d", cptplant);
        TTFRenderWithTextBefore(startText[i], widths[i], heights[i], buff1, ArialNarrowB77, { 150, 200, 150 }, posRess);
        posRess.y += arrond(70 * Zoom);
    }
    ++i;
    if (Ress.Treecut) {
        int cptcut = Ress.Treecut;
        if (Ress.Trees < 100)
            cptcut += 4;
        sprintf(buff1, "%d", Ress.Treecut);
        TTFRenderWithTextBefore(startText[i], widths[i], heights[i], buff1, ArialNarrowB77, { 200, 150, 150 }, posRess);
        posRess.y += arrond(70 * Zoom);
    }
    ++i;
    
}

static inline void DisplayTexts2(TTF_Font* ArialNarrowB77) {
    char buff1[100];
    sprintf(buff1, "%d", Ress.Pop);
    SDL_Point posRess = { arrond(1200 * Zoom), arrond(5 * Zoom) };
    TTFrender(buff1, ArialNarrowB77, { 200, 200, 200 }, posRess);
    sprintf(buff1, "%d", Ress.Pop);
    posRess.x += arrond(500 * Zoom);
    TTFrender(buff1, ArialNarrowB77, { 200, 200, 200 }, posRess);
    posRess.x += arrond(600 * Zoom);
    sprintf(buff1, "%d", Ress.Trees);
    TTFrender(buff1, ArialNarrowB77, { 200, 200, 200 }, posRess);
    posRess.x += arrond(600 * Zoom);
    sprintf(buff1, "%d", Ress.Animals);
    TTFrender(buff1, ArialNarrowB77, { 200, 200, 200 }, posRess);
}