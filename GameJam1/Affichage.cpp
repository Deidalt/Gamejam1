
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "Main.h"
#include "Fonctions.h"


void TTFrender(char *chaine, TTF_Font *ft, SDL_Color color, SDL_Point posft) {
    //Text rendering
    //can write one line def
    SDL_Surface* HudRessS = TTF_RenderText_Blended(ft, chaine, color);
    SDL_Texture* HudRessT = SDL_CreateTextureFromSurface(Renderer, HudRessS);
    SDL_Rect posT = { posft.x,posft.y,HudRessS->w,HudRessS->h };
    SDL_RenderCopy(Renderer, HudRessT, NULL, &posT);
    SDL_FreeSurface(HudRessS);
    SDL_DestroyTexture(HudRessT);
}

void Afficher() {
    SDL_SetRenderDrawColor(Renderer, 16, 8, 21, 255);
    SDL_RenderClear(Renderer);
    static int Initialised = 0;
    int i = 0, j = 0; //loop
    int wText = 0, hText = 0, wText2,hText2; //dimensions des textures récupérées
    char buff1[100]; //
    float Zoom4K = float(ScreenL.y) / H4K;
    if (Initialised == 0) {
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
        //SDL_RenderSetLogicalSize(Renderer,Wecran/zoom4K,Hecran/zoom4K); //resize tout l'écran

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best"); //utile pour les rotations de Sdl_renderCopyEx
        //SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "0");
        SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1"); //0 par defaut = pas de minimize
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        ScreenL.x = DM.w;
        ScreenL.y = DM.h;
        Zoom = float(ScreenL.y) / H4K;
        printf("Zoom %.2f\n", Zoom);
        Initialised = 1;
    }

    static SDL_Texture* TestT[6];
    static TTF_Font* ArialNarrowB40 = TTF_OpenFont("ttf/Arial-Narrow-Bold.ttf", 40);
    static SDL_Texture* TreeAT[4];
    static SDL_Texture* TreeBT[4];
    static SDL_Texture* TreeCT[4];
    if(Initialised==1){
        //Vars init
        SDL_Surface* TestS[8];
        for (int m = 0;m < 8;m++) {
            TestS[m] = SDL_CreateRGBSurface(0, LCASE, LCASE, 32, 0, 0, 0, 0);
            if(m==0)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 38, 230, 0));
            else if (m == 1)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 128, 42, 0));
            else if (m == 2)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 0, 80, 255));
            else if (m == 3)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 0, 10, 200));
            else if (m == 4)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 0, 77, 0));
            else if (m == 5)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 255, 255, 0));
            else if (m == 6)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 255, 225, 0));
            else if (m == 7)
                SDL_FillRect(TestS[m], NULL, SDL_MapRGB(TestS[m]->format, 255, 185, 0));
            TestT[m] = SDL_CreateTextureFromSurface(Renderer, TestS[m]);
            SDL_FreeSurface(TestS[m]);
        }
        //CaseT = IMG_LoadTexture(Renderer,"Images/Case.png");
        Initialised = 2;
        for (i = 0;i < 4;i++) {
            sprintf(buff1, "Assets/Tiles/Trees/tree_%d_tempere.png", i+1);
            TreeAT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/Trees/tree_%d_snowy.png", i+1);
            TreeBT[i] = IMG_LoadTexture(Renderer, buff1);
            sprintf(buff1, "Assets/Tiles/Trees/tree_%d_glacial.png", i+1);
            TreeCT[i] = IMG_LoadTexture(Renderer, buff1);
        }
    }
    
    //Textures init
    static SDL_Texture* MapA1T = IMG_LoadTexture(Renderer, "Assets/Map/Map_base_tempere1.png");
    static SDL_Texture* MapA2T = IMG_LoadTexture(Renderer, "Assets/Map/Map_base_tempere2.png");
    static SDL_Texture* BackgroundT = IMG_LoadTexture(Renderer, "Assets/Map/Background_tempere.png");
    

    
    //Background and base Map
    QueryText(BackgroundT, &wText, &hText);
    QueryText(MapA1T, &wText2, &hText2);
    //SDL_Rect posBackground = { -posMap.x + wText2-wText/2,-posMap.y+hText2/2-hText/2,wText,hText };
    SDL_Rect posBackground = { 0,0,wText,hText };
    SDL_RenderCopy(Renderer, BackgroundT, NULL, &posBackground);
    SDL_Rect posMapA1 = { -posMap.x,-posMap.y,wText2,hText2 };
    SDL_RenderCopy(Renderer, MapA1T, NULL, &posMapA1);
    QueryText(MapA2T, &wText2, &hText2);
    SDL_Rect posMapA2 = { -posMap.x+posMapA1.w,-posMap.y,wText2,hText2 };
    SDL_RenderCopy(Renderer, MapA2T, NULL, &posMapA2);

    if (1) { //refaire
        //Render Tiles one by one
        //Tiles
        SDL_Texture* CaseT = IMG_LoadTexture(Renderer, "Images/Case.png");
        for (i = 0;i < LMAP;i++) {
            for (j = 0;j < HMAP;j++) {
                QueryText(CaseT, &wText, &hText);
                SDL_Point ObjectP = { i * wText / 2 ,j * hText  };
                SDL_Point ObjectIsoP = ToIso(ObjectP);
                SDL_Rect posObject = { ObjectIsoP.x - posMap.x,ObjectIsoP.y - posMap.y,wText,hText  };
                //SDL_Rect posObjectB = { ObjectP.x,ObjectP.y,wText,hText };
                //SDL_RenderCopy(Renderer, TestT[Grid[i][j].Object], NULL, &posObject);
                if (Grid[i][j].Object == 1)
                    SDL_SetTextureColorMod(CaseT, 152, 57, 0);
                else if (Grid[i][j].Object == 2)
                    SDL_SetTextureColorMod(CaseT, 100, 100, 250);
                else if (Grid[i][j].Object == 3)
                    SDL_SetTextureColorMod(CaseT, 30, 0, 200);
                else if (Grid[i][j].Object == 4) {
                    if (Grid[i][j].State == 4)
                        SDL_SetTextureColorMod(CaseT, 100, 150, 100);
                    else
                        SDL_SetTextureColorMod(CaseT, 100, 255, 100);
                }
                else if (Grid[i][j].Object == 5)
                    SDL_SetTextureColorMod(CaseT, 200, 200, 100);
                else if (Grid[i][j].Object == 6)
                    SDL_SetTextureColorMod(CaseT, 350, 300, 200);
                else
                    SDL_SetTextureColorMod(CaseT, 255, 255, 255);
                //SDL_RenderCopy(Renderer, CaseT, NULL, &posObject);
                for (int arb = 0; arb < 4; arb++) {
                    int randTree = rand() % 3;
                    //int randTree = j % 3;
                    QueryText(TreeAT[randTree], &wText, &hText);
                    SDL_Rect posTreeA = { (150 * (LMAP-i-1) + 150 * j) * Zoom - posMap.x ,hText2 / 2 + ((74 * i) - (74 * (LMAP-j-1))) * Zoom - posMap.y - hText,wText,hText };
                    if (arb == 0)
                        posTreeA.x += 75 * Zoom;
                    else if (arb == 1)
                        posTreeA.x += 225 * Zoom;
                    else if (arb == 2) {
                        posTreeA.x += 150 * Zoom;
                        posTreeA.y -= 40 * Zoom;
                    }
                    else if (arb == 3) {
                        posTreeA.x += 150 * Zoom;
                        posTreeA.y += 40 * Zoom;
                    }
                    if (randTree == 0) {
                        posTreeA.x -= 67 * Zoom;
                        posTreeA.y -= 165 * Zoom;
                    }
                    if (randTree == 1) {
                        posTreeA.x -= 165 * Zoom;
                        posTreeA.y -= 135 * Zoom;
                    }
                    if (randTree == 2) {
                        posTreeA.x -= 215 * Zoom;
                        posTreeA.y -= 167 * Zoom;
                    }
                    //printf("dda %d %d %d %d\n", posTreeA.x, posTreeA.y, posTreeA.w, posTreeA.h);
                    SDL_RenderCopy(Renderer, TreeAT[randTree], NULL, &posTreeA);
                }
            }
        }
        SDL_DestroyTexture(CaseT);
    }

    //HUD HAUT
    sprintf(buff1, "Food   Pop=%d Trees=%d Animals=%d",Ress.Pop,Ress.Trees,Ress.Animals);
    SDL_Point posRess = { 1000 * Zoom,50 * Zoom };
    TTFrender(buff1, ArialNarrowB40, { 255,255,255 }, posRess);
    sprintf(buff1, "-%d",Ress.Pop);
    posRess.y += 40 * Zoom;
    TTFrender(buff1, ArialNarrowB40, { 255,150,150 }, posRess);
    sprintf(buff1, "Gathering +5");
    posRess.y += 40 * Zoom;
    TTFrender(buff1, ArialNarrowB40, { 150,255,150 }, posRess);
    if (Ress.Hunt) {
        sprintf(buff1, "Hunt +%d", Ress.Hunt * 5);
        posRess.y += 40 * Zoom;
        TTFrender(buff1, ArialNarrowB40, { 150,255,150 }, posRess);
    }

    SDL_RenderPresent(Renderer);
}