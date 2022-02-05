
#ifndef PIXEL_H_
#define PIXEL_H_

#include "main.h"


int arrond(float val);
Uint32 GetPixel(SDL_Surface *s, int x, int y);
void SDL_PutPixel32(SDL_Surface *surface, int x, int y, Uint32 pixel);
void SDL_PutPixel8(SDL_Surface* surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int GetRGBPixel(SDL_Surface* s, int x, int y, uint8_t* r, uint8_t* g, uint8_t* b);
int GetRGBAPixel(SDL_Surface* s, int x, int y, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
void MysetPixel(SDL_Surface *surface, int x, int y);
void MonDegrade1(SDL_Surface* surface, int x, int y, SDL_Color Couleur, SDL_Color Degrade);
void OmbrePortee(SDL_Surface* surface, int x, int y);
void FonctionAdd(SDL_Surface* surface, int x, int y, SDL_Surface* surfadd);
void Flou1(SDL_Surface* surface, int x, int y);
int ChangeColorpx(SDL_Surface* surface, int debut, int fin,int perso);
void MySetPixel2(SDL_Texture *Texture, int x, int y);
void setPixelVerif(int x, int y, Uint8 r,Uint8 g,Uint8 b);
SDL_Surface* VerticalMirror(SDL_Surface* image);


#endif
