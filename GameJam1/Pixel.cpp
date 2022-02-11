
#include <stdio.h>
#include <SDL_image.h>
#include "main.h"


int arrond(float val){
    if(val<0)
        return (int)(val-0.5);
    return (int)(val+0.5);
}

Uint32 GetPixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    return *(Uint32*)p;
}

void SDL_PutPixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8 *p = (Uint8*)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32*)p = pixel;
}
void SDL_PutPixel8(SDL_Surface* surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    //change un pixel rgb d'une surface
    int coltemp1 = SDL_MapRGBA(surface->format, r, g, b,a);
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32*)p = coltemp1;
}
int GetRGBPixel(SDL_Surface* s, int x, int y, uint8_t* r, uint8_t* g, uint8_t* b){
   if (!(x >= 0 && y >= 0 && x < s->w && y < s->h))
     return 0;
   SDL_LockSurface(s);

   /*const uint8_t * const pt = (uint8_t*)s->pixels;
   const uint8_t bpp = s->format->BytesPerPixel;

   SDL_GetRGB((*(uint32_t*)&(pt[(y * s->w + x) * bpp])) & ((0xFFFFFFFF << bpp) >> bpp),
              s->format, r, g, b);*/
    SDL_GetRGB(GetPixel(s, x, y),s->format, r, g, b);
    
   SDL_UnlockSurface(s);


   return 1;
 }
int GetRGBAPixel(SDL_Surface* s, int x, int y, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    if (!(x >= 0 && y >= 0 && x < s->w && y < s->h))
        return 0;
    SDL_LockSurface(s);

    /*const uint8_t * const pt = (uint8_t*)s->pixels;
    const uint8_t bpp = s->format->BytesPerPixel;

    SDL_GetRGB((*(uint32_t*)&(pt[(y * s->w + x) * bpp])) & ((0xFFFFFFFF << bpp) >> bpp),
               s->format, r, g, b);*/
    SDL_GetRGBA(GetPixel(s, x, y), s->format, r, g, b, a);

    SDL_UnlockSurface(s);


    return 1;
}

void MysetPixel(SDL_Surface *surface, int x, int y) //changer la couleur d'un px
{
    //nbOctetsParPixel représente le nombre d'octets utilisés pour stocker un pixel.
    //En multipliant ce nombre d'octets par 8 (un octet = 8 bits), on obtient la profondeur de couleur
    //de l'image : 8, 16, 24 ou 32 bits.
    /* Mettre ça dans affichage pour dégrader tout le texte
    for (i = 0;i < ttfTchat->w;i++)
            for (j = 0;j < ttfTchat->h;j++)
               MysetPixel(ttfTchat, i, j); */
    int nbOctetsParPixel = surface->format->BytesPerPixel;
    //Ici p est l'adresse du pixel que l'on veut modifier
    //surface->pixels contient l'adresse du premier pixel de l'image
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * nbOctetsParPixel;
    Uint8 r,  g,  b,  a;
    Uint8 rb, gb, bb, ab;
    int varr, varg, varb, vara; //obligatoire car les chiffres de uint8 ne vont que de 0 à 255
   
    SDL_GetRGBA(GetPixel(surface, x, y), surface->format, &r, &g, &b, &a); //r,g,b,a
    varr = r;
    varg = g;
    varb = b;
    vara = a;
    if (y > 0) {
        SDL_GetRGBA(GetPixel(surface, x, y-1), surface->format, &rb, &gb, &bb, &ab); //dégradé linéaire en prenant le pixel précédent
        //on change la couleur ici
        //varr = rb - r/30;
        //varg = gb - g/30; //degrade
        //varb = bb - b/30;
        r = varr;
        g = varg;
        b = varb;
        a = vara;
    }

    
    if (varr > 255)
        r = 255;
    if (varg > 255)
        g = 255;
    if (varb > 255)
        b = 255;
    if (vara > 255)
        a = 255;
    if (varr < 0)
        r = 0;
    if (varg < 0)
        g = 0;
    if (varb < 0)
        b = 0;
    if (vara < 0)
        a = 0;
    Uint32 pixel=SDL_MapRGBA(surface->format, r, g, b, a);
    //Gestion différente suivant le nombre d'octets par pixel de l'image
    switch(nbOctetsParPixel)
    {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            //Suivant l'architecture de la machine
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
}
void MonDegrade1(SDL_Surface* surface, int x, int y, SDL_Color Couleur, SDL_Color Degrade ) //changer la couleur d'un px
{
    //Utilisé à l'initialisation seulement pour pas perdre de FPS (1 seule fois par chiffre)
    int nbOctetsParPixel = surface->format->BytesPerPixel;
    //Ici p est l'adresse du pixel que l'on veut modifier
    //surface->pixels contient l'adresse du premier pixel de l'image
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * nbOctetsParPixel;
    Uint8 r, g, b, a;

    SDL_GetRGBA(GetPixel(surface, x, y), surface->format, &r, &g, &b, &a); //r,g,b,a
    //on change la couleur ici
    r = Degrade.r + (Couleur.r - Degrade.r) * y / surface->h;
    g = Degrade.g + (Couleur.g - Degrade.g) * y / surface->h;
    b = Degrade.b + (Couleur.b - Degrade.b) * y / surface->h;
    if (r > 255)
        r = 255;
    if (g > 255)
        g = 255;
    if (b > 255)
        b = 255;
    if (a > 255)
        a = 255;
    if (r < 0)
        r = 0;
    if (g < 0)
        g = 0;
    if (b < 0)
        b = 0;
    if (a < 0)
        a = 0;
    Uint32 pixel = SDL_MapRGBA(surface->format, r, g, b, a);
    //Gestion différente suivant le nombre d'octets par pixel de l'image
    switch (nbOctetsParPixel)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        //Suivant l'architecture de la machine
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;
    }
}

void OmbrePortee(SDL_Surface* surface, int x, int y) //changer la couleur d'un px
{
    //nbOctetsParPixel représente le nombre d'octets utilisés pour stocker un pixel.
    //En multipliant ce nombre d'octets par 8 (un octet = 8 bits), on obtient la profondeur de couleur
    //de l'image : 8, 16, 24 ou 32 bits.
    int nbOctetsParPixel = surface->format->BytesPerPixel;
    /* Mettre ça dans affichage pour avoir l'ombre sur tout le texte
    for (i = 0;i < ttfTchat->w;i++)
            for (j = 0;j < ttfTchat->h;j++)
               OmbrePortee(ttfTchat, i, j); */

    //Ici p est l'adresse du pixel que l'on veut modifier
    //surface->pixels contient l'adresse du premier pixel de l'image

    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * nbOctetsParPixel;
    Uint8 r, g, b, a;
    Uint8 rb, gb, bb, ab; //rgba des pixels autour
    int varr, varg, varb, vara; //obligatoire car les chiffres de uint8 ne vont que de 0 à 255

        
    SDL_GetRGBA(GetPixel(surface, x, y), surface->format, &r, &g, &b, &a); //r,g,b,a
    varr = r;
    varg = g;
    varb = b;
    vara = a;
    int i, j;
    int cptmoy = 4; //diviseur  (Monter le chiffre pour éclaircir l'ombre)
    int sommepx = 0; //somme des alpha des 25 pixel divisé par cptmoy
    //moyenne des 9 cases
    for (i = x - arrond(8*Zoom); i <= x + arrond(8 * Zoom);i++) {
        for (j = y - arrond(8 * Zoom);j <= y + arrond(8 * Zoom);j++) {
            if ((i  > surface->w || i < 0) || (j > surface->h || j < 0)) {
                cptmoy+=1;
                
                //sommepx = 0;
            }
            else {
                SDL_GetRGBA(GetPixel(surface, i, j), surface->format, &rb, &gb, &bb, &ab); //dégradé linéaire en prenant le pixel précédent
                //on change la couleur ici
                sommepx += ab;
                cptmoy+=1;
            }
        }
    }
    
    vara = sommepx / cptmoy;
    if (vara > 255)
        vara = 255;
    varr = 0; //changer couleur de l'ombre ici
    varg = 0;
    varb = 0;
    r = varr;
    g = varg;
    b = varb;
    a = vara;

    if (varr > 255)
        r = 255;
    if (varg > 255)
        g = 255;
    if (varb > 255)
        b = 255;
    if (vara > 255)
        a = 255;
    if (varr < 0)
        r = 0;
    if (varg < 0)
        g = 0;
    if (varb < 0)
        b = 0;
    if (vara < 0)
        a = 0;
    Uint32 pixel = SDL_MapRGBA(surface->format, r, g, b, a);
    //Gestion différente suivant le nombre d'octets par pixel de l'image
    switch (nbOctetsParPixel)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        //Suivant l'architecture de la machine
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;
    }
}

void FonctionAdd(SDL_Surface* surface, int x, int y, SDL_Surface * surfadd) //petit antialiasing
{
    //nbOctetsParPixel représente le nombre d'octets utilisés pour stocker un pixel.
    //En multipliant ce nombre d'octets par 8 (un octet = 8 bits), on obtient la profondeur de couleur
    //de l'image : 8, 16, 24 ou 32 bits.
    int nbOctetsParPixel = surface->format->BytesPerPixel;

    //Ici p est l'adresse du pixel que l'on veut modifier
    //surface->pixels contient l'adresse du premier pixel de l'image
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * nbOctetsParPixel;
    Uint8 r, g, b, a;
    Uint8 rb=0, gb=0, bb=0; //rgba des pixels autour
    SDL_GetRGBA(GetPixel(surface, x, y), surface->format, &r, &g, &b, &a);
    //SDL_GetRGB(GetPixel(surfadd, x, y), surfadd->format, &rb, &gb, &bb);
    //r,g,b,a
    r = r+rb;
    g = g+gb;
    b = b+bb;
    if (r > 255)
        r = 255;
    if (g > 255)
        g = 255;
    if (b > 255)
        b = 255;
    if (r < 0)
        r = 0;
    if (g < 0)
        g = 0;
    if (b < 0)
        b = 0;
    Uint32 pixel = SDL_MapRGBA(surface->format, r, g, b, a);
    //Gestion différente suivant le nombre d'octets par pixel de l'image
    switch (nbOctetsParPixel)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        //Suivant l'architecture de la machine
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;
    }
}

void Flou1(SDL_Surface* surface, int x, int y) //petit antialiasing
{
    //nbOctetsParPixel représente le nombre d'octets utilisés pour stocker un pixel.
    //En multipliant ce nombre d'octets par 8 (un octet = 8 bits), on obtient la profondeur de couleur
    //de l'image : 8, 16, 24 ou 32 bits.
    int nbOctetsParPixel = surface->format->BytesPerPixel;

               //Ici p est l'adresse du pixel que l'on veut modifier
               //surface->pixels contient l'adresse du premier pixel de l'image

    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * nbOctetsParPixel;
    Uint8 r, g, b, a;
    int varr, varg, varb, vara; //obligatoire car les chiffres de uint8 ne vont que de 0 à 255

    SDL_GetRGBA(GetPixel(surface, x, y), surface->format, &r, &g, &b, &a);
     //r,g,b,a
    varr = r*10;
    varg = g*10;
    varb = b*10;
    vara = a*10;
    int i, j;
    int cptmoy = 10; //diviseur  (Monter le chiffre pour éclaircir l'ombre)
    int sommepx = 0; //somme des alpha des 25 pixel divisé par cptmoy
    //moyenne des 9 cases
    for (i = x; i <= x + 1;i++) {
        for (j = y ;j <= y+1 ;j++) {
            if (a==0 || (i + 1 > surface->w || i < 0) || (j + 1 > surface->h || j < 0)) {
                cptmoy += 1;
                //sommepx = 0;
            }
            else {
                cptmoy += 1;
                SDL_GetRGBA(GetPixel(surface, i, j), surface->format, &r, &g, &b, &a);
                vara += a;
                varr += r;
                varg += g;
                varb += b;
            }
        }
    }


    vara = vara / cptmoy;
    if (vara > 255)
        vara = 255;
    varr = varr / cptmoy; //changer couleur de l'ombre ici
    varg = varg / cptmoy;
    varb = varb / cptmoy;
    r = varr;
    g = varg;
    b = varb;
    a = vara;
    if (varr > 255)
        r = 255;
    if (varg > 255)
        g = 255;
    if (varb > 255)
        b = 255;
    if (vara > 255)
        a = 255;
    if (varr < 0)
        r = 0;
    if (varg < 0)
        g = 0;
    if (varb < 0)
        b = 0;
    if (vara < 0)
        a = 0;
    Uint32 pixel = SDL_MapRGBA(surface->format, r, g, b, a);
    //Gestion différente suivant le nombre d'octets par pixel de l'image
    switch (nbOctetsParPixel)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        //Suivant l'architecture de la machine
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;
    }
}


int ChangeColorpx(SDL_Surface* surface, int debut, int fin, int perso) { 
    //Change la couleur des pixel en fonction de la palette de couleur debut et fin : Addition
    Uint8 r, g, b, a;
    int i = 0, j = 0;
    for (i = 0;i < surface->w;i++) {
        for (j = 0;j < surface->h;j++) {
            SDL_GetRGBA(GetPixel(surface, i, j), surface->format, &r, &g, &b, &a);
            if (b>25 && r<50 && a>10) {  //76 59 48 vs 28 30 33
                //r60 b50
                //(b>50 && r>42 && r <50 && g>40 && a>10)
                Uint8 maxrgb = r;
                Uint8 minrgb = r;
                
                if (g > maxrgb)
                    maxrgb = g;
                if (g > maxrgb)
                    maxrgb = b;
                if(b < minrgb)
                    minrgb = b;
                if (g < minrgb)
                    minrgb = g;
                
                if (perso == 1) {  //Bleu claire
                    g = b;
                    r = b;
                    b = b;
                }
                else if (perso == 2) {  //orange
                    g = static_cast<Uint8>(g * 1.8);
                    Uint8 vartmp = r;
                    r = (b * 2);
                    b = vartmp;
                }
                else if (perso == 4) {  //Violet
                    Uint8 vartmp = g;
                    g = r;
                    r = static_cast<Uint8>(vartmp * 1.2);
                    b = b;
                }
                else if (perso == 3) {  //Rose
                    g = r;
                    r = static_cast<Uint8>(b * 1.5);
                    b = r;
                }
                else if (perso == 5) {  //Rouge
                    g = r;
                    r = static_cast<Uint8>(b * 2);
                    b = g;
                }
                else if (perso == 8) {  //Vert
                    g = (g * 2);
                    r = b;
                    b = b;
                }
                else if (perso == 9) { //gris
                    g = static_cast<Uint8>((g + r + b) / 3 * 1.5);
                    r = g;
                    b = g;
                }
                else if (perso == 10) { //blanc
                    g = (unsigned char)(((g + r + b) / 3) * 3.5f);
                    r = g;
                    b = g;
                }

                if (r > 255)
                    r = 255;
                if (g > 255)
                    g = 255;
                if (b > 255)
                    b = 255;
                if (g < 0)
                    g = 0;
                int nbOctetsParPixel = surface->format->BytesPerPixel;
                Uint8* p = (Uint8*)surface->pixels + j * surface->pitch + i * nbOctetsParPixel;
                Uint32 pixel = SDL_MapRGBA(surface->format, r, g, b, a);
                //Gestion différente suivant le nombre d'octets par pixel de l'image
                switch (nbOctetsParPixel)
                {
                case 1:
                    *p = pixel;
                    break;

                case 2:
                    *(Uint16*)p = pixel;
                    break;

                case 3:
                    //Suivant l'architecture de la machine
                    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                    {
                        p[0] = (pixel >> 16) & 0xff;
                        p[1] = (pixel >> 8) & 0xff;
                        p[2] = pixel & 0xff;
                    }
                    else
                    {
                        p[0] = pixel & 0xff;
                        p[1] = (pixel >> 8) & 0xff;
                        p[2] = (pixel >> 16) & 0xff;
                    }
                    break;

                case 4:
                    *(Uint32*)p = pixel;
                    break;
                }
            }
        }
    }
    return 0;
}

void MySetPixel2(SDL_Texture* texture,int x,int y) {
    Uint32* pixels = NULL;
    int pitch = 0;
    Uint32 format;

    // Get the size of the texture.
    int w, h;
    SDL_QueryTexture(texture, &format, NULL, &w, &h);

    // Now let's make our "pixels" pointer point to the texture data.
    if (SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch))
    {
        // If the locking fails, you might want to handle it somehow. SDL_GetError(); or something here.
    }

    SDL_PixelFormat pixelFormat = {};
    pixelFormat.format = format;
    // Now you want to format the color to a correct format that SDL can use.
    // Basically we convert our RGB color to a hex-like BGR color.
    Uint32 color = SDL_MapRGB(&pixelFormat, 50, 50,50);

    // Before setting the color, we need to know where we have to place it.
    Uint32 pixelPosition = y * (pitch / sizeof(unsigned int)) + x;

    // Now we can set the pixel(s) we want.
    pixels[pixelPosition] = color;

    // Also don't forget to unlock your texture once you're done.
    SDL_UnlockTexture(texture);
}

void setPixelVerif(int x, int y, Uint8 r,Uint8 g,Uint8 b)
{
  if (x >= 0 && x < ScreenL.x && y >= 0 && y < ScreenL.y){
    SDL_RenderDrawPoint(Renderer, x, y);
    SDL_SetRenderDrawColor(Renderer,r,g,b, 255);
  }
}

SDL_Surface* uint8ToSurface(Uint8* image, int largeur, int hauteur) {
    SDL_Surface* surface;
    Uint32 rmask, gmask, bmask, amask;
    SDL_Rect pixel = {};

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    surface = SDL_CreateRGBSurface(0, largeur, hauteur, 32, rmask, gmask, bmask, amask);
    if (surface == NULL) {
        SDL_Log("SDL_CreateRGBSurface() failed: %s", SDL_GetError());
        return NULL;
    }

    pixel.w = 1;
    pixel.h = 1;
    //pixel.x = 0; pixel.x < largeur; ++pixel.x
    //pixel.y = hauteur-1; pixel.y >= 0 ; --pixel.y
    for (pixel.y = 0; pixel.y < hauteur; ++pixel.y) {
        for (pixel.x = 0; pixel.x < largeur ; ++pixel.x) {
            SDL_FillRect(surface, &pixel, SDL_MapRGBA(surface->format, image[0], image[1], image[2], image[3]));
            image += 4; // On passe au pixel suivant
        }
    }

    return surface;
}

// On renvoie l'image miroir par rapport à un axe de symétrie vertical
SDL_Surface* VerticalMirror(SDL_Surface* image)
{
    SDL_Surface* surface = NULL;
    Uint8 r, g, b, a;
    int i = 0, j = 0;
    SDL_Rect line1 = {}, line2 = {};
    line1.y = 0 ;
    line1.w = 1 ;
    line2.y = 0 ;
    line2.w = 1 ;
    //Création d'une surface vide de la taille de l'image à copier
    surface = SDL_CreateRGBSurface(image->flags,image->w,image->h,32,image->format->Rmask,image->format->Gmask,image->format->Bmask,image->format->Amask);
    /*for (i=0;i<surface->w;i++) //old qui marchait pas sur la transparence
    {
        //On copie successivement un trait de l'image de départ qu'on recopie de l'autre coté de l'image d'arrivée
        line1.x = i ;
        line1.h = surface->h ;
        line2.x = surface->w - i - 1 ;
        line2.h = surface->h ;
        SDL_BlitSurface(image, &line1, surface, &line2);
    }*/
    for (i = 0;i < surface->w;i++) {
        for (j = 0;j < surface->h;j++) {
            SDL_GetRGBA(GetPixel(image, image->w-i-1, j), image->format, &r, &g, &b, &a);

            int nbOctetsParPixel = surface->format->BytesPerPixel;
            Uint8* p = (Uint8*)surface->pixels + j * surface->pitch + i * nbOctetsParPixel;
            Uint32 pixel = SDL_MapRGBA(surface->format, r, g, b, a);
            //Gestion différente suivant le nombre d'octets par pixel de l'surface
            switch (nbOctetsParPixel)
            {
            case 1:
                *p = pixel;
                break;

            case 2:
                *(Uint16*)p = pixel;
                break;

            case 3:
                //Suivant l'architecture de la machine
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                {
                    p[0] = (pixel >> 16) & 0xff;
                    p[1] = (pixel >> 8) & 0xff;
                    p[2] = pixel & 0xff;
                }
                else
                {
                    p[0] = pixel & 0xff;
                    p[1] = (pixel >> 8) & 0xff;
                    p[2] = (pixel >> 16) & 0xff;
                }
                break;

            case 4:
                *(Uint32*)p = pixel;
                break;
            }
        }
    }
    
    return surface;
}
