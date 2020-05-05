#ifndef __SPRITE_MANAGER_H__ 
#define __SPRITE_MANAGER_H__ 
#include "common.h"

// Public Sprite Manager data and types
#define SM_BACKGROUND 1
#define SM_SPRITE     2


// Typedefs for functions used to update a specific type of sprite's position
typedef int (*UpdateSpritePositionFunction) (void *s, float mb);
typedef struct BASIC_SPRITE_STRUCT
{
  // ************************* Linked List Structure **************************
  // First 4 elements of sprite structure must be these 4 or Draw List 
  // Class will not work correctly
  void *prev;
  void *next;
  float zPos;
  MM_DrawImageFunction DrawImage;
  // **************************************************************************
  
  SDL_Surface    *img;
  unsigned char  numImages;
  unsigned char  type;
  unsigned char  free;
  unsigned char  active;
  unsigned char  show;
  unsigned short h;
  unsigned short w;
  short xPosTmp;
  float xPos;
  short yPos;
  unsigned short xDel;
  unsigned short yDel;
  unsigned short fDel;
  unsigned short cDel;
  unsigned short xDelCur;
  unsigned short yDelCur;
  unsigned short fDelCur;
  unsigned short cDelCur;
  
  unsigned char  weaponInUse;
  unsigned char  isMoving;
  unsigned char  isJumping;
  float xVel;
  short yVel;
  short yVelCur;
  unsigned char  gravity;
  unsigned short groundLevel;
  
  int            misc;
  unsigned char  collision;
  unsigned char  collisionDir;
  short          collisionVal;
  unsigned char  collisionHero;
  unsigned short curFrm;
  unsigned char  curDir;
  unsigned char  frmCount;
  short          frmIndex;
  unsigned char  frmOrder[2][20];
  
  // function used to update sprite's position
  UpdateSpritePositionFunction UpdateSpritePosition;
  
  SDL_Rect srcRec, dstRec, boundRec, wBoundRec;
  
} Sprite;


// Public Sprite Manager Functions
void SM_Init();
int  SM_InitLevel(unsigned int level);
int  SM_CreateSprite(float xPos, int yPos, float zPos, int id, int type, void *setup);
int  SM_UpdateSpritePositions(float moveBg);
int  SM_DrawSprites(void *s);
int  SM_DetectCollision();
void SM_EnableSprite(int eId);
void SM_DisableSprite(int eId);
void SM_DestroySprite(Sprite *s);
int  SM_AdjustHeroPosition(int yPos, int hw, SDL_Rect *hBr, float *xPos, float *moveBg);
void SM_ShowBlinkSprites();

// Functions used to create "Special" sprites
void SM_CreateRandomSprite();                         // used in hero_manager
void SM_CreateLevelCompleteSprite(unsigned int frm);  // used in hero_manager
void SM_CreateScreenshotSprite(unsigned int id);      // used in main
void SM_DestroyScreenShotText();                      // used in main
void SM_ClearList();


#endif

