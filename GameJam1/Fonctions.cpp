
#include "Fonctions.h"
#include "Pixel.h"

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
	*wText = arrond(*wText * Zoom7K);
	*hText = arrond(*hText * Zoom7K);
}
void QueryText4(SDL_Texture* Texture, int* wText, int* hText) {
	//Get sizes and zoom it for 4K
	SDL_QueryTexture(Texture, NULL, NULL, wText, hText);
	*wText = arrond(*wText * Zoom);
	*hText = arrond(*hText * Zoom);
}
void QueryText2(SDL_Texture* Texture, int* wText, int* hText) {
	//Get sizes and zoom it for 2K
	SDL_QueryTexture(Texture, NULL, NULL, wText, hText);
	*wText = arrond(*wText * Zoom2K);
	*hText = arrond(*hText * Zoom2K);
}