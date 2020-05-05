#ifndef __MENU_MANAGER_H__
#define __MENU_MANAGER_H__
#include "common.h"


// Public
void         MUNU_Init(char *);
void         MENU_DrawIntro(SDL_Event *event);
unsigned int MENU_DrawGameOver(SDL_Event *event);
unsigned int MENU_DrawPauseGame(SDL_Event *event, unsigned int gameState);
void         MENU_DrawViewScreenshots(SDL_Event *event);
unsigned int MENU_DrawEnterCode(SDL_Event *event, unsigned int gameLevel);
unsigned int MENU_DrawMain(SDL_Event *event, unsigned int gameLevel, Mix_Chunk *ding);
unsigned int MENU_DrawOptions(SDL_Event *event, SDL_Surface *startScreenImg, SDL_Surface *cursorImg, unsigned int gameLevel, Mix_Chunk *select, Mix_Chunk *ding);
void         MENU_DrawLoadScreen();
unsigned int MENU_DrawCredits(SDL_Event *event);
unsigned int MENU_DrawFinalLevel(SDL_Event *event);
unsigned int MENU_DrawHiddenLevel1(SDL_Event *event);

#endif
