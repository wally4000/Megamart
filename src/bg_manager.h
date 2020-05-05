#ifndef __BG_MANAGER_H__
#define __BG_MANAGER_H__
#include "common.h"

// Public data & functions from Background Manager Class


#define HERO_MIDPOINT_EAST 200
#define HERO_MIDPOINT_WEST 140


void  BG_Init();
int   BG_InitLevel(unsigned int level);
int   BG_UpdatePosition(float dir);
int   BG_EndReached(float dir);
void  BG_DrawBackground();
int   BG_SetCeilingFloorSpeed(int isRunning);
float BG_GetxPosGlobal();
void  BG_SetXPosGlobal(float x);




#endif

