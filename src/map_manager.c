//-----------------------------------------------------------------------------
//  Class:
//  Map Manager
//
//  Description:
//  This class is used to create the layout of the main level.  It tracks
//  when each sprite/background object should be drawn on screen.
//-----------------------------------------------------------------------------

#include "map_manager.h"
#include "bg_manager.h"
#include "sprite_manager.h"
#include "resource_manager.h"

// Private Data Types & Constants
#define LEVEL_1_MAP_SIZE 30950

typedef struct LEVEL_MAP_STRUCT
{
  unsigned int   yPos;               // 4 
  float          zPos;               // 4 
  unsigned int   id;                 // 4
  unsigned short type;               // 2
  void           *sprInitInfo;       // 4
  short          enabled;            // 2
  void           *nextPtr;           // 4
} LevelMap;


// Private members
static SDL_Surface *_scr;
static LevelMap    **_map;
static int         _levelSize;

// Private functions
static void EnableObjectsPrivate( LevelMap *curPtr, int xPosAbs);
static int  InitLevelOne();
static int  FreeMemory();
static LevelMap* AddSpriteObject(int xPos, int yPos, float zPos, 
                                 int id, int type, void *setup);

// Private functions used to make creating instances of the specified
// sprite easier (they handle the dynamic memory allocation and random
// number generation for you)
static void AddSeriesSprite(int xPos, int id, int numSprites, int frmDel);
static void AddRandomSprite(int numSprites, int xStart, int xEnd);
static void AddTentSprite(int xPos, float zPos, int id, int type, int dir);
static void AddLBSprite(int xPos, int yPos);
static void AddBicycleSprite(int xPos, int yPos, float zPos, int id, 
                             int type, int dir);

//------------------------------------------------------------------------------
// Name:     MAP_Init
// Summary:  Called 1 time to initialize map class
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MAP_Init()
{
  _scr       = MM_GetScreenPtr();
  _levelSize = 0;
  _map       = 0;
}

//------------------------------------------------------------------------------
// Name:     MAP_InitLevel
// Summary:  Initializes the specified level.
// Inputs:   Level to initializes
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: Only 1 level present, but the framework exists for multiple 
//           levels
//------------------------------------------------------------------------------
int MAP_InitLevel(unsigned int level)
{
  int status = 0;
  FreeMemory();
 
  if (level == MM_LEVEL1)
  {
    _levelSize = MAP_GetLevelSize(level);
    status     = InitLevelOne();
  }
  else if (level == MM_LEVEL_CREDITS || level == MM_LEVEL_FINAL)
  {
    // do nothing, main purpose here was to free any memory in use by map
    status = 0; 
  }
  else
  {
    status = 1; 
  }
  return(status);  
}

//------------------------------------------------------------------------------
// Name:     InitLevelOne
// Summary:  Initializes Level 1, specifies what objects to draw on screen 
//           and where to draw them.
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int InitLevelOne()
{
  int xPos;
  int xPosOrig;
  int xPosNew;
  int id;
  int dir;
  int x;
  int w;
  int type;
  int rand;
  int extraLife;
  float zPos;

  _map = (LevelMap**) malloc(LEVEL_1_MAP_SIZE * sizeof(LevelMap*));
  
  for (x=0; x < _levelSize; x++)
  {
    _map[x] = 0;
  }

  
  //AddSpriteObject(200, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  
  //xPos += 500;
  
  // ---------- HOCKEY STICK / SHELF COMBO ----------
  xPos = 201;
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF);
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);

  // ---------- FALLING GRILLS ----------
  xPos += 100;
  w    = RM_GetImageWidth(GRILL_SPRITE);
  rand = MM_RandomNumberGen(0, 3);
  for (x=0; x < 9; x++)
  {
    AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, 
                    (((x+rand)%3)==0)?SM_SPRITE:SM_BACKGROUND,  0);
    xPos += w;
  }
 
  // ---------- SHELF SERIES / DECOY FALLING SHELF / RANDOM SPRITES ----------
  xPos    += 100;
  xPosOrig = xPos;
  w        = RM_GetImageWidth(GEN_RANDOM_SHELF) - 1;
  rand     = MM_RandomNumberGen(0, 4);
  
  // Create shelves  
  for (x=0; x < 5; x++)
  {
    // Falling shelf is wider by 34 pixels, so offset x location by this amount
    if (x==rand)  
      AddSpriteObject((xPos-34), 49, 6, FALLING_SHELF_SPRITE, SM_BACKGROUND, 0); 
    else
      AddSpriteObject(xPos, 63, 5,  GEN_RANDOM_SHELF, SM_BACKGROUND,  0);
    xPos += w;
  }
  
  // Dual Bicycle sprites or a series of random moving sprites that will
  // randomly appear by shelves (Lean towards bicycles)
  if (MM_RandomNumberGen(0, 3))
  {
    AddBicycleSprite(xPosOrig+350, 0, 9, RM_IMG_BICYCLE_SPRITE, 
                     SM_SPRITE, MM_WEST);
    AddBicycleSprite(xPosOrig+340, 0, 9, RM_IMG_BICYCLE_SPRITE, 
                     SM_SPRITE, MM_EAST);
  }
  else
  {
    rand = MM_RandomNumberGen(1, 4);  // number of sprites to make
    AddRandomSprite(rand, xPosOrig+50, xPos);  
  }
  
  // ---------- TWIN ARCHER SPRITES ----------
  xPos += 30;
  AddSpriteObject(xPos, 0, 9, RM_IMG_ARCHER_SPRITE, SM_SPRITE, 0);
  
  if (MM_RandomNumberGen(0, 1))
    MAP_AddExtraLifeObject(xPos+200, MM_RandomNumberGen(80, 250), 9);
  
  xPos += 400;
  AddSpriteObject(xPos, 0, 9, RM_IMG_ARCHER_SPRITE, SM_SPRITE, 0);
  
  
  // ---------- ALTERNATING FALLING SHELVES ----------
  xPos += 100;
  w     = RM_GetImageWidth(GEN_RANDOM_SHELF) - 1;
  
  rand  = MM_RandomNumberGen(xPos, xPos+100);
  id    = MM_RandomNumberGen(BASKETBALL_SPRITE, SOCCERBALL_SPRITE);
  AddSpriteObject(rand, 0, 9, id, SM_SPRITE, 0);
  
  rand  = MM_RandomNumberGen(xPos+100, xPos+200);
  id    = MM_RandomNumberGen(BASKETBALL_SPRITE, SOCCERBALL_SPRITE);
  AddSpriteObject(rand, 0, 9, id, SM_SPRITE, 0);
  
  rand  = MM_RandomNumberGen(xPos+200, xPos+300);
  id    = MM_RandomNumberGen(BASKETBALL_SPRITE, SOCCERBALL_SPRITE);
  AddSpriteObject(rand, 0, 9, id, SM_SPRITE, 0);
  
  // Create shelves  
  extraLife = 0;
  for (x=1; x <= 10; x++)
  {
    if ((x%2) == 0)  // Even numbers are falling shelves
    {
      AddSpriteObject((xPos-34), 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0); 
      if (extraLife == 0 && MM_RandomNumberGen(1,3)==2)
      {
        MAP_AddExtraLifeObject(xPos+55, 77, 5.5);
        extraLife = 1;
      }
    }
    else             // odd numbers are normal shelves
    {
      AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
      AddSpriteObject(xPos+w/2, 25, 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
    }
    xPos += w;
  }
  
  // ---------- SERIES OF ROLLING BOMBS ----------
  AddSeriesSprite(xPos+30, RM_IMG_BOMB_SPRITE, 15, 25);
  
  // ---------- DECOY PUNCHING BAGS ----------
  xPos += 35;
  w     = RM_GetImageWidth(PUNCHING_BAG_SPRITE) + 15;
  for (x = 0; x < 4; x++) // Create PBs  
  {
    AddSpriteObject(xPos, 117, 6, PUNCHING_BAG_SPRITE, SM_BACKGROUND, 0);
    xPos += w;
  }
  AddSpriteObject(xPos-30, 0, 9, RM_IMG_ARCHER_SPRITE, SM_SPRITE, 0);
  
  
  // ---------- BICYCLE SERIES ----------
  xPos += 400;
  dir  = MM_RandomNumberGen(0, 1)?MM_WEST:MM_EAST;
  AddBicycleSprite(xPos, 184, 6, RM_IMG_BICYCLE1_SPRITE, SM_BACKGROUND, dir);
  xPos += RM_GetImageWidth(RM_IMG_BICYCLE1_SPRITE);

  AddSeriesSprite(xPos-50, RM_IMG_BICYCLE_SPRITE, 255, 30);
  dir   = (dir==MM_EAST)?MM_WEST:MM_EAST;
  AddBicycleSprite(xPos, 184, 6, RM_IMG_BICYCLE1_SPRITE, SM_BACKGROUND, dir);
  xPos += RM_GetImageWidth(RM_IMG_BICYCLE1_SPRITE);
  
  
  // ---------- HOCKEY STICK / SHELF COMBOS ----------
  // ---------- DECOY BOWLING / RANDOM FALLING SHELF ----------
  xPos    += 300;
  w        = RM_GetImageWidth(GEN_RANDOM_SHELF);
  xPosOrig = xPos;
  rand     = MM_RandomNumberGen(0, 2);
  for (x=0; x < 3; x++)
  {
    AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
    xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
    
    // Offset xPos of falling shelf by 34 since it has 34 transparnet
    // pixels before shelf starts in 1st frame
    if (x == rand) // Falling shelf
      AddSpriteObject((xPos-34), 49, 6, FALLING_SHELF_SPRITE, 
                      MM_RandomNumberGen(0,1)?SM_BACKGROUND:SM_SPRITE, 0);
    else
      AddSpriteObject(xPos, 63, 5, RM_BOWLING_SHELF_SPRITE, SM_BACKGROUND, 0);

    xPos += w;
    AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
    xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
    
    xPos += 200; // Add space between end of current set and start of next
  }
  
  // Create an Employee Sprite
  AddSpriteObject(MM_RandomNumberGen(xPosOrig, xPos), 0, 10, EMPLOYEE_SPRITE, 
                  SM_SPRITE, 0);

  // Create 2 bouncing bomb sprites
  AddSpriteObject(MM_RandomNumberGen(xPosOrig, xPos), 0, 10, 
                  RM_IMG_BOUNCE_BOMB_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(MM_RandomNumberGen(xPosOrig, xPos), 0, 10, 
                  RM_IMG_BOUNCE_BOMB_SPRITE, SM_SPRITE, 0);
  
  
  //---------- BOWLING BALLL SHELVES ----------
  w        = RM_GetImageWidth(GEN_RANDOM_SHELF)-1;
  xPosOrig = xPos;
  for (x = 0; x < 10; x++)
  {
    AddSpriteObject(xPos, 63, 5, RM_BOWLING_SHELF_SPRITE, SM_SPRITE, 0);
    xPos += w;
  }
  
  rand = MM_RandomNumberGen(1, 3);  
  for (x=0; x < rand; x++)
  {
    xPosNew = MM_RandomNumberGen(xPosOrig, xPos);  
    AddSpriteObject(xPosNew, MM_RandomNumberGen(95, 200), 
                    9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  }
  
  // Create 0, 1, or 2 random sprites
  rand = MM_RandomNumberGen(2, 4);
  AddRandomSprite(rand, xPosOrig+w, xPos-(2*w));  
  
  // Create a wheelie bicycle sprite (almost allways)
  if (MM_RandomNumberGen(1, 10) != 5)
    AddBicycleSprite(xPos-w, 0, 9, RM_IMG_BICYCLE2_SPRITE, SM_SPRITE, MM_WEST);
  
  
  // ---------- ARNOLD SPRITE (BALL BOMBARDMENT) ----------
  xPos += 240;
  w     = RM_GetImageWidth(RM_IMG_WEIGHTS_SPRITE) + 20;
  
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;

  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  
  AddSeriesSprite(xPos+10, BASKETBALL_SPRITE, 255, 25);
  AddSpriteObject(xPos, 123, 5, ARNOLD_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(ARNOLD_SPRITE) + 20;
 
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
 
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;


  // ---------- RANDOM PUNCHING BAGS ----------
  xPosOrig = xPos;
  xPos += 100;
  w     = RM_GetImageWidth(PUNCHING_BAG_SPRITE) + 80;
  for (x = 0; x < 6; x++) // Create PBs  
  {
    AddSpriteObject(xPos, 117, 6, PUNCHING_BAG_SPRITE, SM_SPRITE, 0);
    if (x%2 == 0)
      AddSpriteObject(xPos+w/2, 100, 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
    xPos += w;
  }
  if (MM_RandomNumberGen(0,1))  // maybe he'll be created, maybe he won't
    AddSpriteObject(xPos, 0, 9, RM_IMG_ARCHER_SPRITE, SM_SPRITE, 0);
  
  xPosNew = MM_RandomNumberGen(xPosOrig, xPos);
  //MAP_AddExtraLifeObject(xPosNew, MM_RandomNumberGen(80, 250), 9);

 
  // ---------- RANDOM TENTS & GRILLS ----------
  xPos    += 200;
  w        = RM_GetImageWidth(TENT_GREEN);
  xPosOrig = xPos;  
  
  // Normal tent
  AddTentSprite(xPos, 5, 0, 0, 0);
  xPos +=  w;
  
  //Sideways tent with 2 grills
  AddSpriteObject(xPos, 125, 5, L1S1_TENT3, SM_BACKGROUND, 0);
  xPosNew = xPos + 10;
  AddSpriteObject(xPosNew, 174, 8.5, GRILL_SPRITE, SM_BACKGROUND, 0);
  xPosNew = xPos + RM_GetImageWidth(GRILL_SPRITE) + 40;
  AddSpriteObject(xPosNew, 174, 8.5, GRILL_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(L1S1_TENT3);
  
  // Tent/Grill/Tent Series
  xPos += 120;
  AddTentSprite(xPos, 5, 0, 0, MM_EAST);
  xPos += w;
  xPosNew = xPos - (RM_GetImageWidth(GRILL_SPRITE) / 2);
  AddSpriteObject(xPosNew, 170, 4, GRILL_SPRITE, SM_BACKGROUND, 0);
  AddTentSprite(xPos, 5, 0, 0, MM_WEST);
  xPos += w;
  
  rand = MM_RandomNumberGen(1,3);
  AddRandomSprite(rand, xPosOrig, xPos);
  
  xPosNew = MM_RandomNumberGen(xPosOrig, xPos);
  AddSpriteObject(xPosNew, 0, 9, RM_IMG_BICYCLE_SPRITE, SM_SPRITE, 0);
  
  
  // Random Falling Grills
  xPos += 35;
  w     = RM_GetImageWidth(GRILL_SPRITE) + 35;
  for (x=0; x < 9; x++)
  {
    type = MM_RandomNumberGen(0,1)?SM_BACKGROUND:SM_SPRITE;
    AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, type,  0);
    xPos += w;
  }
  
  AddTentSprite(xPos, 5, 0, 0, 0);
  xPos += RM_GetImageWidth(TENT_GREEN);
  
  // Tent / Sideways Tent / Tent
  xPosOrig = xPos;
  w    = RM_GetImageWidth(TENT_GREEN);
  AddTentSprite(xPos, 5.1, TENT_RED, 0, MM_EAST);
  xPos += w - 93;
  AddSpriteObject(xPos, 125, 5.0, L1S1_TENT3, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(L1S1_TENT3) - 99;
  AddTentSprite(xPos, 5.1, TENT_GREEN, 0, MM_WEST);
  xPos += w;
  
  // Tent / Grill / Tent
  id = MM_RandomNumberGen(TENT_GREEN, TENT_BLUE); 
  xPos += 3;
  w     = RM_GetImageWidth(TENT_GREEN);
  AddTentSprite(xPos, 5, id, 0, MM_WEST);
  xPos += w + 5;
  xPosNew = xPos - (RM_GetImageWidth(GRILL_SPRITE) / 2) + 2;
  AddSpriteObject(xPosNew, 172, 8.5, GRILL_SPRITE, SM_SPRITE,  0);
  AddTentSprite(xPos, 5, id, 0, MM_EAST);
  xPos += w;
  
  // Overlapping Tents
  xPos += 20;
  w     = RM_GetImageWidth(TENT_GREEN);
  AddTentSprite(xPos, 5.3, TENT_PURPLE, 0, MM_EAST);
  xPos += w - 25;
  AddTentSprite(xPos, 5.2, TENT_ORANGE, 0, MM_EAST);
  xPos += w - 55;
  AddSpriteObject(xPos, 125, 5.1, L1S1_TENT3, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(L1S1_TENT3);
  
  rand = MM_RandomNumberGen(1,5);
  AddRandomSprite(rand, xPosOrig, xPos);
  
  // Tent / 4 Grills / Tent
  xPos += 75;
  AddTentSprite(xPos, 5, 0, 0, MM_WEST);

  // Random Falling Grills
  xPos += RM_GetImageWidth(TENT_GREEN) - 27;
  w     = RM_GetImageWidth(GRILL_SPRITE);
  for (x=0; x < 4; x++)
  {
    type = MM_RandomNumberGen(0,1)?SM_BACKGROUND:SM_SPRITE;
    AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, type,  0);
    xPos += w;
  }
  xPos -= 22;
  AddTentSprite(xPos, 5, 0, 0, MM_EAST);
  xPos +=  RM_GetImageWidth(TENT_GREEN);
 
 
 //---------- LONG SHELF SERIES ----------
  xPos    += 30;
  w        = RM_GetImageWidth(GEN_RANDOM_SHELF)-1;
  xPosOrig = xPos;
  for (x = 0; x < 20; x++)
  {
    if (MM_RandomNumberGen(1, 7) == 4)
    {
      type = (MM_RandomNumberGen(0,4))?SM_SPRITE:SM_BACKGROUND;
      if (MM_RandomNumberGen(0,1))
        AddSpriteObject(xPos, 63, 5, RM_BOWLING_SHELF_SPRITE, type, 0);
      else
        AddSpriteObject(xPos-34, 49, 6, FALLING_SHELF_SPRITE, type, 0);
    }
    else
    {
      AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
    }
    xPos += w;
  }
  
  // Create random sprites
  rand = MM_RandomNumberGen(10, 15);
  AddRandomSprite(rand, xPosOrig, xPos);
  
  xPosNew = MM_RandomNumberGen(xPosOrig, xPos);
  AddSpriteObject(xPosNew, 0, 9, RM_IMG_ARCHER_SPRITE, SM_SPRITE, 0);
  
  xPos += 250;
  AddLBSprite(xPos, 40);  


  // ---------- BICYCLE SERIES ----------
  xPos += 500;
  dir  = MM_RandomNumberGen(0, 1)?MM_WEST:MM_EAST;
  AddBicycleSprite(xPos, 184, 6, RM_IMG_BICYCLE1_SPRITE, SM_BACKGROUND, dir);
  xPos += RM_GetImageWidth(RM_IMG_BICYCLE1_SPRITE);

  AddSeriesSprite(xPos-50, RM_IMG_BICYCLE_SPRITE, 255, 30);
  dir   = (dir==MM_EAST)?MM_WEST:MM_EAST;
  AddBicycleSprite(xPos, 184, 6, RM_IMG_BICYCLE1_SPRITE, SM_BACKGROUND, dir);
  xPos += RM_GetImageWidth(RM_IMG_BICYCLE1_SPRITE);

 
  // ----------  FALLING SHELVES & GRILLS ----------
  extraLife = 0;
  // HOCKEY STICKS / FALLING SHELVES / GRILL / FALLING SHELVES / HOCKEY STICKS
  xPos += 100;
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  xPos -= 34;  // move back 34 spaces so hockey sticks allign with shelf
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  if (extraLife == 0 && MM_RandomNumberGen(1,4)==2)
  {
    MAP_AddExtraLifeObject(xPos+55, 77, 5.5);
    extraLife = 1;
  }
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE) + 10;
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE) + 10;
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE) - 34;
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);

  AddSpriteObject(xPos + 30,  100, 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddBicycleSprite(xPos + 100, 0, 9, RM_IMG_BICYCLE_SPRITE, SM_SPRITE, MM_WEST);

  xPos += 50;
  // Hockey Sticks, random shelf, hockey sticks
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF);
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);


  // Long Series of Falling shelves / grills
  xPos += 200;  
  // |o|oo|o|
  // Shelf|Grill|Shelf|Grill|Shelf
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  if (extraLife == 0 && MM_RandomNumberGen(1,4)==2)
  {
    MAP_AddExtraLifeObject(xPos+55, 77, 5.5);
    extraLife = 1;
  }
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE) + 10;
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE) + 10;
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE)+10;
  
  // Grill|Grill
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE);
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE) + 10;
  
  // Shelf|Grill|Shelf
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE) + 10;
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE) + 10;
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  if (extraLife == 0 && MM_RandomNumberGen(1,4)==2)
  {
    MAP_AddExtraLifeObject(xPos+55, 77, 5.5);
    extraLife = 1;
  }
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE);
  
  xPos += 10;
  AddSpriteObject(xPos, 0, 9, RM_IMG_ARCHER_SPRITE, SM_SPRITE, 0);
  
  xPos += 50;
  // Hockey Sticks, random shelf, hockey sticks
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  AddSpriteObject(xPos, 63, 5, RM_BOWLING_SHELF_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF);
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  xPos += 100;
  
  //HOCKEY STICKS / FALLING SHELVES / GRILL / FALLING SHELVES / HOCKEY STICKS
  xPos += 200;
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  xPos -= 34;  // move back 34 spaces so hockey stiks allign with shelf
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  if (extraLife == 0)
  {
    MAP_AddExtraLifeObject(xPos+55, 77, 5.5);
    extraLife = 1;
  }
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE) + 10;
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE) + 10;
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE) - 34;
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);

  AddSpriteObject(xPos + 30, 100, 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddBicycleSprite(xPos + 100, 0, 9, RM_IMG_BICYCLE_SPRITE, SM_SPRITE, MM_WEST);
  
  
  // ---------- ARNOLD SPRITE (BALL BOMBARDMENT) ----------
  xPos += 240;
  w     = RM_GetImageWidth(RM_IMG_WEIGHTS_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSeriesSprite(xPos+10, BASKETBALL_SPRITE, 255, 30);
  AddSpriteObject(xPos, 123, 5, ARNOLD_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(ARNOLD_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;

  
  // ---------- TENT SERIES 2 ----------
  xPosOrig = xPos;
  xPos += 200;
  w    = RM_GetImageWidth(GRILL_SPRITE);
  type = MM_RandomNumberGen(0, 1)?SM_SPRITE:SM_BACKGROUND;
  for (x=0; x < 4; x++)
  {
    AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, type, 0);
    xPos += w;
  }  
  
  // Sets of randomly facing tents
  w = RM_GetImageWidth(TENT_GREEN);
  dir = MM_RandomNumberGen(0, 1)?MM_EAST:MM_WEST;
  for (x=0; x < 1; x++)
  {
    AddTentSprite(xPos, 5, 0, 0, dir);
    xPos += w;
    dir = (dir==MM_WEST)?MM_EAST:MM_WEST;
    AddTentSprite(xPos, 5, 0, 0, dir);
    xPos += w + 44;
    dir = (dir==MM_WEST)?MM_EAST:MM_WEST;
  }  
  xPos += 20;
  
  // 3 Overlapping tents
  xPos += 30;
  w     = RM_GetImageWidth(TENT_GREEN) - 16;
  zPos  = 5.5;
  for (x=0; x < 3; x++)
  {
    AddTentSprite(xPos, zPos, 0, SM_SPRITE, MM_EAST);
    zPos -= .1;
    xPos += w;
  }
  
  // 2 Grills
  xPos += 20;
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE);
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE);
  
  
  // Tent / Sideways Tent / Tent
  xPosOrig = xPos;
  w    = RM_GetImageWidth(TENT_GREEN);
  AddTentSprite(xPos, 5.1, 0, SM_SPRITE, MM_EAST);
  xPos += w - 93;
  AddSpriteObject(xPos, 125, 5.0, L1S1_TENT3, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(L1S1_TENT3) - 99;
  AddTentSprite(xPos, 5.1, 0, SM_SPRITE, MM_WEST);
  xPos += w;
  // Grill in the middle 
  AddSpriteObject(xPos-22, 172, 8.5, GRILL_SPRITE, SM_BACKGROUND, 0);
  AddTentSprite(xPos, 5.1, 0, SM_SPRITE, MM_EAST);
  xPos += w - 93;
  AddSpriteObject(xPos, 125, 5.0, L1S1_TENT3, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(L1S1_TENT3) - 99;
  AddTentSprite(xPos, 5.1, 0, 0, MM_WEST);
  xPos += w;
  // Grill in the middle 
  AddSpriteObject(xPos-22, 172, 8.5, GRILL_SPRITE, SM_BACKGROUND, 0);
  AddTentSprite(xPos, 5.1, 0, SM_SPRITE, MM_EAST);
  xPos += w - 93;
  AddSpriteObject(xPos, 125, 5.0, L1S1_TENT3, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(L1S1_TENT3) - 99;
  AddTentSprite(xPos, 5.1, 0, 0, MM_WEST);
  xPos += w;
  
  // 2 Grills
  xPos += 20;
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE);
  AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GRILL_SPRITE);
  
  // 3 tents overlapping
  xPos += 20;
  w     = RM_GetImageWidth(TENT_GREEN) - 16;
  zPos  = 5.0;
  for (x=0; x < 3; x++)
  {
    AddTentSprite(xPos, zPos, 0, SM_SPRITE, MM_WEST);
    zPos += .1;
    xPos += w;
  }
  
  // Sets of randomly facing tents
  xPos += 20;
  w     = RM_GetImageWidth(TENT_GREEN);
  dir   = MM_RandomNumberGen(0, 1)?MM_EAST:MM_WEST;
  for (x=0; x < 1; x++)
  {
    AddTentSprite(xPos, 5, 0, SM_SPRITE, dir);
    xPos += w;
    dir = (dir==MM_WEST)?MM_EAST:MM_WEST;
    AddTentSprite(xPos, 5, 0, 0, dir);
    xPos += w + 44;
    dir = (dir==MM_WEST)?MM_EAST:MM_WEST;
  }  
  
  // Four grills
  w    = RM_GetImageWidth(GRILL_SPRITE);
  type = MM_RandomNumberGen(0, 1)?SM_SPRITE:SM_BACKGROUND;
  for (x=0; x < 4; x++)
  {
    AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, type, 0);
    xPos += w;
  }  
  
  rand = MM_RandomNumberGen(3, 8);
  AddRandomSprite(rand, xPosOrig, xPos);  
  
  for (x=0; x < 3; x++)
  {
    rand    = MM_RandomNumberGen(125, 200);  
    xPosNew = MM_RandomNumberGen(xPosOrig, xPos);  
    AddSpriteObject(xPosNew, rand, 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  }


  // ---------- ARNOLD SPRITE / ARNOLD SPRITE (BOMB BOMBARDMENT) ----------
  xPos    += 240;
  xPosOrig = xPos;
  w     = RM_GetImageWidth(RM_IMG_WEIGHTS_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSeriesSprite(xPos+10, RM_IMG_BOMB_SPRITE, 255, 50);
  AddSpriteObject(xPos, 123, 5, ARNOLD_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(ARNOLD_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
  
  xPos += 60;
  w     = RM_GetImageWidth(RM_IMG_WEIGHTS_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSeriesSprite(xPos+10, RM_IMG_BOMB_SPRITE, 255, 50);
  AddSpriteObject(xPos, 123, 5, ARNOLD_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(ARNOLD_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
  
  rand  = MM_RandomNumberGen(1, 3);
  AddRandomSprite(rand, xPosOrig, xPos);  
  
  // ---------- FALLING SHLELF / 4 SPRITE GRILLS / MORE SHELVES ----------
  xPos += 150;
  AddSpriteObject(xPos, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  if (MM_RandomNumberGen(0,2))
    AddSpriteObject(xPos+55, 77, 5.5, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(FALLING_SHELF_SPRITE) + 10;
  
  w    = RM_GetImageWidth(GRILL_SPRITE);
  rand = MM_RandomNumberGen(0, 3);
  for (x=0; x < 4; x++)
  {
    AddSpriteObject(xPos, 172, 8.5, GRILL_SPRITE, SM_SPRITE, 0);
    xPos += w;
  }
  xPos += 10;
  AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF)-1;
  AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF)-1;
  AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF)-1;
  AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF)-1;
  
  
  // ---------- HOCKEY STICK / BOWLING SHELF COMBO ----------
  xPos += 100;
  type = (MM_RandomNumberGen(0,2)==1)?SM_BACKGROUND:SM_SPRITE;
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  AddSpriteObject(xPos, 63, 5, RM_BOWLING_SHELF_SPRITE, type, 0);
  xPos += RM_GetImageWidth(RM_BOWLING_SHELF_SPRITE);
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  
  //---------- PUNCHING BAGS spaced 5 apart, aletrnating ----------
  xPos += 200;
  w     = RM_GetImageWidth(PUNCHING_BAG_SPRITE) + 5;
  rand  = MM_RandomNumberGen(0,1);
  for (x = 0; x < 10; x++) // Create PBs  
  {
    type = ((x%2)==rand)?SM_BACKGROUND:SM_SPRITE;
    AddSpriteObject(xPos, 117, 6, PUNCHING_BAG_SPRITE, type, 0);
    xPos += w;
  }
  
  // ---------- ARNOLD SPRITE (BALL BOMBARDMENT) ----------
  xPos += 75;
  w     = RM_GetImageWidth(RM_IMG_WEIGHTS_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE,  SM_BACKGROUND, 0);
  xPos += w;
  AddSeriesSprite(xPos+10, BASKETBALL_SPRITE, 255, 25);
  AddSpriteObject(xPos, 123, 5, ARNOLD_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(ARNOLD_SPRITE) + 20;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
  AddSpriteObject(xPos, 164, 5, RM_IMG_WEIGHTS_SPRITE, SM_BACKGROUND, 0);
  xPos += w;
  
  //---------- PUNCHING BAGS spaced 5 aspart, aletrnating ----------
  xPos += 75;
  w     = RM_GetImageWidth(PUNCHING_BAG_SPRITE) + 5;
  rand  = MM_RandomNumberGen(0,1);
  for (x = 0; x < 10; x++) // Create PBs  
  {
    type = ((x%2)==rand)?SM_BACKGROUND:SM_SPRITE;
    AddSpriteObject(xPos, 117, 6, PUNCHING_BAG_SPRITE, type, 0);
    xPos += w;
  }
  
  // ---------- HOCKEY STICK / BOWLING SHELF COMBO ----------
  xPos += 100;
  type = (MM_RandomNumberGen(0,2)==1)?SM_BACKGROUND:SM_SPRITE;
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_WEST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  AddSpriteObject(xPos, 63, 5, RM_BOWLING_SHELF_SPRITE, type, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF) - 1;
  AddSpriteObject(xPos-34, 49, 6, FALLING_SHELF_SPRITE, SM_SPRITE, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF) - 1;
  AddSpriteObject(xPos, 63, 5, GEN_RANDOM_SHELF, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(GEN_RANDOM_SHELF);
  AddSpriteObject(xPos, 126, 5, RM_IMG_HOCKEY_EAST_SPRITE, SM_BACKGROUND, 0);
  xPos += RM_GetImageWidth(RM_IMG_HOCKEY_WEST_SPRITE);
  
      
  xPosNew = MAP_GetLevelSize(MM_LEVEL1) - 180;
  AddSpriteObject(xPosNew, 145, 6, RM_IMG_SIGN_SPRITE, SM_BACKGROUND, 0);
   
  for (xPos=0; xPos < MM_SCREEN_WIDTH; xPos++)
  {
    if ( _map[xPos] && _map[xPos]->enabled == 0 )
    {
      EnableObjectsPrivate(_map[xPos], xPos);
    }
  }

  return(0);
}

//------------------------------------------------------------------------------
// Name:     AddBicycleSprite
// Summary:  Wrapper function used to easily implement a bicycle sprite
// Inputs:   1. Sprite x position
//           2. Sprite y position
//           3. Sprite z position
//           4. Sprite ID
//           5. Sprite type
//           6. Sprite dircteion (MM_EAST or MM_WEST, 0 for random direction)
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void AddBicycleSprite(int xPos, int yPos, float zPos, int id, int type, int dir)
{
  int *p = 0;
  if (dir) // if a dir was given, allocate memory for it
  {
    p = malloc(sizeof(int));
    if (p) // verify memory was allocated
      *p = dir; 
  }
  
  // add the sprite to the map
  AddSpriteObject(xPos, yPos, zPos, id, type, (void*) p);
}

//------------------------------------------------------------------------------
// Name:     AddSeriesSprite
// Summary:  Wrapper function used to easily implement a series sprite
// Inputs:   1. Sprite x position
//           2. Sprite ID
//           3. Number of sprites in series
//           4. Frame delay between sprites in series
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void AddSeriesSprite(int xPos, int id, int numSprites, int frmDel)
{
  // Allocate memory for the 3 additional peices of information 
  // required by series sprites
  int *p = malloc(sizeof(int) * 3);
  if (p) // give up if memory was not allocated
  {
    p[0] = id;
    p[1] = numSprites;
    p[2] = frmDel;
    AddSpriteObject(xPos, 0, 0, RM_IMG_SERIES_SPRITE, SM_SPRITE, (void*) p); 
  }
}

//------------------------------------------------------------------------------
// Name:     AddTentSprite
// Summary:  Wrapper function used to easily implement a tent sprite
// Inputs:   1. Sprite x position
//           2. Sprite z position
//           3. Sprite ID (o for random colored tent)
//           4. Type (SM_BACKGROUND, SM_SPRITE, 0 for random value)
//           5. Direction (MM_EAST, MM_WEST, 0 for random dirextion)
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void AddTentSprite(int xPos, float zPos, int id, int type, int dir)
{
  int *p;
  
  if (id == 0)  // chose random tent color
   id = MM_RandomNumberGen(TENT_GREEN, TENT_BLUE); // tent color

  if (type == 0)  // choose random type
    type = (MM_RandomNumberGen(0,2)==1)?SM_BACKGROUND:SM_SPRITE;  
  
  if (dir == 0)  // chose random direction
   dir = MM_RandomNumberGen(0, 1)?MM_EAST:MM_WEST;
  
  // Allocate memory for direction (this is sprite specific data)
  p  = malloc(sizeof(int)); 
  if (p)
    *p = dir;
    
  AddSpriteObject(xPos, 130, zPos, id, type, (void*) p);
}

//------------------------------------------------------------------------------
// Name:     AddRandomSprite
// Summary:  Wrapper function used to easily implement random moving sprites
// Inputs:   1. Number of random sprites to create
//           2. Starting x position allowed for sprite
//           3. Ending x position allowed for sprite
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void AddRandomSprite(int numSprites, int xStart, int xEnd)
{
  #define NUM_RAND_SPRITES 5
  int xPos = 0;
  int  x   = 0;
  int id;   
  static int map[NUM_RAND_SPRITES] = { 
                                       0,
                                       EMPLOYEE_SPRITE, 
                                       RM_IMG_BOUNCE_BOMB_SPRITE, 
                                       RM_IMG_BOMB_SPRITE,
                                       RM_IMG_BICYCLE_SPRITE
                                     };
  // Create the specified number of sprites
  for (x = 0; x < numSprites; x++)
  {
    id = MM_RandomNumberGen(0, NUM_RAND_SPRITES-1);
    
    if (id == 0)
      id = MM_RandomNumberGen(BASKETBALL_SPRITE, SOCCERBALL_SPRITE);
    else
      id = map[id];     
    
    xPos = MM_RandomNumberGen(xStart, xEnd);
    AddSpriteObject(xPos, 0, 9, id, SM_SPRITE, 0);
  }
}

//------------------------------------------------------------------------------
// Name:     AddLBSprite
// Summary:  Wrapper function used to easily implement the famous LB sprite
// Inputs:   1. X Position
//           2. Y position
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void AddLBSprite(int xPos, int yPos)
{
  SDL_Surface *t = RM_GetImage(RM_IMG_POWER_UP_SPRITE);
  int          w = t->w;
  int          h = t->h / 6;
  AddSpriteObject(xPos, yPos+(h*0), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*1), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*2), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*3), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*4), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  
  xPos += w;
  AddSpriteObject(xPos, yPos+(h*4), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  
  xPos += w;
  AddSpriteObject(xPos, yPos+(h*2), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*3), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*4), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*5), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*6), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  
  xPos += w;
  AddSpriteObject(xPos, yPos+(h*2), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*4), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*6), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  
  xPos += w;
  AddSpriteObject(xPos, yPos+(h*2), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*4), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*6), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  
  xPos += w - 8;
  AddSpriteObject(xPos, yPos+(h*3), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
  AddSpriteObject(xPos, yPos+(h*5), 9, RM_IMG_POWER_UP_SPRITE, SM_SPRITE, 0);
}

//------------------------------------------------------------------------------
// Name:     MAP_AddExtraLifeObject
// Summary:  Wrapper function used to easily an an extra life sprite
// Inputs:   1. X Position
//           2. Y position
//           3. Z Position
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MAP_AddExtraLifeObject(int xPos, int yPos, float zPos)
{
  int levelSize = MAP_GetLevelSize(MM_LEVEL1) - 30;
  if (xPos < levelSize)
    AddSpriteObject(xPos, yPos, zPos, RM_IMG_BONUS_LIFE_SPRITE, SM_SPRITE, 0);
}

//------------------------------------------------------------------------------
// Name:     AddSpriteObject
// Summary:  Creates memory for and adds the current levelmap structure into
//           the level map list.  Each element in this list contains the 
//           information to create a specific sprite at the specified time and
//           location in the game
// Inputs:   1. X Position
//           2. Y position
//           3. Z Position
//           4. Sprite ID (as specified in Resource Manager)
//           5. type (SM_BACKGROUND or SM_SPRITE)
//           6. Pointer to structure containing optional initilization info
// Outputs:  None
// Returns:  Pointer to levelmap structure
// Cautions: None
//------------------------------------------------------------------------------
LevelMap * AddSpriteObject(int xPos, int yPos, float zPos, 
                           int id, int type, void *setup)
{
  LevelMap *insertPoint;
  LevelMap *curPtr    = (LevelMap*) malloc(sizeof(LevelMap));
  
  if (xPos <= _levelSize)
  {
    if ( curPtr )
    {
      curPtr->yPos        = yPos;
      curPtr->zPos        = zPos;
      curPtr->id          = id;
      curPtr->type        = type;
      curPtr->enabled     = 0;
      curPtr->sprInitInfo = setup;
      curPtr->nextPtr     = 0;
    }
    
    if (_map[xPos] == 0)
    {
      _map[xPos] = curPtr;
    }
    else
    {
      insertPoint = _map[xPos];
      while (insertPoint->nextPtr != 0)
      {
        insertPoint = insertPoint->nextPtr;
      }
      insertPoint->nextPtr = curPtr;
    }
  }
  else
  {
    EH_Error(EH_SEVERE, 
             "Cannot add sprite %i at position %i, Level Size is only %i.", 
             id, xPos, _levelSize);
  }
  
  return(curPtr);
}

//------------------------------------------------------------------------------
// Name:     MAP_EnableObjects
// Summary:  Enables the sprites destined to appear at the current xPosition 
//           in the game
// Inputs:   Amount of space hero has covered in current frame.  Based on the
//           current absolute position in the level, and how many pixels the 
//           hero just moved, the map is referenced and all sprites located 
//           in that range are activated.
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MAP_EnableObjects(float move)
{
  float xPosGlobal = BG_GetxPosGlobal();
  int xPos         = (int) xPosGlobal;
  // xPos will be hero's global x position (relative to entire level size)
  // We add 2 to it to esnure that when we look for new sprites to activate
  // we do not miss any sprites between the hero's current movement and his 
  // previous movement.  A pixel may be missed here and there due to rounding
  // as a result of using a float to represent the hero's x position
  int mv           = (int) move + 2;  
  
  if (mv > 0)
  {
    int x, newPixel;
    for (x=1; x <= mv; x++)
    {
      // Start at the end of the screen (pixel 479) and work backwords
      // first sprites appearing at 479 gets enabled, then 478, etc, etc...
      newPixel = xPos + MM_SCREEN_WIDTH - x;
      if ( _map[newPixel] && _map[newPixel]->enabled == 0 )
      {
        EnableObjectsPrivate(_map[newPixel], newPixel);
      }
    }
  }
}

// xPosRel - Value fro 0 - 480, as to where new sprite should be placed in the
//           screen
// xPosAbs - Value betwen 0 and level size, as in absolute place in level 
//           where sprite became active
// Could not get Background objects to line up perfectly using relative values
// hopefully sprites will not need quite as much percision.
//------------------------------------------------------------------------------
// Name:     EnableObjectsPrivate
// Summary:  Function used to interface with Spriate Manager class and actually
//           create an instance of a given sprite.
// Inputs:   1.  LevelMap structure with info on sprite's initilization
//           2.  Absolute x Position relative to entire level of where sprite
//               is going to appear (will be scaled down to a vlaue between
//               0 & 479)
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void EnableObjectsPrivate( LevelMap *curPtr, int xPosAbs)
{
  float xPosRelFloat, xDecOffset;
  float xPosGlobal = BG_GetxPosGlobal();  // get current xPos
  // xDecOffset will be added to sprite's X Position so that it has the same
  // decimal value as the global x position.  This will insure all new sprites added
  // to the screen have the same decimal value, and are not slightly off.  If decimal
  // values are off, two objects drawn side by side will appear to have 1 column
  // between them at random times.
  
  // get decimal value of current global xPos (I.E. the background's position)
  xDecOffset = xPosGlobal - (int)xPosGlobal;  
  // if non-zero, take 1 - value.  This is because sprites are updated by
  // subtracting the current moveBg value from the sprite's current x position
  if (xDecOffset > 0.0)  
    xDecOffset = 1 - xDecOffset;
    
  // Absolute value minus current x position dictates where the current sprite should
  // be drawn on screen (A value between 1 and 480).  We take the integer portion
  // of xPosGlobal so the deciaml value we calculated above can be added in to
  // ensure all new sprites enter on screen with the same decimal value
  xPosRelFloat = xPosAbs - (int)xPosGlobal;  
  // add in correct decimal value so this number matches for all sprites
  xPosRelFloat += xDecOffset;  
  
  //if (curPtr->type == SM_BACKGROUND)
  //{
  //  xPosRelFloat = xPosAbs;
  //}
  
  while (curPtr)    // If entry exists at this index, remove it
  {
    curPtr->enabled = 1;
    SM_CreateSprite(xPosRelFloat, curPtr->yPos, curPtr->zPos, 
                    curPtr->id, curPtr->type, curPtr->sprInitInfo);
    curPtr = (LevelMap*) curPtr->nextPtr; // get pointer to next map struct in case it exists
  }
}

//------------------------------------------------------------------------------
// Name:     FreeMemory
// Summary:  Function used to free all memory heled by Map Manager class
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
int FreeMemory()
{
  LevelMap *curPtr, *nextPtr;
  int x = 0;
  if (_map)
  {  
    for (x=0; x < _levelSize; x++)
    {
      curPtr = _map[x];  // Point to current map entry
      while (curPtr)    // If entry exists at this index, remove it
      {
        // get pointer to next map struct in case it exists
        nextPtr = (LevelMap*) curPtr->nextPtr; 
        if (curPtr->sprInitInfo)       // free current map struct
          free(curPtr->sprInitInfo);
        free(curPtr);
        // set current pointer to next map struct 
        // (if it exists, will be 0 if it doesn't)
        curPtr = nextPtr;  
      }
      _map[x] = 0;
    }
    free(_map);
    _map = 0;
  }
  return(0);
}

//------------------------------------------------------------------------------
// Name:     MAP_GetLevelSize
// Summary:  Returns the size of the specified level
// Inputs:   None
// Outputs:  None
// Returns:  Size of specified level (in pixels)
// Cautions: Only 1 level exists, but the framework is present for multiple
//           levels.
//------------------------------------------------------------------------------
int MAP_GetLevelSize(unsigned int level)
{
  int levelSize = 0;
  if (level == MM_LEVEL1)
  {
    levelSize = LEVEL_1_MAP_SIZE;
  }
  return(levelSize);
}

//------------------------------------------------------------------------------
// Name:     MAP_DebugReset
// Summary:  Function used to reset the current level when debugging
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: Function remains in code for Sentimental reasons
//------------------------------------------------------------------------------
int MAP_DebugReset()
{
  SM_ClearList();
  BG_SetXPosGlobal(0);
  FreeMemory();
  return(0);
}

