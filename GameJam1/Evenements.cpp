



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
                break;
            }
            if (eventV.key.keysym.scancode == SDL_SCANCODE_A || eventV.key.keysym.scancode == SDL_SCANCODE_LEFT) {//Gauche
                LastMove.x = 2;
                break;
            }
            if (eventV.key.keysym.scancode == SDL_SCANCODE_S || eventV.key.keysym.scancode == SDL_SCANCODE_DOWN) {//BAS
                LastMove.y = 3;
                break;
            }
            if (eventV.key.keysym.scancode == SDL_SCANCODE_W || eventV.key.keysym.scancode == SDL_SCANCODE_UP) {//Haut
                LastMove.y = 4;
                break;
            }
            break;
        }
        case SDL_KEYUP: {
            if (eventV.key.keysym.scancode == SDL_SCANCODE_D || eventV.key.keysym.scancode == SDL_SCANCODE_RIGHT) {//DROITE
                LastMove.x = 0;
                break;
            }
            if (eventV.key.keysym.scancode == SDL_SCANCODE_A || eventV.key.keysym.scancode == SDL_SCANCODE_LEFT) {//Gauche
                LastMove.x = 0;
                break;
            }
            if (eventV.key.keysym.scancode == SDL_SCANCODE_S || eventV.key.keysym.scancode == SDL_SCANCODE_DOWN) {//BAS
                LastMove.y = 0;
                break;
            }
            if (eventV.key.keysym.scancode == SDL_SCANCODE_W || eventV.key.keysym.scancode == SDL_SCANCODE_UP) {//Haut
                LastMove.y = 0;
                break;
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