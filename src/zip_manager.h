#ifndef __ZIP_MANAGER_H__
#define __ZIP_MANAGER_H__
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"

#define ZIP_MAIN     1
#define ZIP_LEVEL1   2

#define ZIP_FONT1    "free_sans.ttf"
#define ZIP_FONT2    "oposs.ttf"

typedef struct ZIP_Font
{
  TTF_Font *f;
  void     *d;
} ZIP_Font;



SDL_Surface *ZIP_LoadImage(const char *);
Mix_Chunk   *ZIP_LoadMusic(const char *name);
ZIP_Font    *ZIP_LoadFont(const char *name, int ptSize);
void        ZIP_CloseFont(ZIP_Font *z);
int         ZIP_OpenZipFile(unsigned int);
int         ZIP_CloseZipFile();

#endif
