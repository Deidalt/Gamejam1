



#include "Main.h"

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
            if (eventV.key.keysym.scancode == SDL_SCANCODE_D || eventV.key.keysym.scancode == SDL_SCANCODE_RIGHT) {//DROITE
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
            break;
        }
        default:
            break;
        }
	}//fin while event
    if(LastMove.x==1)
        posMap.x += static_cast<int>(LCASE * Zoom);
    else if (LastMove.x == 2)
        posMap.x -= static_cast<int>(LCASE * Zoom);
    if (LastMove.y == 3)
        posMap.y += static_cast<int>(LCASE * Zoom);
    else if (LastMove.y == 4)
        posMap.y -= static_cast<int>(LCASE * Zoom);
}