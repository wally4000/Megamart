#ifndef __HERO_MANAGER_H__
#define __HERO_MANAGER_H__
#include "common.h"

// Public Functions
void  HM_Init();
int   HM_InitLevel(int level);
float HM_UpdateHeroPosition();
float HM_GetXPos();

// Functions to control Hero Movement
void  HM_ShowHero();
void  HM_MoveLeft();
void  HM_MoveRight();
void  HM_StopMoving();
void  HM_StartRunning();          
void  HM_StopRunning();          
void  HM_Jump();
void  HM_UseWeapon();
void  HM_Duck();
void  HM_StopDuck();
void  HM_SetCollision(short val, unsigned char dir);
int   HM_GetCurrentDir();
void  HM_HeroInvulnerable();
void  HM_GetCollisionInfo(int *hXPos, int *hYPos, int *heroDir, 
                          SDL_Rect *hBoundRec, 
                          int *weaponInUse, SDL_Rect *hWBoundRec);


#endif

