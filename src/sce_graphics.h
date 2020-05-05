#ifndef __SCE_GRAPHICS_H__
#define __SCE_GRAPHICS_H__
#include "common.h" 
#include <pspgu.h>

unsigned int __attribute__((aligned(16))) _SCEBgList[4096];


SDL_Surface* SCE_LoadVramImage(const char *file, unsigned int format, unsigned int flags);
SDL_Surface* SCE_LoadBackground2(const char *file);
SDL_Surface* SCE_LoadBackground1(const char *file);
void         SCE_BlitSurface(SDL_Surface* l1, SDL_Rect *, SDL_Surface *, SDL_Rect*);
void         SCE_Init();

#define SCE_TRANSP_FORMAT   2
#define SCE_SCREEN_FORMAT   1
#define SCE_NATIVE_FORMAT   0

#endif
