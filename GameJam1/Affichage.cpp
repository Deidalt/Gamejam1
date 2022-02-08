
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "Main.h"
#include "Fonctions.h"
#include "Pixel.h"

#define OMAPY 1446
#define DECALAGE 0 //Manque un bout à gauche de la map donc on décale tout


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
    Screen = SDL_CreateWindow("Hello", 0, 0, 640, 480, SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED/*   | SDL_WINDOW_ALLOW_HIGHDPI*/);
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

    printf("Zoom %.2f\n", Zoom);
}

static inline const char* const ActionName() {
    static const char* names[NB_ACTIONS] = {
        "Plant", "Rain", "Cold", "Meteor", "Devour", "Drown"
    };

    return names[getCurrentAction()];
}

void Afficher() {
    SDL_FPoint CaseL = { LCASE + 0.7,HCASE + 0.1 };
    SDL_SetRenderDrawColor(Renderer, 16, 8, 21, 255);
    SDL_RenderClear(Renderer);
    static int Initialised = 0;
    int i = 0, j = 0; //loop
    int wText = 0, hText = 0, wText2,hText2; //dimensions des textures récupérées
    char buff1[100]; //

    static TTF_Font* ArialNarrowB40 = TTF_OpenFont("ttf/Arial-Narrow-Bold.ttf", 40);
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
    static SDL_Texture* MillT;
    static SDL_Texture* RiverT[3];
    static SDL_Texture* RainT[7];
    static SDL_Texture* SnowT[80];

    if (Initialised == 0) {
        //Vars init
        Initialised = 2;
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
            sprintf(buff1, "Assets/Fx/Pluie/Pluie_%05d.png",i);
            RainT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        for (i = 0;i < 80;i++) {
            sprintf(buff1, "Assets/Fx//NeigeNeige_%05d.png", i);
            SnowT[i] = IMG_LoadTexture(Renderer, buff1);
        }
        sprintf(buff1, "Assets/Tiles/House/moulin.png");
        MillT = IMG_LoadTexture(Renderer, buff1);
    }
    
    

    int Period = (Year / YEARS_PER_SEASON) % 3;
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
    SDL_Rect posMapRe1 = { posMap.x,arrond(posMap.y- hText +posMap.h),wText,hText };
    SDL_RenderCopy(Renderer, MapRe1T[Period], NULL, &posMapRe1);
    /*QueryText(MapRe2T[Period], &wText, &hText);
    SDL_Rect posMapRe2 = { arrond(posMap.x+4895*Zoom7K),arrond(posMap.y - hText + posMap.h),wText,hText };
    SDL_RenderCopy(Renderer, MapRe2T[Period], NULL, &posMapRe2);*/
    QueryText4(RiverT[Period], &wText, &hText);
    SDL_Rect posRiver = { arrond(posMap.x ),arrond(posMap.y+posMap.h-hText ),wText,hText };
    if (Ress.River) {
        SDL_RenderCopy(Renderer, RiverT[Ress.River-1], NULL, &posRiver);
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
                SDL_Point ObjectP = { i * wText/2  ,j * hText  };
                SDL_Point ObjectIsoP = ToIso(ObjectP);
                SDL_Rect posObject = { ObjectIsoP.x - posxy.x-300*Zoom,ObjectIsoP.y - posxy.y,wText,hText  };
                //SDL_Rect posObjectB = { ObjectP.x,ObjectP.y,wText,hText };
                if (Grid[j][i].Object == MOUNTAIN) {
                    SDL_SetTextureColorMod(CaseT, 152, 57, 0);
                }
                else if (Grid[j][i].Object == RIVER) {
                    SDL_SetTextureColorMod(CaseT, 100, 100, 250);
                    wText = arrond(LCASE * Zoom );
                    hText = arrond(HCASE * Zoom );
                    SDL_Rect posRiver = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * i) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText/2,wText,hText };
                    SDL_RenderCopy(Renderer, CaseT, NULL, &posRiver);

                }
                else if (Grid[j][i].Object == SEA) {
                    SDL_SetTextureColorMod(CaseT, 30, 0, 200);
                    wText = arrond(CaseL.x * Zoom);
                    hText = arrond(CaseL.y * Zoom);
                    SDL_Rect posRiver = { arrond((CaseL.x * (LMAP - i - 1) + CaseL.x * j - DECALAGE) * Zoom) - posxy.x, arrond((OMAPY + (CaseL.y * i) - (CaseL.y * (LMAP - j - 1))) * Zoom) - posxy.y - hText / 2,wText,hText };
                    SDL_RenderCopy(Renderer, CaseT, NULL, &posRiver);
                }
                else if (Grid[j][i].Object == FOREST) {
                    if (Grid[j][i].State == 4)
                        SDL_SetTextureColorMod(CaseT, 100, 150, 100);
                    else
                        SDL_SetTextureColorMod(CaseT, 100, 255, 100);
                    for (int arb = 0; arb < 4 - Grid[j][i].State; arb++) { //
                        //Mapping des arbres RandTree à set en static
                        int randTree = 0; //rand à faire
                        //int randTree = j % 3;
                        QueryText(TreeAT[randTree], &wText, &hText);
                        SDL_Rect posTreeA = { arrond((LCASE * (LMAP - i - 1) + LCASE * j) * Zoom) - posxy.x, arrond(posMap.h / 2 + ((HCASE * i) - (HCASE * (LMAP - j - 1))) * Zoom) - posxy.y - hText, wText, hText };
                        if (arb == 0)
                            posTreeA.x += arrond(75 * Zoom7K);
                        else if (arb == 1)
                            posTreeA.x += arrond(225 * Zoom7K);
                        else if (arb == 2) {
                            posTreeA.x += arrond(150 * Zoom7K);
                            posTreeA.y -= arrond(40 * Zoom7K);
                        }
                        else if (arb == 3) {
                            posTreeA.x += arrond(150 * Zoom7K);
                            posTreeA.y += arrond(40 * Zoom7K);
                        }
                        if (randTree == 0) {
                            posTreeA.x -= arrond(67 * Zoom7K);
                            posTreeA.y -= arrond(165 * Zoom7K);
                        }
                        if (randTree == 1) {
                            posTreeA.x -= arrond(165 * Zoom7K);
                            posTreeA.y -= arrond(135 * Zoom7K);
                        }
                        if (randTree == 2) {
                            posTreeA.x -= arrond(215 * Zoom7K);
                            posTreeA.y -= arrond(167 * Zoom7K);
                        }
                        SDL_RenderCopy(Renderer, TreeAT[randTree], NULL, &posTreeA);
                    }
                }
                else if (Grid[j][i].Object == HUT) {
                    SDL_SetTextureColorMod(CaseT, 255, 153, 255);
                    int randHouse = 0; //rand à faire
                    QueryText(HouseAT[randHouse], &wText, &hText);
                    SDL_Rect posHouseA = { arrond((LCASE * (LMAP - i - 1) + LCASE * j - DECALAGE) * Zoom) - posxy.x, arrond(posMap.h / 2 + ((HCASE * i) - (HCASE * (LMAP - j - 1))) * Zoom) - posxy.y - hText, wText, hText };
                    SDL_RenderCopy(Renderer, HouseAT[randHouse], NULL, &posHouseA);

                }
                else if (Grid[j][i].Object == HOUSE) {
                    SDL_SetTextureColorMod(CaseT, 204, 0, 204);
                    int randHouse = 0; //rand à faire
                    QueryText4(HouseBT[randHouse], &wText, &hText);
                    SDL_Rect posHouseB = { arrond((LCASE * (LMAP - i - 1) + LCASE * j - DECALAGE) * Zoom) - posxy.x, arrond(posMap.h / 2 + ((HCASE * i) - (HCASE * (LMAP - j - 1))) * Zoom) - posxy.y - hText, wText, hText };
                    SDL_RenderCopy(Renderer, HouseBT[randHouse], NULL, &posHouseB);
                }
                else if (Grid[j][i].Object == APPART)
                    SDL_SetTextureColorMod(CaseT, 102, 0, 102);
                else if (Grid[j][i].Object == FIELD) {
                    if (i == 20 && j == 13) {
                        printf("AAA\n");
                        SDL_SetTextureColorMod(CaseT, 230, 153, 0);
                        QueryText4(MillT, &wText, &hText);
                        SDL_Rect posMill = { arrond((LCASE * (LMAP - i - 1) + LCASE * j - DECALAGE) * Zoom) - posxy.x, arrond(posMap.h / 2 + ((HCASE * i) - (HCASE * (LMAP - j - 1))) * Zoom) - posxy.y - hText, wText, hText };
                        SDL_RenderCopy(Renderer, MillT, NULL, &posMill);
                    }
                    else
                        SDL_SetTextureColorMod(CaseT, 255, 255, 153);
                }
                else if (Grid[j][i].Object == SHIP) {
                    SDL_SetTextureColorMod(CaseT, 255, 255, 255);
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
        static int timerSnow = SDL_GetTicks()+83;
        if (timerSnow < SDL_GetTicks()) {
            cptSnow++;
            if (cptSnow > 79)
                cptSnow = 0;
            timerSnow = SDL_GetTicks()-timerSnow+SDL_GetTicks()+83;
            timerSnow < SDL_GetTicks();
                timerSnow = SDL_GetTicks() + 83;
        }
        QueryText4(SnowT[0], &wText, &hText);
        SDL_Rect posMeteo = { 0,0,wText,hText };
        SDL_RenderCopy(Renderer, SnowT[cptSnow], NULL, &posMeteo);
    }

    //HUD HAUT
    #pragma warning(suppress : 4996)
    sprintf(buff1, "Food   Pop=%d Trees=%d Animals=%d ;; Action=%s [%s]",Ress.Pop,Ress.Trees,Ress.Animals, ActionName(), GetEraName());
    SDL_Point posRess = { arrond(1000 * Zoom), arrond(50 * Zoom) };
    TTFrender(buff1, ArialNarrowB40, { 255, 255, 255 }, posRess);
    #pragma warning(suppress : 4996)
    sprintf(buff1, "-%d", Ress.Pop);
    posRess.y += arrond(70 * Zoom);
    TTFrender(buff1, ArialNarrowB40, { 255, 150, 150 }, posRess);
    #pragma warning(suppress : 4996)
    sprintf(buff1, "Gathering +5");
    posRess.y += arrond(70 * Zoom);
    TTFrender(buff1, ArialNarrowB40, { 150, 255, 150 }, posRess);
    if (Ress.Hunt) {
        #pragma warning(suppress : 4996)
        sprintf(buff1, "Hunt +%d", Ress.Hunt * 5);
        posRess.y += arrond(70 * Zoom);
        TTFrender(buff1, ArialNarrowB40, { 150, 255, 150 }, posRess);
    }
    if (Ress.Fish) {
        sprintf(buff1, "Fish +%d", Ress.Fish * 10);
        posRess.y += arrond(70 * Zoom);
        TTFrender(buff1, ArialNarrowB40, { 150,255,150 }, posRess);
    }
    if (Ress.Harvest) {
        sprintf(buff1, "Harvest +%d", Ress.Harvest * 5);
        posRess.y += arrond(70 * Zoom);
        TTFrender(buff1, ArialNarrowB40, { 150,255,150 }, posRess);
    }
    if (IsColdOn()) {
        sprintf(buff1, "Sick %d", GetSickNumber());
        posRess.y += arrond(70 * Zoom);
        TTFrender(buff1, ArialNarrowB40, { 150, 255, 150 }, posRess);
    }

    SDL_RenderPresent(Renderer);
}