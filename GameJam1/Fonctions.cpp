
#include "Fonctions.h"

SDL_Point ToIso(SDL_Point point) {
	//Normal pos to isometric pos
	SDL_Point pointIso;
	pointIso.x = point.x - point.y;
	pointIso.y = (point.x + point.y) / 2;
	return(pointIso);
}

void QueryText(SDL_Texture* Texture, int* wText, int* hText) {
	//Get sizes and zoom it
	SDL_QueryTexture(Texture, NULL, NULL, wText, hText);
	*wText = *wText * Zoom;
	*hText = *hText * Zoom;
}