



#include "Main.h"
#include "Pixel.h"

void Evenement() {
	SDL_Event eventV;

	while (SDL_PollEvent(&eventV)) {
        switch (eventV.type)
        {
        case SDL_QUIT: { //close
            EndMain = 0;
            break;

        }
        case SDL_KEYDOWN: {
            if (eventV.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {//menu
                if (Menu==ESCAPE)
                    Menu = NONE;
                else
                    Menu = ESCAPE;
            }
            else if (eventV.key.keysym.scancode == SDL_SCANCODE_D || eventV.key.keysym.scancode == SDL_SCANCODE_RIGHT) {//DROITE
                LastMove.x = 1;
            }
            else if (eventV.key.keysym.scancode == SDL_SCANCODE_A || eventV.key.keysym.scancode == SDL_SCANCODE_LEFT) {//Gauche
                LastMove.x = 2;
            }
            else if (eventV.key.keysym.scancode == SDL_SCANCODE_S || eventV.key.keysym.scancode == SDL_SCANCODE_DOWN) {//BAS
                LastMove.y = 3;
            }
            else if (eventV.key.keysym.scancode == SDL_SCANCODE_W || eventV.key.keysym.scancode == SDL_SCANCODE_UP) {//Haut
                LastMove.y = 4;
            }
            

            else if (eventV.key.keysym.sym == SDLK_0 || eventV.key.keysym.sym == SDLK_KP_0) {
                SetAsAction(PLANT);
            }
            else if (eventV.key.keysym.sym == SDLK_1 || eventV.key.keysym.sym == SDLK_KP_1) {
                SetAsAction(RAIN);
            }
            else if (eventV.key.keysym.sym == SDLK_2 || eventV.key.keysym.sym == SDLK_KP_2) {
                SetAsAction(COLD);
            }
            else if (eventV.key.keysym.sym == SDLK_3 || eventV.key.keysym.sym == SDLK_KP_3) {
                SetAsAction(METEOR);
            }
            else if (eventV.key.keysym.sym == SDLK_4 || eventV.key.keysym.sym == SDLK_KP_4) {
                SetAsAction(DEVOUR);
            }
            else if (eventV.key.keysym.sym == SDLK_5 || eventV.key.keysym.sym == SDLK_KP_5) {
                SetAsAction(DROWN);
            }

            break;
        }
        case SDL_KEYUP: {
            if (eventV.key.keysym.scancode == SDL_SCANCODE_D || eventV.key.keysym.scancode == SDL_SCANCODE_RIGHT) {//DROITE
                LastMove.x = 0;
            }
            else if (eventV.key.keysym.scancode == SDL_SCANCODE_A || eventV.key.keysym.scancode == SDL_SCANCODE_LEFT) {//Gauche
                LastMove.x = 0;
            }
            else if (eventV.key.keysym.scancode == SDL_SCANCODE_S || eventV.key.keysym.scancode == SDL_SCANCODE_DOWN) {//BAS
                LastMove.y = 0;
            }
            else if (eventV.key.keysym.scancode == SDL_SCANCODE_W || eventV.key.keysym.scancode == SDL_SCANCODE_UP) {//Haut
                LastMove.y = 0;
            }
            else if (eventV.key.keysym.sym == SDLK_KP_PLUS) {
                timeTurn = timeTurn / 2;
                if (timeTurn < 250)
                    timeTurn = 250;
            }
            else if (eventV.key.keysym.sym == SDLK_KP_MINUS) {
                timeTurn = timeTurn * 2;
                if (timeTurn > 2000)
                    timeTurn = 2000;
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            if (eventV.button.button == SDL_BUTTON_LEFT) {
                if (Menu == ESCAPE) {

                }
                else {
                    if (eventV.button.y < 100 * Zoom && eventV.button.x < 270 * Zoom) {
                        if (eventV.button.x < 100 * Zoom) {
                            //Speeddown
                            timeTurn = timeTurn * 2;
                            if (timeTurn > 2000)
                                timeTurn = 2000;
                        }
                        else if (eventV.button.x > 190 * Zoom) {
                            //Speeddown
                            timeTurn = timeTurn / 2;
                            if (timeTurn < 250)
                                timeTurn = 250;
                        }
                    }
                    else if (eventV.button.y > 1900 * Zoom && eventV.button.y < 2100 * Zoom) {
                        for (int i = 0;i < 7;i++) {
                            if (eventV.button.x > 200 * Zoom + i * 220 * Zoom && eventV.button.x < 400 * Zoom + i * 220 * Zoom) {
                                SetAsAction(Actions(i));
                                break;
                            }
                        }
                    }
                    else if (Menu == NONE) {
                        if (eventV.button.y < 100 * Zoom) {
                            Menu = UIUP;
                        }
                    }
                    else if (Menu == UIUP) {
                        if (eventV.button.y < 300 * Zoom) {
                            Menu = NONE;
                        }
                    }
                }
            }
        }
        default:
            break;
        }
	}//fin while event
    if(LastMove.x==1)
        posxy.x += arrond(LCASE * Zoom);
    else if (LastMove.x == 2)
        posxy.x -= arrond(LCASE * Zoom);
    if (LastMove.y == 3)
        posxy.y += arrond(LCASE * Zoom);
    else if (LastMove.y == 4)
        posxy.y -= arrond(LCASE * Zoom);
    if (posxy.y > 2000 * Zoom)
        posxy.y = 2000 * Zoom;
    if (posxy.x > 3000 * Zoom)
        posxy.x = 3000 * Zoom;
    if (posxy.x < -2000 * Zoom)
        posxy.x = -2000 * Zoom;
    if (posxy.y < -2000 * Zoom)
        posxy.y = -2000 * Zoom;
}