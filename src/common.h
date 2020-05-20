#ifndef __COMMON_H__
#define __COMMON_H__
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "zip_manager.h"
#include "eh_manager.h"

typedef int (*MM_DrawImageFunction) (void *);


#define MM_SCREEN_WIDTH        480
#define MM_SCREEN_HEIGHT       272

#define MM_EAST                  1
#define MM_WEST                  2

#define MM_LEVEL1                1
#define MM_LEVEL2                2
#define MM_LEVEL3                4
#define MM_LEVEL4                8
#define MM_LEVEL_FINAL          16
#define MM_LEVEL_HIDDEN1        32
#define MM_LEVEL_CREDITS       128

#define MM_STATE_EXIT            0
#define MM_STATE_INITIALIZE      1
#define MM_STATE_RUNNING         2
#define MM_STATE_GAME_OVER       3
#define MM_STATE_MAIN_MENU       4
#define MM_STATE_LEVEL_COMPLETE  5

#define MM_BACK_BUFFER           1
#define MM_DRAW_BUFFER           2

float        MM_GetFreeRam();
void*        MM_GetScreenBuffer(unsigned int buffer);
int          MM_RandomNumberGen(int lower, int upper);
void         MM_SetGameState(unsigned int state);
SDL_Surface* MM_GetScreenPtr();
void         MM_PressAnyKeyToContinue(SDL_Event *event);
int          MM_Abs(int val);
void         MM_TakeMenuScreenshot();

//  Button Values
//  0 Triangle
//  1 Circle
//  2 Cross
//  3 Square
//  4 Left trigger
//  5 Right trigger
//  6 Down
//  7 Left
//  8 Up
//  9 Right
// 10 Select
// 11 Start
// 12 Home
// 13 Hold

#endif
