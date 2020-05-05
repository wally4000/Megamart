#ifndef __POWER_MANAGER_H__
#define __POWER_MANAGER_H__
#include "common.h"

void PM_Init();
int  PM_InitLevel(unsigned int level);
void PM_AdjustPower(short value, int *retPower, int *retLives);
void PM_AdjustLives(int count);
int  PM_DrawMeeter();
void PM_ResetHealth();
void PM_ActivateInfinityLives();
void PM_DisableDrawing();

#endif
