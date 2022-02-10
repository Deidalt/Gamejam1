
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "Main.h"
#include "Fonctions.h"
#include "Pixel.h"

#define OMAPY 1446
#define DECALAGE 0 //Manque un bout à gauche de la map donc on décale tout

struct sTree {
    int init = 0;
    SDL_Point posArb[4]; 
};

void TTFrender(const char *chaine, TTF_Font *ft, SDL_Color color, SDL_Point posft) {
    //Text rendering
    //can write one line
    SDL_Surface* HudRessS = TTF_RenderText_Blended(ft, chaine, color);
    SDL_Texture* HudRessT = SDL_CreateTextureFromSurface(Renderer, HudRessS);
    SDL_Rect posT = { posft.x,posft.y,HudRessS->w,HudRessS->h };
    SDL_RenderCopy(Renderer, HudRessT, NULL, &posT);
    SDL_FreeSurface(HudRessS);
    SDL_DestroyTexture(HudRessT);
}

void InitAffichage() {
    //Libs init
    Screen = SDL_CreateWindow("Hello", 0, 0, 640, 480, SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP/*   | SDL_WINDOW_ALLOW_HIGHDPI*/);
    if (Screen == NULL)
        printf("NULL Window\n");
    Renderer = SDL_CreateRenderer(Screen, -1, SDL_RENDERER_ACCELERATED); //SDL_RENDERER_PRESENTVSYNC //SDL_RENDERER_TARGETTEXTURE
    if (Renderer == NULL)
        printf("NULL renderer\n");
    if (TTF_Init() == -1)
        printf("TTF %s\n", TTF_GetError());

    SDL_Surface* IconeExe = IMG_Load_RW(SDL_RWFromFile("Images/Case.png", "rb"), 1);

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

void Afficher() {
    SDL_FPoint CaseL = { LCASE + 0.7,HCASE + 0.1 };
    SDL_SetRenderDrawColor(Renderer, 16, 8, 21, 255);
    SDL_RenderClear(Renderer);
    static int Initialised = 0;
    int i = 0, j = 0; //loop
    int wText = 0, hText = 0, wText2, hText2; //dimensions des textures récupérées
    char buff1[100];
    char buff2[50];
    static sTree posTree[30][30];

    static TTF_Font* ArialNarrowB32 = TTF_OpenFont("ttf/Arial-Narrow-Bold.ttf", 32);
    //Textures init
    static SDL_Texture* MapBase1T[3];
    static SDL_Texture* MapBase2T[3];
    static SDL_Texture* MapRe1T[3];
    static SDL_Texture* MapRe2T[3];
    static SDL_Texture* BackgroundT = IMG_LoadTexture(Renderer, "Assets/Map/Background_tempere.png");
    static SDL_Texture* TreeAT[4];
    static SDL_Texture* TreeBT[4];
    static SDL_Texture* TreeCT[4];
    static SDL_Texture* HouseAT[4];
    static SDL_Texture* HouseBT[4];
    static SDL_Texture* HouseCT[4];
    static SDL_Texture* MillT;
    static SDL_Texture* FieldT[3];
    static SDL_Texture* ShipT;
    static SDL_Texture* RiverT[3];
    static SDL_Texture* RainT[7];
    static SDL_Texture* SnowT[80];
    static SDL_Texture* FireT[6];
    static SDL_Texture* BuildingT;
    static SDL_Texture* FondNoirT;
    static SDL_Texture* FrameT[4];
    static SDL_Texture* SpeedT[2];

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
            sprintf(buff1, "Assets/Tiles/House/tribal_house_%d.png", i + 1);
            HouseAT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/House/medieval_house_%d.png", i + 1);
            HouseBT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/House/contemporary_house_%d.png", i + 1);
            HouseCT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/UI/Frame%d.png", i);
            FrameT[i] = IMG_LoadTexture(Renderer, buff1);

        }
        for (i = 0;i < 3;i++) {
            sprintf(buff1, "Assets/Map/Relief%c.png", 'A' + i);
            MapRe1T[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Map/MAP_base%c.png", 'A' + i);
            MapBase1T[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/River/river%d.png", i);
            RiverT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 7;i++) {
            sprintf(buff1, "Assets/Fx/Pluie/Pluie_%05d.png", i);
            RainT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 80;i++) {
            sprintf(buff1, "Assets/Fx//Neige/Neige 2_%05d.png", i);
            //SnowT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 6;i++) {
            sprintf(buff1, "Assets/Fx/Feu/Feu_1_%05d.png", i);
            FireT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        sprintf(buff1, "Assets/Tiles/Other/moulin.png");
        MillT = IMG_LoadTexture(Renderer, buff1);
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
        sprintf(buff1, "Assets/Tiles/construction.png");
        BuildingT = IMG_LoadTexture(Renderer, buff1);
        sprintf(buff1, "Assets/UI/FondNoir.png");
        FondNoirT = IMG_LoadTexture(Renderer, buff1);
        Year = 0; //Game Starts here

    }



    int Period = (Year / YEARS_PER_SEASON) % 4;
    if (Period == 2)
        Period = 0;
    if (Period == 3)
        Period = 2;

    //Background and base Map
    QueryText4(BackgroundT, &wText, &hText);
    QueryText4(MapBase1T[Period], &wText2, &hText2);
    //SDL_Rect posBackground = { -posxy.x + wText2-wText/2,-posxy.y+hText2/2-hText/2,wText,hText };
    SDL_Rect posBackground = { 0,0,wText,hText };
    SDL_RenderCopy(Renderer, BackgroundT, NULL, &posBackground);
    SDL_Rect posMap = { -posxy.x,-posxy.y,wText2,hText2 };
    SDL_RenderCopy(Renderer, MapBase1T[Period], NULL, &posMap);
    /*QueryText(MapBase2T[Period], &wText2, &hText2);
    SDL_Rect posMap2 = { posMap.x+posMap.w,posMap.y,wText2,hText2 };
    SDL_RenderCopy(Renderer, MapBase2T[Period], NULL, &posMap2);*/
    QueryText4(MapRe1T[Period], &wText, &hText);
    SDL_Rect posMapRe1 = { posMap.x,arrond(posMap.y - hText + posMap.h),wText,hText };
    SDL_RenderCopy(Renderer, MapRe1T[Period], NULL, &posMapRe1);
    /*QueryText(MapRe2T[Period], &wText, &hText);
    SDL_Rect posMapRe2 = { arrond(posMap.x+4895*Zoom7K),arrond(posMap.y - hText + posMap.h),wText,hText };
    SDL_RenderCopy(Renderer, MapRe2T[Period], NULL, &posMapRe2);*/
    QueryText4(RiverT[Period], &wText, &hText);
    SDL_Rect posRiver = { arrond(posMap.x),arrond(posMap.y + posMap.h - hText),wText,hText };
    if (Ress.River) {
        SDL_RenderCopy(Renderer, RiverT[Ress.River - 1], NULL, &posRiver);
    }

    if (1) { //refaire
        //Render Tiles one by one
        //Tiles
        SDL_Texture* CaseT = IMG_LoadTexture(Renderer, "Images/Case.png"); //delete test
        //i et j sont inversés à l'affichage à cause de l'isometrie
        //reverse i and j for isometric
        for (i = 0;i < LMAP;i++) {
            for (j = 0;j < HMAP;j++) {
                QueryText(CaseT, &wText, &hText);
                SDL_Point ObjectP = { i * wText / 2  ,j * hText };
                SDL_Point ObjectIsoP = ToIso(ObjectP);
                SDL_Rect posObject = { ObjectIsoP.x - posxy.x - 300 * Zoom,ObjectIsoP.y - posxy.y,wText,hText };
                //SDL_Rect posObjectB = { ObjectP.x,ObjectP.y,wText,hText };
                if (Grid[j][i].Object == MOUNTAIN) {
                    SDL_SetTextureColorMod(CaseT, 152, 57, 0);
                }
                else if (Grid[j][i].Object == RIVER) {
                    SDL_SetTextureColorMod(CaseT, 100, 100, 250);
                    /*wText = arrond(CaseL.x * Zoom*1.8);
                    hText = arrond(CaseL.y * Zoom*1.8);
                    SDL_Rect posRiver = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * i) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText / 2,wText,hText };
                    SDL_RenderCopy(Renderer, CaseT, NULL, &posRiver);*/
                }
                else if (Grid[j][i].Object == SEA) {
                    SDL_SetTextureColorMod(CaseT, 30, 0, 200);
                }
                else if (Grid[j][i].Object == FOREST) {
                    if (Grid[j][i].State == 4)
                        SDL_SetTextureColorMod(CaseT, 100, 150, 100);
                    else
                        SDL_SetTextureColorMod(CaseT, 100, 255, 100);
                    if (Grid[j][i].State > 4) { //fire
                        static int cptFire = 0;
                        static int timerFire = SDL_GetTicks() + 83;
                        if (timerFire < SDL_GetTicks()) {
                            cptFire++;
                            if (cptFire > 5)
                                cptFire = 0;
                            timerFire = SDL_GetTicks() - timerFire + SDL_GetTicks() + 83;
                            timerFire < SDL_GetTicks();
                            timerFire = SDL_GetTicks() + 83;
                        }
                        QueryText4(FireT[cptFire], &wText, &hText);
                        SDL_Rect posFire = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                        SDL_RenderCopy(Renderer, FireT[cptFire], NULL, &posFire);
                    }
                    int idtree = Grid[j][i].id;
                    if (Grid[j][i].State < 4) {
                        for (int arb = 0; arb < 4 - Grid[j][i].State % 5; arb++) {
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
                                    posTree[j][i].posArb[arb].x = arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) + arrond(Zoom * CaseL.x * 1.5);
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
                            if (Period < 2) {
                                SDL_RenderCopy(Renderer, TreeAT[idtree], NULL, &posFinal);
                            }
                            else if (triggerCold) {
                                SDL_RenderCopy(Renderer, TreeBT[idtree], NULL, &posFinal);
                            }
                            else if (Period == 2) {
                                SDL_RenderCopy(Renderer, TreeCT[idtree], NULL, &posFinal);
                            }
                        }
                    }
                    else if (Grid[j][i].State > 4) {
                        for (int arb = 0; arb < 4 - Grid[j][i].State % 5; arb++) {
                            QueryText(TreeAT[idtree], &wText, &hText);
                            SDL_Rect posFinal = { posTree[j][i].posArb[arb].x - posxy.x,posTree[j][i].posArb[arb].y - posxy.y,wText,hText };
                            if (Period < 2) {
                                SDL_SetTextureColorMod(TreeAT[idtree], 255, 10, 50);
                                SDL_RenderCopy(Renderer, TreeAT[idtree], NULL, &posFinal);
                                SDL_SetTextureColorMod(TreeAT[idtree], 255, 255, 255);
                            }
                            else if (triggerCold) {
                                SDL_SetTextureColorMod(TreeBT[idtree], 255, 10, 50);
                                SDL_RenderCopy(Renderer, TreeBT[idtree], NULL, &posFinal);
                                SDL_SetTextureColorMod(TreeBT[idtree], 255, 255, 255);
                            }
                            else if (Period == 2) {
                                SDL_SetTextureColorMod(TreeCT[idtree], 255, 10, 50);
                                SDL_RenderCopy(Renderer, TreeCT[idtree], NULL, &posFinal);
                                SDL_SetTextureColorMod(TreeCT[idtree], 255, 255, 255);
                            }
                        }
                    }
                }
                else if (Grid[j][i].Object == HUT) {
                    SDL_SetTextureColorMod(CaseT, 255, 153, 255);
                    if (Grid[j][i].State > SDL_GetTicks()) {
                        //Building
                        QueryText4(BuildingT, &wText, &hText);
                        SDL_Rect posBuilding = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                        SDL_RenderCopy(Renderer, BuildingT, NULL, &posBuilding);

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
                    if (Grid[j][i].State > SDL_GetTicks()) {
                        //Building
                        QueryText4(BuildingT, &wText, &hText);
                        SDL_Rect posBuilding = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                        SDL_RenderCopy(Renderer, BuildingT, NULL, &posBuilding);

                    }
                    else {
                        if (Grid[j][i].State < SDL_GetTicks())
                            Grid[j][i].State = 1;
                        SDL_SetTextureColorMod(CaseT, 204, 0, 204);
                        QueryText4(HouseBT[Grid[j][i].id], &wText, &hText);
                        SDL_Rect posHouseB = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                        SDL_RenderCopy(Renderer, HouseBT[Grid[j][i].id], NULL, &posHouseB);
                    }
                }
                else if (Grid[j][i].Object == APPART) {
                    SDL_SetTextureColorMod(CaseT, 102, 0, 102);
                    QueryText4(HouseCT[Grid[j][i].id], &wText, &hText);
                    SDL_Rect posHouseC = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                    SDL_RenderCopy(Renderer, HouseCT[Grid[j][i].id], NULL, &posHouseC);
                }

                else if (Grid[j][i].Object == FIELD) {
                    if (i == 20 && j == 13) {
                        if (Grid[j][i].State > SDL_GetTicks()) {
                            //Building
                            QueryText4(BuildingT, &wText, &hText);
                            SDL_Rect posBuilding = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, BuildingT, NULL, &posBuilding);

                        }
                        else {
                            if (Grid[j][i].State < SDL_GetTicks())
                                Grid[j][i].State = 1;
                            SDL_SetTextureColorMod(CaseT, 230, 153, 0);
                            QueryText4(MillT, &wText, &hText);
                            SDL_Rect posMill = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, MillT, NULL, &posMill);
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
                            SDL_SetTextureColorMod(CaseT, 255, 255, 153);
                            QueryText4(FieldT[idField], &wText, &hText);
                            SDL_Rect posField = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                            SDL_RenderCopy(Renderer, FieldT[idField], NULL, &posField);
                        }
                    }
                }
                else if (Grid[j][i].Object == SHIP) {
                    SDL_SetTextureColorMod(CaseT, 255, 255, 255);
                    QueryText4(ShipT, &wText, &hText);
                    SDL_Rect posShip = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * (i + 1)) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText,wText,hText };
                    SDL_RenderCopy(Renderer, ShipT, NULL, &posShip);
                }
                else if (Grid[j][i].Object == HOSPI) {
                    SDL_SetTextureColorMod(CaseT, 255, 255, 255);
                }
                else if (Grid[j][i].Object == FIRESTATION) {
                    SDL_SetTextureColorMod(CaseT, 255, 0, 0);
                }
                else if (Grid[j][i].Object == BARRAGE) {
                    SDL_SetTextureColorMod(CaseT, 200, 200, 200);
                }
                else if (Grid[j][i].Object == SECOURIST) {
                    SDL_SetTextureColorMod(CaseT, 0, 0, 0);
                }
                else {
                    SDL_SetTextureColorMod(CaseT, 100, 200, 100);
                }
                SDL_RenderCopy(Renderer, CaseT, NULL, &posObject);

            }
        }
        SDL_DestroyTexture(CaseT);
    }
    //FX
    if (rain) {
        static int cptRain = 0;
        static int timerRain = SDL_GetTicks() + 83;
        if (timerRain < SDL_GetTicks()) {
            cptRain++;
            if (cptRain > 6) 
                cptRain = 0;
            timerRain = SDL_GetTicks() - timerRain + SDL_GetTicks() + 83;
            timerRain < SDL_GetTicks();
            timerRain = SDL_GetTicks() + 83;
        }
        QueryText4(RainT[0], &wText, &hText);
        SDL_Rect posMeteo = { 0,0,wText,hText };
        SDL_RenderCopy(Renderer, RainT[cptRain], NULL, &posMeteo);

    }
    else if (triggerCold) {
        static int cptSnow = 0;
        static int timerSnow = SDL_GetTicks() + 83;
        if (timerSnow < SDL_GetTicks()) {
            cptSnow++;
            if (cptSnow > 79)
                cptSnow = 0;
            timerSnow = SDL_GetTicks() - timerSnow + SDL_GetTicks() + 83;
            timerSnow < SDL_GetTicks();
            timerSnow = SDL_GetTicks() + 83;
        }
        QueryText2(SnowT[0], &wText, &hText);
        SDL_Rect posMeteo = { 0,0,wText,hText };
        SDL_RenderCopy(Renderer, SnowT[cptSnow], NULL, &posMeteo);
    }

    //HUD HAUT
    
    if (Menu == NONE) {
        SDL_Rect posNoir = { 0,0,ScreenL.x,100 * Menu * Zoom };
        SDL_RenderCopy(Renderer, FondNoirT, NULL, &posNoir);
        for (i = 0;i < ScreenL.x / 60 + 1;i++) {
            SDL_Rect posFrame = { i * 60,0,64,8 };
            SDL_RenderCopy(Renderer, FrameT[0], NULL, &posFrame);
            posFrame.y = arrond(100 * Menu * Zoom) - 8;
            SDL_RenderCopy(Renderer, FrameT[1], NULL, &posFrame);
        }
        for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
            SDL_Rect posFrame = { 0,i * 60,8,64 };
            if (posFrame.y + posFrame.h > 100 * Menu * Zoom)
                posFrame.y = 100 * Menu * Zoom - 64;
            SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            posFrame.x = ScreenL.x - 8;
            SDL_RenderCopy(Renderer, FrameT[3], NULL, &posFrame);
        }
        


        
    }
    else if (Menu == UIUP) {
        SDL_Rect posNoir = { 0,0,ScreenL.x,100 * Menu * Zoom };
        SDL_RenderCopy(Renderer, FondNoirT, NULL, &posNoir);
        for (i = 0;i < ScreenL.x / 60 + 1;i++) {
            SDL_Rect posFrame = { i * 60,0,64,8 };
            SDL_RenderCopy(Renderer, FrameT[0], NULL, &posFrame);
            posFrame.y = arrond(100 * Menu * Zoom) - 8;
            SDL_RenderCopy(Renderer, FrameT[1], NULL, &posFrame);
        }
        for (i = 0;i < arrond(100 * Menu * Zoom) / 60 + 1;i++) {
            SDL_Rect posFrame = { 0,i * 60,8,64 };
            if (posFrame.y + posFrame.h > 100 * Menu * Zoom)
                posFrame.y = 100 * Menu * Zoom - 64;
            SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame);
            posFrame.x = ScreenL.x - 8;
            SDL_RenderCopy(Renderer, FrameT[3], NULL, &posFrame);
        }

        sprintf(buff1, "Food  |  Pop=%d Trees=%d Animals=%d  ||  Action=%s [%s]", Ress.Pop, Ress.Trees, Ress.Animals, ActionName(), GetPeriodName());
        SDL_Point posRess = { arrond(1100 * Zoom), arrond(10 * Zoom) };
        TTFrender(buff1, ArialNarrowB32, { 255, 255, 255 }, posRess);
#pragma warning(suppress : 4996)
        sprintf(buff1, "-%d", Ress.Pop);
        posRess.y += arrond(70 * Zoom);
        TTFrender(buff1, ArialNarrowB32, { 255, 150, 150 }, posRess);
#pragma warning(suppress : 4996)
        sprintf(buff1, "Gathering +5");
        posRess.y += arrond(70 * Zoom);
        TTFrender(buff1, ArialNarrowB32, { 150, 255, 150 }, posRess);
        if (Ress.Hunt) {
#pragma warning(suppress : 4996)
            sprintf(buff1, "Hunt +%d", Ress.Hunt * 10);
            posRess.y += arrond(70 * Zoom);
            TTFrender(buff1, ArialNarrowB32, { 150, 255, 150 }, posRess);
        }
        if (Ress.Fish) {
            sprintf(buff1, "Fish +%d", Ress.Fish * 10);
            posRess.y += arrond(70 * Zoom);
            TTFrender(buff1, ArialNarrowB32, { 150,255,150 }, posRess);
        }
        if (Ress.Harvest && Ress.River) {
            sprintf(buff1, "Harvest +%d", Ress.Harvest * 5);
            posRess.y += arrond(70 * Zoom);
            TTFrender(buff1, ArialNarrowB32, { 150,255,150 }, posRess);
        }
        if (IsColdOn()) {
            sprintf(buff1, "Sick %d", GetSickNumber());
            posRess.y += arrond(70 * Zoom);
            TTFrender(buff1, ArialNarrowB32, { 150, 255, 150 }, posRess);
        }
    }
    QueryText2(SpeedT[0], &wText, &hText);
    SDL_Rect posSpeed = { arrond(20 * Zoom),arrond(20 * Zoom),wText,hText };
    SDL_RenderCopy(Renderer, SpeedT[1], NULL, &posSpeed);
    posSpeed.x += arrond(20 * Zoom);
    SDL_RenderCopy(Renderer, SpeedT[1], NULL, &posSpeed);
    posSpeed.x += arrond(70 * Zoom);
    sprintf(buff1, "X%d", 2000 / timeTurn);
    SDL_Point posXtime = { posSpeed.x, arrond(8 * Zoom) };
    TTFrender(buff1, ArialNarrowB32, { 150, 150, 150 }, posXtime);
    posSpeed.x += arrond(80 * Zoom);
    SDL_RenderCopy(Renderer, SpeedT[0], NULL, &posSpeed);
    posSpeed.x += arrond(20 * Zoom);
    SDL_RenderCopy(Renderer, SpeedT[0], NULL, &posSpeed);
    sprintf(buff1, "%s era | Year %d", GetEraName(), Year);
    SDL_Point posYear = { arrond(300 * Zoom), arrond(10 * Zoom) };
    TTFrender(buff1, ArialNarrowB32, { 100, 100, 100 }, posYear);
    for (j = 0; j < 7;j++) {
        //Actions
        SDL_Rect posNoir = { arrond(200 * Zoom + j * 220 * Zoom),arrond(1900 * Zoom),arrond(200 * Zoom),arrond(200 * Zoom) };
        SDL_RenderCopy(Renderer, FondNoirT, NULL, &posNoir);
        SDL_Rect posFrame = { arrond(200 * Zoom + j * 220 * Zoom),arrond(1900 * Zoom),arrond(200 * Zoom),8 };
        SDL_RenderCopy(Renderer, FrameT[0], NULL, &posFrame);
        posFrame.y += arrond(200 * Zoom) - 8;
        SDL_RenderCopy(Renderer, FrameT[1], NULL, &posFrame);

        SDL_Rect posFrame2 = { arrond(200 * Zoom + j * 220 * Zoom),arrond(1900 * Zoom),8,arrond(200 * Zoom) };
        SDL_RenderCopy(Renderer, FrameT[2], NULL, &posFrame2);
        posFrame2.x += arrond(200 * Zoom) - 8;
        SDL_RenderCopy(Renderer, FrameT[3], NULL, &posFrame2);
        sprintf(buff1, "%d", j);
        SDL_Point posAction = { arrond(200 * Zoom + j * 220 * Zoom+ posNoir.w/2 -18*Zoom),arrond(1900 * Zoom)+ posNoir.h/2};
        TTFrender(buff1, ArialNarrowB32, { 255, 150, 150 }, posAction);
    }

    SDL_RenderPresent(Renderer);
}