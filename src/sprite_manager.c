//-----------------------------------------------------------------------------
//  Class:
//  Sprite Manager
//
//  Description:
//  This class contains the code and logic used to control each type of sprite 
//  implemented in the game.  It also handles collision detection.  This...
//  is the biggest and most intimidating of all Mega-Mart source code files.
//-----------------------------------------------------------------------------


#include "sprite_manager.h"
#include "hero_manager.h"
#include "bg_manager.h"
#include "resource_manager.h"
#include "dl_manager.h"

// Private Data
#define MAX_SPRITES             50
#define MAX_BLOCK_SPRITES        5
#define MAX_BLINK_SPRITES        5
#define MAX_PROJECTILE_SPRITES   5
#define EXIT_EAST              485  // Destroy sprites with xPos > this value

// If the sprite's collision vlaue is non 0, the collision detection 
// function WILL NOT change the sprite's collision value, nor will it
// examine it for a potential collision
#define COLLISION_HERO_BAT         1
#define COLLISION_HERO_PROJECTILE  2
#define COLLISION_ACKNOWLEDGED   100



SDL_Surface   *_scr;
static Sprite _sprites[MAX_SPRITES];
static Sprite *_projectile[MAX_BLOCK_SPRITES];
static Sprite *_block[MAX_BLOCK_SPRITES];
static Sprite *_blink[MAX_BLINK_SPRITES];
static int    _numBlockSprites;
static int    _numProjectileSprites;
static Sprite *_screenShotTextSpritePtr;
static Sprite *_levelCompleteTextSpritePtr;


// Private Functions
static int  AddBlinkSprite(Sprite *s);
static void RemoveBlinkSprite(Sprite *s);
static int  AddProjectileSprite(Sprite *s);
static void RemoveProjectileSprite(Sprite *s);
static int  AddBlockSprite(Sprite *s);
static void RemoveBlockSprite(Sprite *s);
static void GetBoundingData(int x, int y, SDL_Rect *r, int *xMin, int *xMax, int *yMin, int *yMax, int *mid);
static int  GetNextFreeSprite();
static void AdjustSpritePosition(Sprite *s);
static void ClearProjectileList();
static void ClearSpriteList();
static void ClearBlockList();
static void ClearBlinkList();
static int  UpdateSpritePositionCollision(Sprite *s, float moveBg, unsigned int sfx); 
static int  CollisionOccured(int hx, int hy, SDL_Rect *hr, int sx, int sy, SDL_Rect *sr);

// Private function used to control behavior of specific types of sprites
static int SMC_UpdateEmployeePosition(void * sv, float moveBg);
static int SMC_UpdateBowlingBallPosition(void * sv, float moveBg);
static int SMC_UpdateBouncingBallPosition(void * sv, float moveBg);
static int SMC_UpdateGrillPosition(void * sv, float moveBg);
static int SMC_UpdateTentGuyPosition(void * sv, float moveBg);
static int SMC_UpdatePunchingBagPosition(void * sv, float moveBg);
static int SMC_UpdateBackgroundSpritePosition(void * sv, float moveBg);
static int SMC_UpdateFallingShelfPosition(void * sv, float moveBg);
static int SMC_UpdateScreenShotSprite(void * sv, float moveBg);
static int SMC_UpdatePowerUpPosition(void * sv, float moveBg);
static int SMC_UpdateLevelCompleteSprite(void * sv, float moveBg);
static int SMC_UpdateBombPosition(void * sv, float moveBg);
static int SMC_UpdateArcherPosition(void * sv, float moveBg);
static int SMC_UpdateArrowPosition(void * sv, float moveBg);
static int SMC_UpdateExplosionPosition(void * sv, float moveBg);
static int SMC_UpdateBouncingBombPosition(void * sv, float moveBg);
static int SMC_UpdateBicyclePosition(void * sv, float moveBg);
static int SMC_UpdateSeriesPosition(void * sv, float moveBg);

// Private functions used to initialize different sprites
static int InitTwinkleSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitPowerUpSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitEmployeeSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitBouncingBallSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitGrillSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitTentSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitTentGuySprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitPunchingBagSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitBackgroundSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitRandomShelfSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitFallingShelfSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitBowlingBallSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitBowlingBallShelfSprite(Sprite *s1, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitScreenShot1Sprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitScreenShot2Sprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitLevelCompleteSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitBombSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitSeriesSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitArcherSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitArrowSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitExplosionSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitBouncingBombSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);
static int InitBicycleSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup);


//------------------------------------------------------------------------------
// Name:     SM_Init
// Summary:  Called 1X, initialses Sprite Manager for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void SM_Init()
{
  _scr       = MM_GetScreenPtr();
  ClearSpriteList(); // Initialize lists used to track sprites
  ClearBlockList();
  ClearBlinkList(); 
  ClearProjectileList();
}

//------------------------------------------------------------------------------
// Name:     SM_Init
// Summary:  Called to initialize the specified level.
// Inputs:   Level to initialize
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: Only 1 level exists in the game, but this framework exists in case
//           more levels are added.
//------------------------------------------------------------------------------
int SM_InitLevel(unsigned int level)
{
  int status                  = 0;
  _screenShotTextSpritePtr    = 0;
  _levelCompleteTextSpritePtr = 0;
  ClearSpriteList();
  ClearBlockList();
  ClearBlinkList();
  ClearProjectileList();
  return(status);
}

//------------------------------------------------------------------------------
// Name:     SM_CreateSprite
// Summary:  Generic function used to create a particular sprite
// Inputs:   1. xPos - X position of where sprite will appear on screen
//           2. yPos - Y position of where sprite will appear on screen
//           3. zPos - Order in which sprite is placed in draw list.  Small 
//              values get drawn to screen first, big values get drawn last.  
//           4. id - Which type of sprite to create
//           type - Backgroiund sprite (SM_BACKGROUND) or active (SM_SPRITE)
//           setup - Pointer to a structure that may contain optional 
//           information used to initilize a sprite.  A value of 0 is given if 
//           no additional information is required.
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: If an invlaid/unknown ID is given, it is quitly ignored
//------------------------------------------------------------------------------
int SM_CreateSprite(float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int index = GetNextFreeSprite();
  if (index >= 0)
  {
    if      (id == EMPLOYEE_SPRITE)
      InitEmployeeSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == RM_BOWLING_SHELF_SPRITE)
      InitBowlingBallShelfSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == RM_BLUE_BOWLING_BALL_SPRITE)
      InitBowlingBallSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);      
    else if (id == BASKETBALL_SPRITE || id== BASEBALL_SPRITE || id== SOCCERBALL_SPRITE)
      InitBouncingBallSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == GRILL_SPRITE)
      InitGrillSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id >= TENT_GREEN && id <= TENT_BLUE)
      InitTentSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == PUNCHING_BAG_SPRITE)
      InitPunchingBagSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == GEN_RANDOM_SHELF)
      InitRandomShelfSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == FALLING_SHELF_SPRITE  )
      InitFallingShelfSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);      
    else if (id == RM_IMG_POWER_UP_SPRITE || id == RM_IMG_BONUS_LIFE_SPRITE)
      InitPowerUpSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == RM_IMG_BOMB_SPRITE)
      InitBombSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == RM_IMG_SERIES_SPRITE)
      InitSeriesSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == RM_IMG_ARCHER_SPRITE)
      InitArcherSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == RM_IMG_BOUNCE_BOMB_SPRITE)
      InitBouncingBombSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (id == RM_IMG_BICYCLE_SPRITE || id == RM_IMG_BICYCLE1_SPRITE || id == RM_IMG_BICYCLE2_SPRITE)
      InitBicycleSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else if (type == SM_BACKGROUND)
      InitBackgroundSprite(&_sprites[index], xPos, yPos, zPos, id, type, setup);
    else
      index = -1;
  }
  
  if ( index >= 0 )
  {
    _sprites[index].DrawImage = SM_DrawSprites;
    DL_Add((void*) &_sprites[index]);
  }
  
  return(index);
}

//------------------------------------------------------------------------------
// Name:     SM_AdjustHeroPosition
// Summary:  Function used to allow a sprite to block the hero's path.  This 
//           function will adjust the hero's X or Y location if need be.
// Inputs:   1. yPos - current y position of hero
//           2. mhWidth - Width of hero
//           3. hr - Hero bounding rectangle
// Outputs:  1. xPos - Hero's X position on screen
//           2. Amount that background should be moved
// Returns:  Ground level for hero
// Cautions: Used mainly to have hero block on shelf sprites, but can be used
//           for any type sprite that should be able to block hero's movement
//------------------------------------------------------------------------------
int SM_AdjustHeroPosition(int yPos, int hWidth, 
                          SDL_Rect *hr, float *xPos, float *moveBg)
{
  int retGroundLevel = 0;
  
  // only bother with this intense calculation if blockable sprites are in play
  if (_numBlockSprites)  
  {
    Sprite *s;
    int i;
    int sXMin, sXMax, sYMin, sYMax, sMid;
    int hXMin, hXMax, hYMin, hYMax, hMid;
    int yFlag = 0;
    GetBoundingData((int)*xPos, yPos, hr, &hXMin, &hXMax, &hYMin, &hYMax, &hMid);
    hXMin = (int)*xPos;
    hXMax = (int)*xPos + hWidth;
    hMid  = hXMin + ((hXMax - hXMin) / 2);
    
    
    for (i=0; i < MAX_BLOCK_SPRITES; i++)
    {
      
      if (_block[i] == 0)
      {
        continue;
      }
      else
      {
        yFlag = 0;
        s     = _block[i];
        GetBoundingData((int)s->xPos,  s->yPos, &s->boundRec, 
                        &sXMin, &sXMax, &sYMin, &sYMax, &sMid);
        
        if ( hYMax <= sYMin )
        {          
          yFlag = 1;
          // Hero is above shelf.  To compensate for the 3d effect of the 
          // shelf, move in the bounding edges of the shelf when hero is 
          // above the shelf.  Hopefully this will prevent effect where
          // hero's feet are not on shelf, but he still thinks he is on 
          // the shelf because their bounding rectangles overlap
          sXMin += 25;
          sXMax -= 25;
        }
        
        // Check the hero's x position first, and make sure it is 
        // not currently inside of the solid sprite
        // The hero's x position lies within the sprite
        if ( !(sXMin > hXMax || sXMax < hXMin) ) 
        {
          if (yFlag)
          {
            retGroundLevel = sYMin - hr->h;
          }
          // hero's feet are below the top of the sprite.  The hero is
          // colliding with the sprite, so we must stop his motion and adjust
          // his position, placing him against the immovable object.
          else               
          {
            *moveBg = 0;
            // hero is to the left of sprite, adjust x position based on 
            // sprite bounding rectangles
            if (hMid <= sMid)  
              *xPos = sXMin - hWidth;  
            else
            {
              *xPos = sXMax + 2;
              if (*xPos > HERO_MIDPOINT_EAST)
              {
                *moveBg = *xPos - HERO_MIDPOINT_EAST;
                *xPos   = HERO_MIDPOINT_EAST;
              }
            }
          }
        }
        // if hero's current x position is not inside the solid sprite, check
        // to see if sprite itself should move.  The sprite's movement will
        // depend upon the moveBg value.
        else if ((int)*moveBg)
        {
          sXMin += (-1.0 * (*moveBg));
          sXMax += (-1.0 * (*moveBg));
          sMid  = sXMin + ((sXMax - sXMin) / 2);
          
          // The hero's x position lies within the sprite
          if ( !(sXMin > hXMax || sXMax < hXMin) ) 
          {
            // hero's feet are higher than, or touching the top of the sprite
            if (yFlag) 
            {
              retGroundLevel = sYMin - hr->h;
            }
            // hero's feet are below the top of the sprite.  The hero is 
            // colliding with the sprite, so we must stop his motion and 
            // adjust his position, placing him against the immovable object.
            else               
            {
              // Sprite is to the right of hero
              if ( hMid <= sMid)  
                *moveBg = (s->xPos + s->boundRec.x) - hXMax;
              // This case should not occur in real game play, because 
              // scrolling west will not be allowed
              else  
                *moveBg = -1 * ( hXMin - (s->xPos + s->boundRec.w-5));
            }
          }
        }
      }
    }
  }

  return(retGroundLevel);
}

//------------------------------------------------------------------------------
// Name:     AdjustSpritePosition
// Summary:  Function used to see if a sprite has encountered a blockable 
//           sprite.  Only certain moving sprites who can have their paths 
//           blcoked by things like falling shelves need to implement this 
//           function.
// Inputs:   Pointer to sprite object
// Outputs:  None
// Returns:  None
// Cautions: This function really could have been designed better, but it 
//           works for the few things that need it.
//------------------------------------------------------------------------------
void AdjustSpritePosition(Sprite *s)
{
  // only bother with this intense calculation if blockable sprites are in play
  if (_numBlockSprites)  
  {
    Sprite *b;
    int i;
    int bXMin, bXMax, bYMin, bYMax, bMid;
    int sXMin, sXMax, sYMin, sYMax, sMid;
    GetBoundingData((int)s->xPos,  s->yPos, 
                    &s->boundRec, &sXMin, &sXMax, &sYMin, &sYMax, &sMid);
    
    for (i=0; i < MAX_BLOCK_SPRITES; i++)
    {
      if (_block[i] == 0)  // Examine active blockable objects only
      {
        continue;
      }
      // If we get here, we are examining a blockable sprite
      else                 
      {
        b = _block[i];     // get pointer to it
        GetBoundingData((int)b->xPos,  b->yPos, 
                        &b->boundRec, &bXMin, &bXMax, &bYMin, &bYMax, &bMid);
        
        // Sprite has collided along x Axis with a blockable sprite
        if ( !(bXMin > sXMax || bXMax < sXMin) ) 
        {
          if ( sYMax <= bYMin )  // Sprite is above the blockable sprite
          {                      
            // adjust its ground level to be that off the top of the
            //  blockable sprite
            s->groundLevel = bYMin - s->h;
          }
          // if sprite is not above blockable sprite, it has collided 
          // into the side of it
          else                   
          {                      
            // Change sprites direction and set his ground level to 
            // normal ground level just to be safe
            s->xVel        = s->xVel * -1.0;
            s->curDir      = (s->xVel > 0)?MM_EAST:MM_WEST;
            s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
            // This should not be needed.  Sprite should never move slower
            // than blokcable object
            if (sMid <= bMid)  // sprite is to the left
              s->xPos = b->xPos - s->w - 1;
            else
              s->xPos = b->xPos + b->w+1;
          }
          // Do to design, a sprite can only be in contact with 1 blockable
          //  object at a time.  If this object was found, break out of loop
          break; 
        }
        else 
        {
          // if sprite was not in contact with blockable object, reset 
          // it's ground level just to be safe
          s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
        }  // end inner else
      }    // end outer else
    }      // end for (i=0; i < MAX_BLOCK_SPRITES; i++)
  }        // end if (_numBlockSprites)  
}          // end void AdjustSpritePosition(Sprite *s)
        

//------------------------------------------------------------------------------
// Name:     GetBoundingData
// Summary:  Returns bounding rectangle information for a sprite
// Inputs:   1.  Sprite x position
//           2.  Sprite y position
// Outputs:  X/Y Min/Max positions used to create a bounding rectangle for
//           sprite.  The midpoint of the bounding rectabgle (on x Axis) is 
//           also returned.
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------        
void GetBoundingData(int x, int y, SDL_Rect *r, 
                     int *xMin, int *xMax, int *yMin, int *yMax, int *mid)
{
  *xMin = x + r->x;
  *xMax = x + r->w;
  *yMin = y + r->y;
  *yMax = y + r->h;
  *mid  = *xMin + ((*xMax - *xMin) / 2);
}
 
//------------------------------------------------------------------------------
// Name:     AddBlockSprite
// Summary:  Adds sprite to list of special sprites that can block hero's 
//           movement
// Inputs:   Pointer to sprite
// Outputs:  None
// Returns:  Index of sprites location in blockable list, -1 on error.
// Cautions: None
//------------------------------------------------------------------------------                    
int AddBlockSprite(Sprite *s)
{                    
  int index = -1;    
  int x;             
  for (x=0; x < MAX_BLOCK_SPRITES; x++)
  {
    // find first free spot
    if (_block[x] == 0)
    {
      index = x;
      break;
    }
  }
  
  // Add sprite to list if open spot was found
  if (index >= 0)
  {
    _block[index] = s;
    _numBlockSprites++;
  }
  return(index);
}

//------------------------------------------------------------------------------
// Name:     RemoveBlockSprite
// Summary:  Removes sprite from list of blockable sprites
// Inputs:   Pointer to sprite
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void RemoveBlockSprite(Sprite *s)
{
  int x;
  for (x=0; x < MAX_BLOCK_SPRITES; x++)
  {
    if (_block[x] == s)
    {
      _block[x] = 0;
      _numBlockSprites--;
      break;
    }
  }
}

//------------------------------------------------------------------------------
// Name:     AddBlinkSprite
// Summary:  List of sprites who blink when they are attacked by hero.  Adding 
//           a sprite to the list who is currently blinking ensures it
//           will appear in the screenshot if a screenshot is taken while the 
//           sprite is blinking.
// Inputs:   Pointer to sprite
// Outputs:  None
// Returns:  None
// Cautions: Sprites must add themself and remove themself from this list or 
//           there wiill be serious problems
//------------------------------------------------------------------------------
int AddBlinkSprite(Sprite *s)
{
  int index = -1;    
  int x;             
  for (x=0; x < MAX_BLINK_SPRITES; x++)
  {
    // find first free spot
    if (_blink[x] == 0)
    {
      index = x;
      break;
    }
  }
  
  // Add sprite to list if open spot was found
  if (index >= 0)
    _blink[index] = s;

  return(index);
}

//------------------------------------------------------------------------------
// Name:     RemoveBlinkSprite
// Summary:  Remove a blinking sprite from the list
// Inputs:   Pointer to sprite
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void RemoveBlinkSprite(Sprite *s)
{
  int x;             
  for (x=0; x < MAX_BLINK_SPRITES; x++)
  {
    // find first free spot
    if (_blink[x] == s)
    {
      _blink[x] = 0;
      break;
    }
  }
}

//------------------------------------------------------------------------------
// Name:     AddProjectileSprite
// Summary:  Adds a sprite to the prjectile sprite list.  A projectile sprite
//           is any sprite that can collide with an enemy sprite and destroy 
//           said enemy.  Right now, only bouncing balls hit by hero can be
//           turned into projectile sprites.
// Inputs:   Pointer to sprite
// Outputs:  None
// Returns:  None
// Cautions: If a sprite adds itself to this list, it must also remove itself
//           from this list or very wacky behavior will ensue.
//------------------------------------------------------------------------------
int AddProjectileSprite(Sprite *s)
{
  int index = -1;    
  int x;             
  
  // find a free spot in list
  for (x=0; x < MAX_PROJECTILE_SPRITES; x++)
  {
    // find first free spot
    if (_projectile[x] == 0)
    {
      index = x;
      break;
    }
  }
  
  // Add sprite to list if open spot was found
  if (index >= 0)
  {
    // Increment counter that tracks total number of projectile sprites
    _numProjectileSprites++;
    _projectile[index] = s;
  }

  return(index);
}

//------------------------------------------------------------------------------
// Name:     RemoveProjectileSprite
// Summary:  Remove a blinking sprite from the list
// Inputs:   Pointer to sprite
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void RemoveProjectileSprite(Sprite *s)
{
  int x;             
  
  // Search list for sprite
  for (x=0; x < MAX_PROJECTILE_SPRITES; x++)
  {
    // if sprite is in list (he may not be if list was full when we attempted
    // to add him) remove him from the list
    if (_projectile[x] == s)
    {
      _numProjectileSprites--; // decrement number of active projectile spirtes
      _projectile[x] = 0;      // remove sprite from list
      break;
    }
  }
}

//------------------------------------------------------------------------------
// Name:     SM_ShowBlinkSprites
// Summary:  Sets show flag to 1 for all active sprites in blink list.  This
//           ensures blinking sprites are drawn to screen during a screenshot.
// Inputs:   Pointer to sprite
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void SM_ShowBlinkSprites()
{
  int x;
  for (x=0; x < MAX_BLINK_SPRITES; x++)
  {
    // Force active spites to show up for the current frame
    if (_blink[x] != 0)
    {
      _blink[x]->show = 1;
    }
  }
}

//------------------------------------------------------------------------------
// Name:     SM_DetectCollision
// Summary:  Compares each active sprite's position to that of hero and chacks
//           for: 1) has sprite been attacked by hero, 2) has hero been 
//           attacked by sprite
// Inputs:   None
// Outputs:  None
// Returns:  None - Calls function to alert Hero clas if collison occured.  
//           Also sets internal collison value for any sprite attacked by hero
// Cautions: None
//------------------------------------------------------------------------------
int SM_DetectCollision()
{
  int ret   = 0;
  static int hXPos, hYPos, hWeaponInUse, heroDir;
  static SDL_Rect hBoundRec, hWBoundRec;
  Sprite *s;
  int index;
  
  HM_GetCollisionInfo(&hXPos, &hYPos, &heroDir, &hBoundRec, 
                      &hWeaponInUse, &hWBoundRec);
  
  for (index=0; index < MAX_SPRITES; index++)
  {
    s = &_sprites[index];
    if ( s->active == 0 || s->type == SM_BACKGROUND)
     continue;
    
    // check to see if any sprites are being attacked by hero's weapon
    // Only sprites not allready in a collsion should be checked
    if (s->collision == 0)
    {
      // sprite is attacked by hero's weapon
      if (hWeaponInUse == 2 && CollisionOccured(hXPos, hYPos, &hWBoundRec,
          (int)s->xPos, s->yPos, &s->boundRec) )
      {
        s->collision    = COLLISION_HERO_BAT;
        s->collisionDir = heroDir;
      }
      // Check to see if sprite is attacked by a projectile weapon
      // but only if their are PW currently on active
      else if (_numProjectileSprites)
      {
        int x = 0;
        for (x = 0; x < MAX_PROJECTILE_SPRITES; x++)
        {
          // If a projectile sprite is active at this index, check for
          // collision
          if (_projectile[x] && 
              CollisionOccured(_projectile[x]->xPos, _projectile[x]->yPos, 
              &_projectile[x]->wBoundRec,(int)s->xPos, s->yPos, &s->boundRec) )
          {
            // if a sprite collided with a projectile, set collision flag
            // to 2.  Some sprites may not need to be effected by projectile
            // collision, setting this will let them know collsion type
            s->collision    = COLLISION_HERO_PROJECTILE;
            s->collisionDir = _projectile[x]->curDir;
            break;
          }  // END if(projectile collision)
        }    // END for (All projectiles)
      }      // END if (Projectile sprites exist)
    }        // END if (s->collision == 0)
    
    // check to see if hero has been attacked by an enemy sprite
    if (s->weaponInUse &&
        CollisionOccured(hXPos,        hYPos,   &hBoundRec,
                         (int)s->xPos, s->yPos, &s->wBoundRec) ) 
    {
      if (s->collisionVal)
        HM_SetCollision(s->collisionVal, s->curDir);
      s->collisionHero = 1; 
    }
  }
  return(ret);
}

//------------------------------------------------------------------------------
// Name:     CollisionOccured
// Summary:  checks to see if 2 sprites have collided with each other
// Inputs:   1. Sprite 1's X position, Y position, and bounding rectange info
//           2. Sprite 2's X position, Y position, and bounding rectange info
// Outputs:  None
// Returns:  None - Calls function to alert Hero clas if collison occured.  
//           Also sets internal collison value for any sprite attacked by hero
// Cautions: None
//------------------------------------------------------------------------------
int CollisionOccured(int hx, int hy, SDL_Rect *hr, int sx, int sy, SDL_Rect *sr)
{
   int ret   = 1;
   int hXMin = hx + hr->x;
   int hXMax = hx + hr->w;
   int hYMin = hy + hr->y;
   int hYMax = hy + hr->h;
   
   int sXMin = sx + sr->x;
   int sXMax = sx + sr->w;
   int sYMin = sy + sr->y;
   int sYMax = sy + sr->h;
   
   if ((sXMin > hXMax || sXMax < hXMin) || (sYMin > hYMax || sYMax < hYMin))
     ret = 0; 
   
   return(ret);
}

//------------------------------------------------------------------------------
// Name:     SM_UpdateSpritePositions
// Summary:  Loops through active sprites and updates their on screen position
// Inputs:   Amount that sprite's movement should be offset to account for the
//           scrolling background
// Outputs:  None
// Returns:  None - Calls function to alert Hero clas if collison occured.  
//           Also sets internal collison value for any sprite attacked by hero
// Cautions: Status value of 0 on success, non-zero on error
//------------------------------------------------------------------------------
int SM_UpdateSpritePositions(float moveBg)
{
  int ret   = 0;
  Sprite *s;
  int index;
  
  for ( index=0; index < MAX_SPRITES; index++)
  {
    s = &_sprites[index];
    if ( s->active != 0 )
      ret = s->UpdateSpritePosition((void*) s, moveBg); 
  }

  return(ret);
}

//-----------------------------------------------------------------------------
// Name:     UpdateSpritePositionCollision
// Summary:  Generic function used to update a sprite's position on screen
//           when the sprite has collided with the hero's weapon.  This 
//           will be called instead of the regular routine used to update a
//           sprites position.
// Inputs:   s - the sprite to update
//           moveBg - Denotes if background is moving.  0 measn bg is not 
//                    moving.  > 0 bg is moving East to West, < 0 bg is moving 
//                    west to east.
//           sfx - ID of sound effect to play when collisioon occurs
// Outputs:  None
// Returns:  0 on success, non zero on failure
// Cautions: None
//-----------------------------------------------------------------------------
int UpdateSpritePositionCollision(Sprite *s, float moveBg, unsigned int sfx)
{
  int status = 0;
  
  // If collision flag is not set to acknoleged, the collision just occured so 
  // initialize several variables
  if (s->collision != COLLISION_ACKNOWLEDGED)  
  {
    RM_PlaySound(sfx);    
    // set flag denoting collision variables are set
    s->collision = COLLISION_ACKNOWLEDGED; 
    // Make sprite's direction the same as the hero's direction
    if ( s->collisionDir != s->curDir )
    {
      if (s->xVel > 0)
        s->xVel = -4;
      else
        s->xVel = 4;
    }
    // Set sprites current frame to his regular "standing" frame
    s->curFrm    = s->frmOrder[s->curDir-1][0] * s->h;
    s->yVelCur   = s->yVel; // Set yVel so sprite will jump
    s->isJumping = 1;       // set flag to denote sprite is jumping
    s->weaponInUse = 0;     //Disables this sprites ability to harm others
  } 
  
  // If the background is moving, first adjust the sprite's position based on
  // how much the background has moved
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  // If sprite is moving, and xDelay is reached, upate the sprite's X position
  // NOTE, x delay now defaults to 0, yielding no effect, thus it may be 
  //  removed soon.
  if (s->isMoving && s->xDelCur++ >= s->xDel)
  { 
    s->xPos += s->xVel;         
    s->xDelCur = 0;
  }
  
  // Code to make sprite jump.  
  // yDelay is used to only update sprite's y position every yDel frames
  if (s->isJumping && s->yDelCur >= s->yDel)
  {
    s->yDelCur = 0; 
    // Add Y velocity to sprites current y position, and ensure result is
    // above the ground level.  NOTE: PSP Cordinates top left = (0,0), 
    // bottom left = (0,272).  Thus ground level is around 265.
    if ( s->yPos + s->yVelCur < s->groundLevel)  
    {
      // gravity is a negative number that makes yVelCur decrease every loop.
      s->yPos    += s->yVelCur;
      s->yVelCur += s->gravity; 
    }
    // Now sprite has finished jumping, set flags
    else
    {
      s->yPos      = s->groundLevel;  // ensure sprite lands at ground level
      s->isJumping = 0;               // set longer jumping flag
    }
  }
  // if sprite is not jumping, then the end of his backwards jump is reached.  
  // Let him remain on screen for yDel*3 additional frames so it does not look 
  // like he disapears as soon as he hits the ground
  else if ( s->isJumping == 0 && s->yDelCur >= (s->yDel * 1))
  {
    // when yDel is reached, call function that de-activates sprite
    SM_DestroySprite(s); 
  }
  
  AdjustSpritePosition(s);
  
  s->yDelCur++;
  return(status);
}

//-----------------------------------------------------------------------------
// Name:     SM_DrawSprites
// Summary:  Standard function used to draw an individual sprite
// Inputs:   Void Pointer to sprite object (must be cast to sprite pointer)
// Outputs:  None
// Returns:  0 on success, non zero on failure
// Cautions: This function clips sprit if part of it is off screen (on x Axis)
//           This may not be necessary as SDL may do this automatically.   
//-----------------------------------------------------------------------------
int SM_DrawSprites(void *vs)
{
  int status = 0;
  Sprite *s = (Sprite*) vs;
  static SDL_Rect sprRec = {0, 0, 0, 0};
  static SDL_Rect scrRec = {0, 0, 0, 0};
  
  sprRec.w = s->w;
  sprRec.h = s->h;
  sprRec.x = sprRec.y = scrRec.x = scrRec.y = 0;
  
  // Do not draw sprite if it is completly off screen
  if ( s->active == 0 || (s->xPos + s->img->w ) <  0 || 
       s->xPos > MM_SCREEN_WIDTH || s->show == 0)
    return(status);

  // sprite is exiting West
  if (s->xPos < 0 ) 
  {
    sprRec.x = s->xPos * -1;
    sprRec.w = s->img->w - sprRec.x;
    scrRec.x = 0;
  }
  else if ( (s->xPos + s->img->w) > MM_SCREEN_WIDTH ) // sprite is exiting East
  {
    sprRec.w = MM_SCREEN_WIDTH - s->xPos+1;
    scrRec.x = s->xPos;
  }
  else // sprite is somewhere in the middle of the screen
  {
    scrRec.x  = s->xPos;
  }      
  
  scrRec.y  = s->yPos;
  sprRec.y  = s->curFrm;
  status   += SDL_BlitSurface(s->img, &sprRec, _scr, &scrRec);      
 
 return(status);
}

//-----------------------------------------------------------------------------
// Name:     ClearSpriteList
// Summary:  Resets all sprite structures to free
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void ClearSpriteList()
{
  int x;
  for (x=0; x < MAX_SPRITES; x++)
  {
    _sprites[x].prev   = 0; // probably not needed but done for good measure
    _sprites[x].next   = 0; // probably not needed but done for good measure
    _sprites[x].zPos   = 5; // probably not needed but done for good measure
    _sprites[x].active = 0; // Needed
    _sprites[x].free   = 1; // Needed
  }
}

//-----------------------------------------------------------------------------
// Name:     ClearBlockList
// Summary:  Resets all slots in blockable sprite list to free/open
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void ClearBlockList()
{
  int x;
  _numBlockSprites = 0;
  for (x=0; x < MAX_BLOCK_SPRITES; x++)
  {
    _block[x] = 0;
  }
}

//-----------------------------------------------------------------------------
// Name:     ClearBlinkList
// Summary:  Clears the ShowList
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void ClearBlinkList()
{
  int x;
  for (x=0; x < MAX_BLINK_SPRITES; x++)
    _blink[x] = 0;
}

//-----------------------------------------------------------------------------
// Name:     ClearProjectileList
// Summary:  Clears the Projectile Sprite list
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void ClearProjectileList()
{
  int x;
  for (x=0; x < MAX_PROJECTILE_SPRITES; x++)
    _projectile[x] = 0;
  _numProjectileSprites = 0;
}

//-----------------------------------------------------------------------------
// Name:     SM_DestroySprite
// Summary:  Destroys a sprite. Resets its structure and marks it as free for
//           use.  Also removes the sprite from the Draw List.
// Inputs:   Pointer to sprite structure
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void SM_DestroySprite(Sprite *s)
{
  if (s->free == 0)
  {
    s->free   = 1;
    s->active = 0;  
    DL_Remove((void*) s);
  }
}

//-----------------------------------------------------------------------------
// Name:     SM_ClearList
// Summary:  This is mainly a debug function that is only in use to allow the
//           map_manager class to re-start a level.  See MAP_DebugReset
//           This function will free all active sprites, reset the blockable 
//           sprite list, and remove all active sprites from the draw list
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: This function will not be used in final version of game but will
//           remain in ocde for sentimental reasons
//-----------------------------------------------------------------------------
void SM_ClearList()
{
  Sprite *s;
  int index, x;
  for ( index=0; index < MAX_SPRITES; index++)
  {
    s = &_sprites[index];
    if ( s->active != 0 )
      SM_DestroySprite(s);
  }

  for (x=0; x < MAX_BLOCK_SPRITES; x++)
  {
    _block[x] = 0;
  }
}

//-----------------------------------------------------------------------------
// Name:     GetNextFreeSprite
// Summary:  Loop through static array of free sprites and find the first
//           sprite free for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: This function will not be used in final version of game but will
//           remain in ocde for sentimental reasons
//-----------------------------------------------------------------------------
int GetNextFreeSprite()
{
  int x;
  int ret = -1;
  
  for (x=0; x < MAX_SPRITES; x++)
  {
    if (_sprites[x].free == 1)
    {
      ret = x;
      break; 
    }
  } 
  
  return(ret);
}

// Functions that can be used to temporarily enable/disbale a spite based on 
// it Sprite Manager Assiged ID (or index in the sprite structure).  These
// are more or les debug functiona nd not really used.
void SM_EnableSprite(int eId)  { _sprites[eId].active  = 1; }
void SM_DisableSprite(int eId) { SM_DestroySprite(&_sprites[eId]); }



//-----------------------------------------------------------------------------
// Name:     SMC_Update<SPRITE_NAME>Position
// Summary:  The following functions are callback functions used to update
//           the position of a given sprite.  These functions provide the AI
//           for each sprite.  All Update Position callbacks have the same 
//           prototype which will be be defined here, 1 time.
// Inputs:   1. Void Pointrer that points to a sprite structure
//           2. Amount to adust sprite's poistion to account for scrolling
//              background
// Outputs:  None
// Returns:  0 on success, non zero on failure
// Cautions: Each sprite is responsible for destroying itself when it goes 
//           off screen or is attacked by hero.
//-----------------------------------------------------------------------------

// BEGIN UPDATE POSITION FUNCTIONS
int SMC_UpdateEmployeePosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  
  if (s->collision != 0) // If collsion occured, use standard collision function
  {
    status = UpdateSpritePositionCollision(s, moveBg, RM_SFX_EMPLOYEE_HIT); 
    s->curFrm = s->frmOrder[s->curDir-1][s->frmCount] * s->h;  // collision image
  }
  else
  {
    if ((int)moveBg != 0)
    {
      s->xPos += (-1.0 * moveBg);
    }
    
    // Determine if sprite should move
    if (s->isMoving && s->fDelCur++ >= s->fDel)
    {
      s->fDelCur = 0;
      s->frmIndex++;
      if (s->frmIndex >= s->frmCount)
        s->frmIndex = 0;
    }
    s->curFrm = s->frmOrder[s->curDir-1][s->frmIndex] * s->h;
    
    if (s->isMoving && s->xDelCur++ >= s->xDel)
    { 
      s->xPos += s->xVel;         
      s->xDelCur = 0;
    }
    
    // Adjust sprites X and Y velocity in case it has encountered a blockabel sprite
    AdjustSpritePosition(s);
    
    if (s->xPos < -200)
    {
      s->xPos   = MM_RandomNumberGen(-199, -50);
      s->xVel   = s->xVel * -1.0;
      s->curDir = MM_EAST;
    }
    
    if ((int)s->xPos > MM_SCREEN_WIDTH+200 )
    {
      s->xPos   = MM_SCREEN_WIDTH + MM_RandomNumberGen(50, 199);
      s->xVel   = s->xVel * -1.0;
      s->curDir = MM_WEST;
    }
  }

  return(status);
}

int SMC_UpdateBowlingBallPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }

  // Make sprite start falling down
  if ( s->xPos - HM_GetXPos() < 125 )
   s->isMoving = 1;
  
  if (s->isMoving && s->fDelCur++ >= s->fDel && s->frmIndex < s->frmCount-1)
  {
    s->fDelCur = 0;
    s->frmIndex++;
    
    if      (s->frmIndex == 1)
      s->yPos--;
    else if (s->frmIndex == 2)
      s->yPos--;
    else if (s->frmIndex >= 3 && s->frmIndex <= 10)
    {
      s->weaponInUse = 1;
      s->yPos += s->yVel;
      s->yVel += 2;  
    }
    else if (s->frmIndex == 11)
    {
      s->weaponInUse = 0;  
      s->yPos -= 5;
      RM_PlaySound(RM_SFX_BOWLING_BALL_FALL);
    }
    else if (s->frmIndex == 12)
    {
      s->yPos += 3;  
      s->weaponInUse = 0;  
    }
  }
  
  s->curFrm = s->frmOrder[0][s->frmIndex] * s->h;
  
  // sprite exits stage left
  if (s->xPos < (-1 * s->w))
  {
    SM_DestroySprite(s); 
  }

  return(status);
}

int SMC_UpdateTentGuyPosition(void * sv, float moveBg)
{
  int status = 0;
  Sprite *s  = (Sprite *) sv;
  
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  // If sprite is not moving, check to see if sprite should start moving
  if (s->isMoving == 0)
  {
    // If hero is in range of sprite, proceed
    if (MM_Abs(s->xPos - HM_GetXPos()) < 90)
    {
      // if start flag is 1, go ahead and start moving sequence
      if (s->misc == 1)
      {
        s->collision = 0; // In case collision occured as sprite entered tent
        s->frmIndex  = 0; // ensure frmOrder does not go out of bounds
        s->isMoving  = 1;
        s->show      = 1;
        s->fDelCur   = 0;
        if (s->xVel < 0)
          s->xVel *= -1;
      }
    }
    // If hero is not in range of sprite, set flag denoting it is safe to
    // start sequence when hero is in range.  Using this flag ensures 
    // sprtite only pops out of tent when hero goes from being out of range
    // to being in range.
    else
    {
      s->misc = 1;
    }
  }
  
  // Annimate sprite if he is moving, and not in a collicion state
  if (s->isMoving == 1 && s->fDelCur++ >= s->fDel)
  {
    s->fDelCur   = 0;  // reset current delay to 0
    s->frmIndex += s->xVel; // increment / decrement currnet frame
    
    // If 2nd frame is reached, sprite's weapon must be activated/de-activated
    // Sprite pops out of tent (positive x velocity) actiavte weapon
    // Sprite goes into tent (negative x velocity) de-actiavte weapon
    if (s->frmIndex == 2)
      s->weaponInUse = (s->xVel > 0)?1:0;
    
    // If final frame is reached, change x velocity to a negative value so
    // frame order is reveresed and sprite goes back into tent
    if (s->frmIndex > s->frmCount-1)
    {
      s->xVel     = -1;
      s->frmIndex = s->frmCount-1;
    }
    // if a negative frame is reached, sprite is back inside tent
    else if (s->frmIndex < 0 )
    {
      s->frmIndex = 0; // reset to 1st frame in sequence
      s->isMoving = 0; // reset, checks to see if hero is in attack range
      s->show     = 0; // hide sprite until he starts attacking
      s->misc     = 0; // set flag preventing sprite from attacking

      /*
      // 50/50 chance sprite will pop out of tent multiple times
      if (MM_RandomNumberGen(0, 1))
      {
        s->frmIndex = 0; // reset to 1st frame in sequence
        s->isMoving = 0; // reset, checks to see if hero is in attack range
        s->show     = 0; // hide sprite until he starts attacking
        s->misc     = 0; // set flag preventing sprite from attacking
      }
      else
      {
        s->isMoving = 3; // Set flag to destroy/free sprite
      }
      */
    }
  }
  
  // if a collision occurs in specified range of frames, start the 
  // sprite's collision sequence  This code will execute 1X since
  // it sets frmIndex to frmCount.
  if ( s->collision && s->frmIndex >= 1 && s->frmIndex < s->frmCount)
  {
    AddBlinkSprite(s);
    RM_PlaySound(RM_SFX_TENT_HIT);
    s->cDelCur++;                  // start the collison delay counter
    s->frmIndex    = s->frmCount;  // set current frame to collision frame
    s->weaponInUse = 0;
    s->isMoving    = 2;  // denotes sprite is being destroyed
  }
  
  // if collison delay current is set, handle collison sequence
  if (s->cDelCur)
  {
    // Make image blink if collision is still in progress
    if (s->cDelCur++ < s->cDel)
      s->show = (s->show)?0:1;
    else
    {
      s->isMoving = 3; // end annimation when collision delay expires
      RemoveBlinkSprite(s);
    }
  }
  
  // Set the current frame to the correct frame based on sprites 
  // current direction (tent he pops out if can face east or west, 
  // thus sprite can face east or west).
  s->curFrm = s->frmOrder[s->curDir-1][s->frmIndex] * s->h;
  
  // sprite exits stage left, or sequence is complete
  if (s->xPos < (-1 * s->w) || s->isMoving == 3)
  {
    // remove sprite from blink list just to be safe, if he is not in it,
    // a little time is wasted, nothing more
    RemoveBlinkSprite(s);  
    SM_DestroySprite(s); 
  }

  return(status);
}

int SMC_UpdateFallingShelfPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
    
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  // Make sprite start falling down
  if (s->isMoving == 0 && s->xPos - HM_GetXPos() < 125 )
  {
    s->isMoving = 1;
    if (s->misc == 1)
    {
      s->misc = 0;
      RM_PlaySound(RM_SFX_SHELF_FALL);
    }
  }
  
  if (s->isMoving == 1 && s->fDelCur++ >= s->fDel)
  {
    s->fDelCur = 0;
    s->frmIndex++;
    
    if (s->frmIndex >= s->frmCount)
    {
      s->frmIndex    = s->frmCount-1; // set frame to last frame (indexed at 0)
      s->isMoving    = 2; // Setting to 2 ensures this code runs only once
    }
    else if (s->frmIndex == 8)
    {
      AddBlockSprite(s);  // when last frame is hit, add sprite to list of blockable sprites      
    }
    else if (s->frmIndex >= 5)
    {
      s->weaponInUse = 1;
    }
  }
  
  s->curFrm = s->frmIndex * s->h;
  
  // sprite exits stage left
  if (s->xPos < (-1 * s->w))
  {
    RemoveBlockSprite(s);
    SM_DestroySprite(s); 
  }

  return(status);
}


int SMC_UpdateGrillPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  // Make sprite start falling down
  if (s->xPos - HM_GetXPos() < 125 )
    s->isMoving = 1;
  
  if (s->isMoving && s->fDelCur++ >= s->fDel)
  {
    s->fDelCur = 0;
    s->frmIndex++;
    
    if (s->frmIndex >= 8)
      s->weaponInUse = 1;
   
    if (s->frmIndex == 4)
      RM_PlaySound(RM_SFX_GRILL_FALLING);      
    
    if (s->frmIndex > s->frmCount-1)
    {
      s->frmIndex = 8;
    }
  }
  
  s->curFrm = s->frmIndex * s->h;
  
  // sprite exits stage left
  if (s->xPos < (-1 * s->w))
  {
    SM_DestroySprite(s); 
  }

  return(status);
}


int SMC_UpdateBouncingBallPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  
  if (s->collision == COLLISION_HERO_BAT)  
  {
    RM_PlaySound(RM_SFX_BALL_HIT);
    
    // Sprite is now a projectile that can cause damage to enemies
    AddProjectileSprite(s);
    // Mark sprite as background so no collison detection will take place 
    // between it and the hero
    s->type      = SM_BACKGROUND;  
    s->collision = COLLISION_ACKNOWLEDGED;
    s->xVel      = ((s->xVel>0)?10:-10);

    if ( HM_GetCurrentDir() != s->curDir )
    {
      s->xVel   =  -1.0 * s->xVel;
      s->curDir = (s->curDir==MM_WEST)?MM_EAST:MM_WEST;
    }

    s->gravity = 0;
    s->yVel    = -10;
    if (s->yVelCur < 0)
       s->yVelCur = -10;
    else
       s->yVelCur = 10;
  }
  // If ball was hit by a projectile weapon is should ignore collision.  It
  // should also reset collision value to 0 so the hero will still be able 
  // to hit it.
  else if (s->collision == COLLISION_HERO_PROJECTILE)
  {
    s->collision = 0;
  }
  
    
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }

  if (s->isMoving && s->xDelCur++ >= s->xDel)
  { 
    s->xPos += s->xVel;         
    s->xDelCur = 0;
  }

  if (s->fDelCur++ >= s->fDel)
  { 
    s->fDelCur = 0;
    if (s->xVel <= 0)
      s->frmIndex++;
    else
     s->frmIndex--;
     
    if ( s->frmIndex >= s->numImages)
      s->frmIndex = 0;
    if ( s->frmIndex < 0)
      s->frmIndex = s->numImages-1;      
  }

  if (s->yDelCur++ >= s->yDel)
  {
    s->yDelCur = 0;
    
    if ( s->yPos + s->yVelCur < s->groundLevel )
    {
      s->yPos    += s->yVelCur;
      s->yVelCur += s->gravity;
    }
    else
    {
      s->yPos      = s->groundLevel;
      s->yVelCur   = s->yVel;
    }
  }                                                                                                                                                                                    
  
  // Adjust sprites X and Y velocity in case it has encountered a blockabel sprite
  AdjustSpritePosition(s);
  
  s->curFrm = s->frmIndex * s->h;
  
  // sprite exits stage left or stage right even...
  if (s->xPos < (-1 * s->w) || (int)s->xPos > EXIT_EAST )
  {
    if (s->type == SM_BACKGROUND )
      RemoveProjectileSprite(s);
    SM_DestroySprite(s); 
  }

  return(status);
}

int SMC_UpdateBouncingBombPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }

  // Sprite should ignore projectile collisions
  if (s->collision == COLLISION_HERO_PROJECTILE)
    s->collision = 0;
  
  // Initialize bomb explosion sequence
  if (s->collision == COLLISION_HERO_BAT ||    // Sprite attcaked by hero
     (s->collisionHero && s->collision == 0))  // Sprite hits hero 
  {
    RM_PlaySound(RM_SFX_EXPLOSION);
    s->collision   = COLLISION_ACKNOWLEDGED;  // Denotes explosion taking place
    s->frmIndex    = 8;  // 1st explosion frame
    s->fDelCur     = 0;  
    s->frmCount    = 16; // last explosion frame is 15
    s->weaponInUse = 0;  // sprite weapon disabled
    s->isMoving    = 0;  // sprite no longer moves
  }
  

  if (s->isMoving == 1 && s->xDelCur++ >= s->xDel)
  { 
    s->xPos += s->xVel;         
    s->xDelCur = 0;
  }

  // If no collision is in progress, annimate bouncing bomb sequence
  if (s->collision == 0)
  {
    if (s->fDelCur++ >= s->fDel)
    { 
      s->fDelCur = 0;
      if (s->xVel <= 0)
        s->frmIndex++;
      else
       s->frmIndex--;
       
      if ( s->frmIndex >= s->frmCount)
        s->frmIndex = 0;
      if ( s->frmIndex < 0)
        s->frmIndex = s->frmCount-1;      
    }
    
    if (s->yDelCur++ >= s->yDel)
    {
      s->yDelCur = 0;
      
      if ( s->yPos + s->yVelCur < s->groundLevel )
      {
        s->yPos    += s->yVelCur;
        s->yVelCur += s->gravity;
      }
      else
      {
        s->yPos      = s->groundLevel;
        s->yVelCur   = s->yVel;
      }
    }                                                                                                                                                                                    
    // Adjust X and Y velocity in case sprite encounters a blockabel sprite
    AdjustSpritePosition(s);
  }
  // An explosion is in progress, annimate explosion sequence
  else
  {
    if (s->fDelCur++ >= s->fDel)
    { 
      s->fDelCur = 0;
      s->frmIndex++;
    }
  }

  // calculate frame to draw to screen
  s->curFrm = s->frmIndex * s->h;  
  
  // check to see if sprite should "expire"
  if ((s->frmIndex >= s->frmCount) ||   // last explosion frame reached
      (s->xPos < (-1 * s->w))      ||   // sprite exists stage left
      ((int)s->xPos > EXIT_EAST))       // sprite exists stage right
      SM_DestroySprite(s); 

  return(status);
}

int SMC_UpdatePunchingBagPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  if (s->fDelCur++ >= s->fDel)
  {
    s->fDelCur   = 0;
    s->frmIndex +=  s->misc;
    
    if (s->frmIndex >= s->numImages-1 || s->frmIndex <= 0)
      s->misc *= -1;
      
    s->weaponInUse = ((s->frmIndex >= 7)?1:0);
  }
  
  s->curFrm = s->frmIndex * s->h;
  
  // sprite exits stage left
  if (s->xPos < (-1 * s->w))
  {
    SM_DestroySprite(s); 
  }

  return(status);
}

int SMC_UpdateScreenShotSprite(void * sv, float moveBg)
{
  Sprite *s = (Sprite *) sv;
  if (s->type == 2)
  {
    if (s->fDelCur == SDL_ALPHA_TRANSPARENT)
    {
      // reset to normal alpha value, and to ignore alpha settings
      SDL_SetAlpha(s->img, 0, SDL_ALPHA_OPAQUE); 
      _screenShotTextSpritePtr = 0;
      SM_DestroySprite(s); 
    }
    else 
    {
      SDL_SetAlpha(s->img, SDL_SRCALPHA, s->fDelCur--); 
    }
  }
  else if (s->type == 1 && s->misc == 1)
  {
    s->misc = 0;
    RM_PlaySound(RM_SFX_SCREENSHOT);           
  }
  return(0);  // X and y position do not change!
}


int SMC_UpdatePowerUpPosition(void * sv, float moveBg)
{
  Sprite *s = (Sprite *) sv;
  // If hero has collided with this sprite, his power was updated,
  // so simply destroy this sprite!
  if (s->misc == RM_IMG_TWINKLE_SPRITE)
  {
    if (s->fDelCur++ >= s->fDel)
    {
      s->fDelCur = 0;
      s->frmIndex++;
      
      if (s->frmIndex >= s->numImages)
        s->frmIndex = 0;
      
      s->curFrm = s->frmIndex * s->h;
    }
  }
  
  if (s->collisionHero)
  {
    //Play Sound
    if (s->misc == RM_IMG_POWER_UP_SPRITE || s->misc == RM_IMG_BONUS_LIFE_SPRITE)
    {
      RM_PlaySound(RM_SFX_POWER_UP);       
    }
    SM_DestroySprite(s); 
  }
  else
  {
    // use simple background update function to move this sprite
    SMC_UpdateBackgroundSpritePosition(sv, moveBg); 
  }
  return(0);
}

int SMC_UpdateBombPosition(void * sv, float moveBg)
{
  int status = 0;
  Sprite *s  = (Sprite *) sv;
  
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  s->xPos += s->xVel;

  // Begin explosion sequence, only done 1 time for weaponInUse is set to 
  // 0 when the collision occurs
  if ( s->weaponInUse == 1 && (s->collisionHero || s->curDir == MM_EAST) )
  {
    s->xVel        = 0;
    s->isMoving    = 0; // start explosion annimation sequence
    s->frmIndex    = 8; // first frame of explosion
    s->weaponInUse = 0; 
    RM_PlaySound(RM_SFX_EXPLOSION);
  }
  
  // If moving flag is set, bomb is rolling and has not yet exploded
  if (s->isMoving) // rolling annimation sequence
  {
    if (s->fDelCur++ >= s->fDel)
    {
      s->fDelCur = 0;  // reset current delay to 0
      s->frmIndex++;
      
      // loop back to start frame if last frame is reached
      if (s->frmIndex >= 8)
        s->frmIndex = 0;
    }      
  }
  // if moving flag is not set, bomb is in the process of exploding
  else // explosion annimation sequence
  {
    if (s->fDelCur++ >= s->fDel)
    {
      s->fDelCur = 0;  // reset current delay to 0
      s->frmIndex++;
      
      // This can be used to make the bounding rectangle of the explosion 
      // grow with each frame
      //s->wBoundRec.x  -= 1;
      //s->wBoundRec.y  -= 1;
      //s->wBoundRec.w  += 1;
    }
    // De-activate weapon when last frame is reached
    if (s->frmIndex >= s->frmCount-2)
      s->weaponInUse = 0; 
  }
  
  // Adjust sprites X and Y velocity in case it has encountered a blockabel sprite
  AdjustSpritePosition(s);
  
  // Set current frame to display
  s->curFrm = s->frmIndex * s->h;
  
  // sprite exits stage left, or sequence is complete 
  if (s->xPos < (-1 * s->w) || s->frmIndex >= s->frmCount)
  {
    SM_DestroySprite(s); 
  }

  return(status);
}

int SMC_UpdateArcherPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
    
  if (s->collision != 0) // If collsion occured, use standard collision function
  {
    // Make sprite jump and fly backwords when attacked
    status = UpdateSpritePositionCollision(s, moveBg, RM_SFX_EMPLOYEE_HIT); 
    
    // force collision image
    s->curFrm = s->frmOrder[s->curDir-1][s->frmCount] * s->h;  
    s->fDel   = 3;
  }
  else
  {  
    // Move sprite with background
    if ((int)moveBg != 0)
    {
      s->xPos += (-1.0 * moveBg);
    }
    
    // Make sure sprite allways faces hero
    // We adjust x velocity so the generic jump function will make sprite
    // jump in the proper direction when this sprite is attacked by hero,
    // and so her will jump in proper direction when hit by sprite
    if (s->curDir != MM_EAST && HM_GetXPos() > (s->xPos+40) )
    {
      s->curDir = MM_EAST;
      s->xVel   = 1;
      s->xPos  +=40;
    }
    if (s->curDir != MM_WEST && HM_GetXPos() < (s->xPos-40) )
    {  
        s->curDir = MM_WEST;
        s->xVel   = -1;
        s->xPos  -=40;
    }
    
    // Update sprite's current frame
    if (s->fDelCur++ > s->fDel)
    {
      s->fDelCur = 0;
      s->frmIndex++;
      
      if (s->frmIndex >= s->frmCount)
        s->frmIndex = 0;
      
      // if 8th frame is active, Add arrow sprite that will attack hero
      if (s->frmIndex == 8) 
      {
        RM_PlaySound(RM_SFX_ARROW);
        int dir     = s->curDir;
        int index   = GetNextFreeSprite();
        if (index >= 0)
        {
          int xOffset = (dir==MM_EAST)?80:-14;
          Sprite *s1  = &_sprites[index];
          InitArrowSprite(s1, s->xPos + xOffset, s->yPos + 46, s->zPos + .5, RM_IMG_ARROW_SPRITE, SM_SPRITE, &dir);
          s1->DrawImage = SM_DrawSprites;
          DL_Add((void*) s1);
        }
      }
        
      // Hold the pose if this is the 1st frame
      // otherwise standard delay of 3 frames
      if (s->frmIndex == 0)
        s->fDel = 10;
      else
        s->fDel = 3;
    }
    
    // Set frame that should be drawn to screen
    s->curFrm = s->frmOrder[s->curDir-1][s->frmIndex] * s->h;  
  }
  
  // Adjust sprites X and Y position if it encounters a blockabel sprite
  AdjustSpritePosition(s);
  
  // sprite exits stage left
  if (s->xPos < (-1 * s->w))
  {
    SM_DestroySprite(s); 
  }

  return(status);
}

int SMC_UpdateArrowPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  
  // update sprite relative to background
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  // update sprites x position
  s->xPos += s->xVel;
  
  // ensure sprite only gets destroyed 1X
  // sprite exits stage left
  if (s->xPos < (-1 * s->w))
  {
    SM_DestroySprite(s); 
  }
  // sprite exists stage right (it's possible!)
  else if ((int)s->xPos > EXIT_EAST )
  {
    SM_DestroySprite(s); 
  }
  // If arrow collides with hero, make a big 'splosion!
  else if (s->collisionHero)
  {
    int index = GetNextFreeSprite();

    // ensure a free sprite was found
    if (index >= 0)  
    {
      int xOffset = -25;
    
      // Adjust xPos of expolosion according to arrow's direction
      if (s->curDir == MM_EAST)
        xOffset += s->w;

      // Create new sprite, add it to draw list, delete arrow sprite
      Sprite *s1  = &_sprites[index];
      InitExplosionSprite(s1, s->xPos+xOffset, s->yPos-25, 11, 
                        RM_IMG_BOMB_SPRITE, SM_BACKGROUND, 0);
      s1->DrawImage = SM_DrawSprites;
      DL_Add((void*) s1);
      SM_DestroySprite(s); // destroy arrow sprite
    }
  }

  return(status);
}

static int SMC_UpdateExplosionPosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
    
  // adjust sprite xPos relative to background
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  // Play explosion sound
  if (s->misc == 0)
  {
    s->misc = 1;
    RM_PlaySound(RM_SFX_EXPLOSION);
  }
  
  // Update sprite's current frame
  if (s->fDelCur++ > s->fDel)
  {
    s->fDelCur = 0;
    s->frmIndex++;
    
    // Destroy sprite when last frame has been shows
    if (s->frmIndex >= s->frmCount)
      SM_DestroySprite(s); 
    else
      s->curFrm = s->frmIndex * s->h;  
  }
  return(status);
}

static int SMC_UpdateBicyclePosition(void * sv, float moveBg)
{
  int status = 0;
  Sprite *s  = (Sprite *) sv;
  
  // Ignore projectile collisions
  if (s->collision == COLLISION_HERO_PROJECTILE)
    s->collision = 0;
  
  if (s->collision != 0) // If collsion occured, use standard collision function
  {
    status = UpdateSpritePositionCollision(s, moveBg, RM_SFX_BICYCLE); 
    
    // Set collision image
    s->curFrm = s->frmOrder[s->curDir-1][s->frmCount-1] * s->h;  
  }
  else  // else update sprite position and frame as follows
  {
    if ((int)moveBg != 0)
    {
      s->xPos += (-1.0 * moveBg);
    }
    
    if (s->misc++ == 20)
      RM_PlaySound(RM_SFX_BICYCLE_BELL);
    
    // Determine if sprite should move
    if (s->isMoving)
    {
      if (s->fDelCur++ >= s->fDel)  // increment frame
      {
        s->fDelCur = 0;
        s->frmIndex++;
        if (s->frmIndex >= s->frmCount)
          s->frmIndex = 0;
      }
      s->xPos += s->xVel; // increment x position
    }
    
    // Update frame to be displayed on screen
    s->curFrm = s->frmOrder[s->curDir-1][s->frmIndex] * s->h;
    
    // Adjust sprites X and Y velocity in case it has encountered a blockabel sprite
    AdjustSpritePosition(s);
    
    // sprite exits stage left or right 
    if (s->xPos < (-1 * s->w) || (int)s->xPos > EXIT_EAST )  
       SM_DestroySprite(s); 
  }

  return(status);
}

// Special sprite used to create a series of sprites.  This sprite is never
// seen on screen, but only creates new sprites
static int SMC_UpdateSeriesPosition(void * sv, float moveBg)
{
  int status = 0;
  Sprite *s  = (Sprite *) sv;
  
  // update the sprites position relative to background
  if ((int)moveBg != 0)
    s->xPos += (-1.0 * moveBg);
  
  // if hero passes sprite, activate barage!
  //if ( HM_GetXPos() >= s->xPos )
     //s->isMoving = 1;  // time to start sequence
     
  // If barage is active, and frame delay is reached, proceed
  if ( s->isMoving && s->fDelCur++ >= s->fDel)
  {
    int id     = s->misc;  // misc contains the image ID of sprite
    s->fDelCur = 0;        // reset delay counter
    
    // If sprite has moved off screen, destroy this sprite
    if (s->xPos < ((-1 * s->w)-100))
    {
      SM_DestroySprite(s); 
    }
    // If sprite is still on screen, proceed
    else
    {
      // FrmCount tracks the total number of instances of this sprite that
      // can be created.  Keep creating sprite until that number is reached
      if (s->frmIndex++ < s->frmCount)
      {
        // if basketball sprite ID was passed in, create random bouncing ball
        if (id == BASKETBALL_SPRITE)
          id = MM_RandomNumberGen(BASKETBALL_SPRITE, SOCCERBALL_SPRITE);
         
        SM_CreateSprite(0, 0, 9, id, SM_SPRITE, 0);      
      }
      // Once specified number have been created, destroy sprite
      else
      {
        SM_DestroySprite(s); 
      }
    }
  }
  
  return(status);
}


int SMC_UpdateBackgroundSpritePosition(void * sv, float moveBg)
{
  int status   = 0;
  Sprite *s    = (Sprite *) sv;
  //float g      = BG_GetxPosGlobal();
  //s->xPos = s->xPosTmp - g;
  
    
  if ((int)moveBg != 0)
  {
    s->xPos += (-1.0 * moveBg);
  }
  
  // This is required to ensure that once this sprite stays alligned 
  // correctly with other object next to it whgen it begins to exit 
  // stage left.  When objects first exit, there xPos is negative.  
  // Since we use floats, all values between 1 and -1 return 0, while 
  // this is really 2 different pixels.  Subtracting 1 only 1 time will 
  // ensure this image stays alligned correctly.  IF we do not sbtract
  // 1, it will be 1 pixel off.
  if (s->xPos < 0 && s->curDir)
  {
    s->xPos -= 1;
    s->curDir = 0;
  }
  
  // sprite exits stage left
  if (s->xPos < (-1 * s->w))
  {
    SM_DestroySprite(s); 
  }

  return(status);
}

static int SMC_UpdateLevelCompleteSprite(void * sv, float moveBg)
{
  return(0); 
}


//-----------------------------------------------------------------------------
// Name:     Init<SPRITE_NAME>Sprite
// Summary:  The following functions are used to initialize the given sprite.
//           All Initilization functions have the same prototype which will 
//           be defined here, 1 time.  The paramaters provided to this 
//           function are desrcibed in the SM_CreateSprite function.
// Inputs:   See SM_CreateSprite
// Outputs:  None
// Returns:  0 on success, non zero on failure
// Cautions: Too many to cover.  but a word of warning, sprite structures are 
//           re-used.  It is the responsibility of each init function to
//           initialize the sprite structure member that will be used in the 
//           sprite's corresponding update function.  If you forget to 
//           initialize a key member, "undesirable" behavior may occur.
//-----------------------------------------------------------------------------

// BEGIN INITILIZATION FUNCTIONS
int InitEmployeeSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status   = 0;
  s->numImages = 10;
  s->img       = RM_GetImage(EMPLOYEE_SPRITE);
  s->h = s->img->h / s->numImages;
  s->w = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos    = 478.0;
  s->yPos    = s->groundLevel;
  s->zPos    = zPos;
  s->xDel    = 0;
  s->yDel    = 2;
  s->fDel    = 6;
  s->xDelCur = 0;
  s->yDelCur = 0;
  s->fDelCur = 0;
  s->type    = type;
  
  s->weaponInUse = 1;
  s->isMoving    = 1;
  s->isJumping   = 0;
  s->xVel        = -4.5;
  s->yVel        = -15;
  s->yVelCur     = 0;
  s->gravity     = 2;
  
  s->curDir         = MM_WEST;
  s->frmCount       = 4;
  s->frmIndex       = 0;
  s->frmOrder[0][0] = 0;
  s->frmOrder[0][1] = 1;
  s->frmOrder[0][2] = 2;
  s->frmOrder[0][3] = 3;
  s->frmOrder[0][4] = 8;

  s->frmOrder[1][0] = 4;  
  s->frmOrder[1][1] = 5;
  s->frmOrder[1][2] = 6;
  s->frmOrder[1][3] = 7;
  s->frmOrder[1][4] = 9;
  
  s->show   = 1;
  s->active = 1;
  s->free   = 0;
  s->collision = 0;
  s->collisionVal = -1;
  
  s->boundRec.x = 15;
  s->boundRec.y = 0;
  s->boundRec.w = s->w-15;
  s->boundRec.h = s->h;
  
  s->wBoundRec.x = 15;
  s->wBoundRec.y = 5;
  s->wBoundRec.w = s->w-15;
  s->wBoundRec.h = s->h-5;
 
  s->UpdateSpritePosition = SMC_UpdateEmployeePosition;
  
  
  return(status);
}

static int InitBowlingBallSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status   = 0;
  s->numImages = 7;
  s->img       = RM_GetImage(id);
  s->h = s->img->h / s->numImages;
  s->w = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos    = xPos;
  s->yPos    = yPos;
  s->zPos    = zPos;
  s->xDel    = 0;
  s->yDel    = 2;
  s->fDel    = 2;
  s->xDelCur = 0;
  s->yDelCur = 0;
  s->fDelCur = 0;
  s->type    = type;
  
  s->weaponInUse = 0;
  s->isMoving    = 0;
  s->isJumping   = 0;
  s->xVel        = 0;
  s->yVel        = 12;
  s->yVelCur     = 0;
  s->gravity     = 2;
  
  s->curDir         = 0;
  s->frmCount       = 14;
  s->frmIndex       = 0;
  s->frmOrder[0][0] = 0;
  s->frmOrder[0][1] = 1;
  s->frmOrder[0][2] = 2;
  s->frmOrder[0][3] = 3;
  s->frmOrder[0][4] = 3;
  s->frmOrder[0][5] = 4;
  s->frmOrder[0][6] = 4;
  s->frmOrder[0][7] = 5;
  s->frmOrder[0][8] = 5;
  s->frmOrder[0][9] = 6;
  s->frmOrder[0][10] = 6;
  s->frmOrder[0][11] = 6;
  s->frmOrder[0][12] = 6;
  s->frmOrder[0][13] = 6;

  s->show         = 1;
  s->active       = 1;
  s->free         = 0;
  s->collision    = 1;  // set to 1 so collision checking is not preformed
  s->collisionVal = -1;
                  
  // This boundRec data should be irrelevant, as this sprite cannot be hurt by the hero
  s->boundRec.x   = 0;
  s->boundRec.y   = 0;
  s->boundRec.w   = s->w;
  s->boundRec.h   = s->h;
                  
  // rectangle used to see if hero has been hit by the ball
  s->wBoundRec.x  = 5;
  s->wBoundRec.y  = 5;
  s->wBoundRec.w  = s->w-5;
  s->wBoundRec.h  = s->h-5;
  
  if (s->type == SM_BACKGROUND)
  {
    s->curDir = 1;
    s->curFrm = 0;
    s->UpdateSpritePosition = SMC_UpdateBackgroundSpritePosition;
  }
  else
  {
    s->UpdateSpritePosition = SMC_UpdateBowlingBallPosition;
  }

  return(status);
}

static int InitBouncingBallSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status   = 0;

  //ballID = (MM_RandomNumberGen(0, 1)?BASKETBALL_SPRITE:BASEBALL_SPRITE); 
  if (id == BASKETBALL_SPRITE || id == SOCCERBALL_SPRITE)
  {
    s->numImages = 10;
  }
  else if (id == BASEBALL_SPRITE)
  {
    s->numImages = 5;
  }
  else
  {
    status = 1; 
  } 
  
  if (status == 0)
  {
    int dir      = (MM_RandomNumberGen(0, 1)?-1:1);
    int yVel     = MM_RandomNumberGen(20, 30);
    s->img       = RM_GetImage(id); 
    s->h = s->img->h / s->numImages;
    s->w = s->img->w;
    s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
    s->xPos    = (dir>0)?1:475;  //xPos;
    s->yPos    = s->groundLevel; //yPos;
    s->zPos    = zPos;
    s->xDel    = 0;
    s->yDel    = 2;
    s->fDel    = 3;
    s->xDelCur = 0;
    s->yDelCur = 0;
    s->fDelCur = 0;
    s->type    = type;
    
    s->weaponInUse = 1;
    s->isMoving    = 1;
    s->isJumping   = 1;
    s->xVel        = dir * MM_RandomNumberGen(2, 8);
    s->yVel        = -1 * yVel; //-20;
    s->yVelCur     = s->yVel;
    
    if (yVel < 25)
       s->gravity = MM_RandomNumberGen(5, 7); //2;
    else if ( 25 <= yVel && yVel < 30)
       s->gravity = MM_RandomNumberGen(6, 12); //2;
    else if (30 >= yVel)
       s->gravity = MM_RandomNumberGen(6, 10); //2;
    
    s->curDir         = (dir>0)?MM_EAST:MM_WEST;
    s->frmIndex       = 0;
    s->frmCount       = s->numImages;
    
    s->show         = 1;
    s->active       = 1;
    s->free         = 0;
    s->collision    = 0;  
    s->collisionVal = -1;
    
    s->boundRec.x   = 5;
    s->boundRec.y   = 5;
    s->boundRec.w   = s->w-5;
    s->boundRec.h   = s->h-5;
                    
    // rectangle used to see if hero has been hit by the ball
    s->wBoundRec.x  = 5;
    s->wBoundRec.y  = 5;
    s->wBoundRec.w  = s->w-5;
    s->wBoundRec.h  = s->h-5;
    
    s->UpdateSpritePosition = SMC_UpdateBouncingBallPosition;
  }
  
  return(status);
}

static int InitBouncingBombSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status     = 0;
  int dir        = (MM_RandomNumberGen(0, 1)?-1:1);
  int yVel       = MM_RandomNumberGen(20, 30);
  
  // same image is used fro bouncing bomb and bomb sprite, but bomb sprite
  // ID actually loads the image into memory
  s->img         = RM_GetImage(RM_IMG_BOMB_SPRITE); 
  s->numImages   = 16;
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos        = (dir>0)?1:475;  
  s->yPos        = s->groundLevel; 
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 2;
  s->fDel        = 3;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->type        = type;
  
  s->weaponInUse = 1;
  s->isMoving    = 1;
  s->isJumping   = 1;
  s->xVel        = dir * MM_RandomNumberGen(2, 8);
  s->yVel        = -1 * yVel; 
  s->yVelCur     = s->yVel;
  
  if (yVel < 25)
     s->gravity = MM_RandomNumberGen(5, 7); 
  else if ( 25 <= yVel && yVel < 30)
     s->gravity = MM_RandomNumberGen(6, 12);
  else if (30 >= yVel)
     s->gravity = MM_RandomNumberGen(6, 10);
  
  // 8 frames for rolling/bouncing bomb, 8 frames fro explosion
  s->frmCount       = 8;  
  s->curDir         = (dir>0)?MM_EAST:MM_WEST;
  s->frmIndex       = 0;
  
  s->show           = 1;
  s->active         = 1;
  s->free           = 0;
  // s->collision values and meanings:
  // 0  =bomb bouncing
  // 1  =explosion init (COLLISION_HERO_BAT)
  // 100=explosion in progress (COLLISION_ACKNOWLEDGED)
  s->collision      = 0;  
  s->collisionVal   = -1;
  s->collisionHero  = 0;
  
  s->boundRec.x   = 12;
  s->boundRec.y   = 15;
  s->boundRec.w   = s->w-12;
  s->boundRec.h   = s->h-15;
                  
  // rectangle used to see if hero has been hit by the ball
  s->wBoundRec.x  = 12;
  s->wBoundRec.y  = 15;
  s->wBoundRec.w  = s->w-12;
  s->wBoundRec.h  = s->h-15;
  
  s->UpdateSpritePosition = SMC_UpdateBouncingBombPosition;
 
  return(status);
}

static int InitGrillSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status     = 0;
  s->numImages   = 10;
  s->img         = RM_GetImage(GRILL_SPRITE); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos        = xPos;
  s->yPos        = yPos;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 0;
  s->fDel        = 3;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->type        = type;
  
  s->weaponInUse = 0;
  s->isMoving    = 0;
  s->isJumping   = 0;
  s->xVel        = 0;
  s->yVel        = 0;
  s->yVelCur     = 0;
  
  
  s->curDir         = 0;
  s->frmIndex       = 0;
  s->frmCount       = s->numImages;
  
  s->show         = 1;
  s->active       = 1;
  s->free         = 0;
  s->collision    = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionVal = -1;
  
  s->boundRec.x   = 5;
  s->boundRec.y   = 5;
  s->boundRec.w   = s->w-5;
  s->boundRec.h   = s->h-5;
                  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 15;
  s->wBoundRec.y  = 55;
  s->wBoundRec.w  = s->w-15;
  s->wBoundRec.h  = s->h-50;
  
  if (s->type == SM_BACKGROUND)
  {
    s->curDir = 1;
    s->curFrm = 0;
    s->UpdateSpritePosition = SMC_UpdateBackgroundSpritePosition;
  }
  else
  {
    s->UpdateSpritePosition = SMC_UpdateGrillPosition;
  }
  
  return(status);
}

static int InitTentSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int index  = 0;
  int status = 0;
  int *dir   = (int *) setup;
  int tentGuyXOffset;
  Sprite *s1;
  InitBackgroundSprite(s, xPos, yPos, zPos, id, SM_BACKGROUND, 0);
  
  if (dir)
  {
    s->curDir = *dir;
    s->curDir = (s->curDir == MM_EAST)?MM_EAST:MM_WEST; // Force east/west val
  }
  else
  {
    s->curDir = MM_WEST;
  }
  
  tentGuyXOffset = (s->curDir==MM_WEST)?-5:82;
  s->img         = RM_GetImage(id); 
  s->numImages   = 2;
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;

  if (s->curDir == MM_EAST)
    s->curFrm    = 0;
  else
    s->curFrm    = s->h;    

  // Create tent guy if this tent should be a sprite
  if (type == SM_SPRITE)
  {
    index  = GetNextFreeSprite();
    if (index >= 0)
    {
      s1 = &_sprites[index];
      InitTentGuySprite(s1, xPos+tentGuyXOffset, yPos, 8, TENT_GUY_SPRITE, SM_SPRITE, setup); //-5
      s1->DrawImage = SM_DrawSprites;
      DL_Add((void*) s1);
    }
  }
  
  return(status);
}

static int InitTentGuySprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status = 0;
  int *dir   = (int *) setup;
  
  if (dir)
  {
    s->curDir = *dir;
    s->curDir = (s->curDir == MM_EAST)?MM_EAST:MM_WEST; // Force east/west val
  }
  else
  {
    s->curDir = MM_WEST;
  }
  
  s->numImages   = 12;
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->xPos        = xPos;
  s->yPos        = yPos;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 0;
  s->fDel        = 4;
  s->cDel        = 70;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->cDelCur     = 0;
  s->type        = type;
  
  // 0=Not safe to starts sprite attack sequence
  // 1=Safe to start sprite attack sequence
  s->misc        = 1;
  
  // 0=Not moving and not drawn on screen
  // 1=Moving and popping out of tent
  // 2=Collision is in progress
  // 3=It is time to destroy/free sprite
  s->isMoving    = 0; 
  s->weaponInUse = 0;
  s->isJumping   = 0;
  s->xVel        = 1; // Set to 1 when coming out of tent, -1 when going in
  s->yVel        = 0;
  s->yVelCur     = 0;

  s->frmIndex       = 0;
  s->frmCount       = 5; 
  
  // 6 frames per direction, 5 for annimation, 1 for Collision image
  s->frmOrder[0][0] = 0;
  s->frmOrder[0][1] = 1;
  s->frmOrder[0][2] = 2;
  s->frmOrder[0][3] = 3;
  s->frmOrder[0][4] = 4;
  s->frmOrder[0][5] = 5; // Collision Image
  
  s->frmOrder[1][0] = 6;
  s->frmOrder[1][1] = 7;
  s->frmOrder[1][2] = 8;
  s->frmOrder[1][3] = 9; 
  s->frmOrder[1][4] = 10; 
  s->frmOrder[1][5] = 11; // Collision Image

  s->show         = 0; // do not show image until hero is in range
  s->active       = 1;
  s->free         = 0;
  s->collision    = 0;  
  s->collisionVal = -1;
  
  s->boundRec.y   = 31;
  s->boundRec.h   = 72;
  
  if (s->curDir == MM_WEST)
  {
    s->boundRec.x = 22;
    s->boundRec.w = 70;
  }
  else
  {
    s->boundRec.x = 42;
    s->boundRec.w = 86;
  }
                  
                  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 34;
  s->wBoundRec.y  = 57;
  s->wBoundRec.w  = 44;
  s->wBoundRec.h  = 44;
  
  if (s->curDir == MM_EAST)
  {
    s->wBoundRec.x = 20;
    s->boundRec.x  = 20;
  }
  
  s->UpdateSpritePosition = SMC_UpdateTentGuyPosition;
  return(status);
}


static int InitPunchingBagSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status     = 0;
  s->numImages   = 11;
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos        = xPos;
  s->yPos        = yPos;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 0;
  s->fDel        = 2;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->type        = type;
  
  s->weaponInUse = 0;
  s->misc        = 1; // determines if frmIndex should be incremented or decramented
  s->isJumping   = 0;
  s->xVel        = 0;
  s->yVel        = 0;
  s->yVelCur     = 0;
  
  s->curDir       = 0;  
  s->frmIndex     = 0;
  s->frmCount     = s->numImages;
  
  s->show         = 1;
  s->active       = 1;
  s->free         = 0;
  s->collision    = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionVal = -1;
  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 9;
  s->wBoundRec.y  = 50;
  s->wBoundRec.w  = 41;
  s->wBoundRec.h  = 100;
  
  if (s->type == SM_BACKGROUND)
  {
    s->curFrm = 5 * s->h;
    s->UpdateSpritePosition = SMC_UpdateBackgroundSpritePosition;
  }
  else
  {
    s->UpdateSpritePosition = SMC_UpdatePunchingBagPosition;
  }
  
  
  return(status);
}


static int InitBackgroundSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status     = 0;
  s->xPosTmp     = xPos;
  s->numImages   = 1;
  s->img         = RM_GetImage(id); 
  s->xPos        = xPos;
  s->yPos        = yPos;
  s->zPos        = zPos;
  s->type        = type;  // sprite is a background object
  s->curFrm      = 0;     // allways draw the first frame
  s->curDir      = 1;     // indicates sprite has not started to exit stage left yet
  s->show        = 1;
  s->active      = 1;     
  s->free        = 0;
  s->weaponInUse = 0;
  s->UpdateSpritePosition = SMC_UpdateBackgroundSpritePosition;
  if (s->img)
  {
    s->h        = s->img->h;
    s->w        = s->img->w;
  }
  
  return(status);
}

static int InitScreenShot1Sprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status;
  status  = InitBackgroundSprite(s, xPos, yPos, zPos, id, type, setup);
  s->xPos = (MM_SCREEN_WIDTH/2) - (s->w/2) - 10;
  s->yPos = 5; 
  s->zPos = 11.2;
  s->type = 1;  // sprite will not use the alpha fading feature
  s->misc = 1;  // 1 denotes screenshot sound has not yet been played 
  s->UpdateSpritePosition = SMC_UpdateScreenShotSprite;
  _screenShotTextSpritePtr   = s;
  return(status);
}

static int InitScreenShot2Sprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status;
  status     = InitBackgroundSprite(s, xPos, yPos, zPos, id, type, setup);
  s->xPos    = (MM_SCREEN_WIDTH/2) - (s->w/2) - 10;
  s->yPos    = 5; 
  s->zPos    = 11.1;
  s->type    = 2;                // sprite will use the alpha fading feature
  s->fDelCur = SDL_ALPHA_OPAQUE; // Initial tranparnetcy value for sprite's surface (Opaque)
  s->cDelCur = 0;                // counter used to descide when to decriment alpha value
  s->UpdateSpritePosition = SMC_UpdateScreenShotSprite;
  _screenShotTextSpritePtr   = s;
  return(status);
}

static int InitRandomShelfSprite(Sprite *s1, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status = 0;
  Sprite *s2 = 0;
  Sprite *s3 = 0;
  Sprite *s4 = 0;
  int index;
   
  InitBackgroundSprite(s1, xPos, yPos, zPos, id, type, setup);
  
  // Create sprites used for remaining 3 shelf layers
  index = GetNextFreeSprite();
  if (index >= 0)
  {
    s2 = &_sprites[index];
    InitBackgroundSprite(s2, xPos, yPos, zPos, GEN_SHELF_B_SPRITE, type, setup);
  }
  
  index = GetNextFreeSprite();
  if (index >= 0)
  {
    s3 = &_sprites[index];
    InitBackgroundSprite(s3, xPos, yPos, zPos, GEN_SHELF_C_SPRITE, type, setup);
  }
  
  index = GetNextFreeSprite();
  if (index >= 0)
  {
    s4 = &_sprites[index];
    InitBackgroundSprite(s4, xPos, yPos, zPos, GEN_SHELF_D_SPRITE, type, setup);
  }
  
  s1->h      = s1->img->h / NUM_SHELF_SETS;
  s1->curFrm = s1->h * MM_RandomNumberGen(0, NUM_SHELF_SETS-1);
  
  if (s2 && s3 && s4)
  {
    s2->h = s2->img->h / NUM_SHELF_SETS;
    s3->h = s3->img->h / NUM_SHELF_SETS;
    s4->h = s4->img->h / NUM_SHELF_SETS;
    
    
    s2->curFrm = s2->h * MM_RandomNumberGen(0, NUM_SHELF_SETS-1);
    s3->curFrm = s3->h * MM_RandomNumberGen(0, NUM_SHELF_SETS-1);
    s4->curFrm = s4->h * MM_RandomNumberGen(0, NUM_SHELF_SETS-1);
    
    s2->yPos = s1->h + s1->yPos;
    s3->yPos = s2->h + s2->yPos;
    s4->yPos = s3->h + s3->yPos;
    
    // must add shelve layers 2-4 here, layer 1 will be added by return function
    s2->DrawImage = SM_DrawSprites;
    s3->DrawImage = SM_DrawSprites;
    s4->DrawImage = SM_DrawSprites;
    DL_Add((void*) s2);
    DL_Add((void*) s3);
    DL_Add((void*) s4);
  }
  
  return(status);
}

static int InitBowlingBallShelfSprite(Sprite *s1, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status  = 0;
  int index;
  int x;
  int r;
  Sprite *s2 = 0;
  Sprite *s3 = 0;
  Sprite *s4 = 0; 
  Sprite *b  = 0;
  
     
  // the passed in sprite will be a simple image of a top shelf that is empty
  InitBackgroundSprite(s1, xPos, yPos, zPos, id, SM_BACKGROUND, setup);
  
  // Create bottom 3 shelf layers
  index = GetNextFreeSprite();
  if (index >= 0)
  {
    s2 = &_sprites[index];
    InitBackgroundSprite(s2, xPos, yPos, zPos, GEN_SHELF_B_SPRITE, SM_BACKGROUND, setup);
  }
  
  index = GetNextFreeSprite();
  if (index >= 0)
  {
    s3 = &_sprites[index];
    InitBackgroundSprite(s3, xPos, yPos, zPos, GEN_SHELF_C_SPRITE, SM_BACKGROUND, setup);
  }
  
  index = GetNextFreeSprite();
  if (index >= 0)
  {
    s4 = &_sprites[index];
    InitBackgroundSprite(s4, xPos, yPos, zPos, GEN_SHELF_D_SPRITE, SM_BACKGROUND, setup);
  }
  
  if (s2 && s3 && s4)
  {
    s2->h = s2->img->h / NUM_SHELF_SETS;
    s3->h = s3->img->h / NUM_SHELF_SETS;
    s4->h = s4->img->h / NUM_SHELF_SETS;
    
    // pick a random shelf image from each shelf layer
    s2->curFrm = s2->h * MM_RandomNumberGen(0, NUM_SHELF_SETS-1);
    s3->curFrm = s3->h * MM_RandomNumberGen(0, NUM_SHELF_SETS-1);
    s4->curFrm = s4->h * MM_RandomNumberGen(0, NUM_SHELF_SETS-1);
    
    s2->yPos = s1->h + s1->yPos;
    s3->yPos = s2->h + s2->yPos;
    s4->yPos = s3->h + s3->yPos;
    
    // must add shelve layers 2-4 here, layer 1 will be added by return function
    s2->DrawImage = SM_DrawSprites;
    s3->DrawImage = SM_DrawSprites;
    s4->DrawImage = SM_DrawSprites;
    DL_Add((void*) s2);
    DL_Add((void*) s3);
    DL_Add((void*) s4);
    
    // Create 3 bowling balls
    for (x=0; x<3; x++)
    {
      index = GetNextFreeSprite();
      if (index >= 0)
      {
        b = &_sprites[index];
        if (type == SM_BACKGROUND)  // inactive bowling shelf
        {
          id = MM_RandomNumberGen(RM_RED_BOWLING_BALL_SPRITE, RM_BLUE_BOWLING_BALL_SPRITE);
          InitBowlingBallSprite(b, xPos, yPos+16, zPos+.5, id, SM_BACKGROUND, 0);
        }
        else  // Active bowling shelf (balls fall randomly)
        {
          id = MM_RandomNumberGen(RM_RED_BOWLING_BALL_SPRITE, RM_BLUE_BOWLING_BALL_SPRITE);
          r  = MM_RandomNumberGen(1, 3);
          if (r == 3) // init as a background sprite
            InitBowlingBallSprite(b, xPos, yPos+16, zPos+.5, id, SM_BACKGROUND, 0);
          else        // init as a moving sprite
            InitBowlingBallSprite(b, xPos, yPos+16, zPos+.5, id, SM_SPRITE, 0);
        }
        b->xPos = xPos + 15 + (x * b->w) + (x*5);  // 15 pixels in from edge, 20 pixels betwen each ball
        b->DrawImage = SM_DrawSprites;
        DL_Add((void*) b);  // add each newly created ball to draw list
      }
    }
  }
  
  return(status);
}

static int InitFallingShelfSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status     = 0;
  s->numImages   = 11;
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos        = xPos;
  s->yPos        = yPos;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 0;
  s->fDel        = 4;
  s->cDel        = 0;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->cDelCur     = 0;
  s->type        = type;
  s->misc        = 1;     // set to 0 after sound gets played
  
  s->weaponInUse = 0;
  s->isMoving    = 0;
  s->isJumping   = 0;
  s->xVel        = 1;
  s->yVel        = 0;
  s->yVelCur     = 0;
  
  
  s->curDir       = 0;
  s->frmIndex     = 0;
  s->frmCount     = s->numImages;
  
  s->show         = 1;
  s->active       = 1;
  s->free         = 0;
  s->collision    = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionVal = -1;
  
  s->boundRec.x   = 4;
  s->boundRec.w   = s->w - s->boundRec.x;
  s->boundRec.y   = 150;  // bigger = hero lower on screen.  Smaller = hero higher on screen.
  s->boundRec.h   = s->h - s->boundRec.y;
                  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 33;
  s->wBoundRec.w  = s->w - s->wBoundRec.x;
  s->wBoundRec.y  = 160;
  s->wBoundRec.h  = s->h;// - s->boundRec.y;
  
  if (s->type == SM_BACKGROUND)
  {
    s->curDir = 1;
    s->curFrm = 0;
    s->UpdateSpritePosition = SMC_UpdateBackgroundSpritePosition;
  }
  else
  {
    s->UpdateSpritePosition = SMC_UpdateFallingShelfPosition;
  }
  return(status);
}

static int InitPowerUpSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status = 0;
  int index  = 0;
  Sprite *s1 = 0;

  if (id == RM_IMG_BONUS_LIFE_SPRITE) // extra life sprite
  {
    s->numImages    = 1;
    s->collisionVal = 100;   // 100 means give hero a bonus life
  }
  else  // RM_IMG_POWER_UP_SPRITE - Increase health sprite
  {
    s->numImages    = 6;
    s->collisionVal = 1;   // 1 means increase hero's health
  }
  
  s->img           = RM_GetImage(id); 
  s->h             = s->img->h / s->numImages;
  s->w             = s->img->w;
  s->groundLevel   = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos          = xPos;
  s->yPos          = yPos;
  s->zPos          = zPos;
  s->type          = type;
  s->weaponInUse   = 1;
  s->show          = 1;
  s->active        = 1;
  s->free          = 0;
  s->collision     = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionHero = 0;   // This gets set to 1 when hero is hit by this sprite
  s->misc          = id;
  
  if (id == RM_IMG_BONUS_LIFE_SPRITE) // extra life sprite
    s->curFrm = 0;
  else  // RM_IMG_POWER_UP_SPRITE - Increase health sprite
    s->curFrm = MM_RandomNumberGen(0, 5) * s->h;
  
  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 0;
  s->wBoundRec.w  = s->w-10;
  s->wBoundRec.y  = 0;
  s->wBoundRec.h  = s->h;
  
  s->UpdateSpritePosition = SMC_UpdatePowerUpPosition;
  
  index = GetNextFreeSprite();
  if (index >= 0)
  {
    s1 = &_sprites[index];
    InitTwinkleSprite(s1, xPos, yPos, zPos+.1, RM_IMG_TWINKLE_SPRITE, type, setup);
    s1->DrawImage = SM_DrawSprites;
    DL_Add((void*) s1);
  }
  
  return(status);
  
}

static int InitTwinkleSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status = 0;
 
  s->numImages     = 3;
  s->img           = RM_GetImage(id); 
  s->h             = s->img->h / s->numImages;
  s->w             = s->img->w;
  s->groundLevel   = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos          = xPos;
  s->yPos          = yPos;
  s->zPos          = zPos;
  s->type          = type;
  s->curFrm        = 0;
  s->frmIndex      = 0;
  s->fDel          = 5;
  s->fDelCur       = 0;
  s->weaponInUse   = 1;
  s->show          = 1;
  s->active        = 1;
  s->free          = 0;
  s->collision     = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionVal  = 0;   // No effect on hero, but sprite will be notified when hero collides with it
  s->collisionHero = 0;   // This gets set to 1 when hero is hit by this sprite
  s->misc          = id; 
  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 0;
  s->wBoundRec.w  = s->w-10;
  s->wBoundRec.y  = 0;
  s->wBoundRec.h  = s->h;
 
  s->UpdateSpritePosition = SMC_UpdatePowerUpPosition;
 
  return(status);
  
}

static int InitBombSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status     = 0;
  
  s->numImages   = 16;
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->xPos        = 479;
  s->yPos        = MM_SCREEN_HEIGHT - s->h;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 0;
  s->fDel        = 3;
  s->cDel        = 0;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->cDelCur     = 0;
  
  // Setting to 0 ensures hero gets thrown in proper direction if a 
  // collison occurs
  s->curDir      = 0;    
  
  s->weaponInUse = 1;
  s->isMoving    = 1;  // 1 = bomb rolling seq, 0 = bomb exploding
  s->xVel        = -3;

  // frmOrder not used in this sequence, we just use the frame index instead
  s->frmIndex    = 0;
  s->frmCount    = 16; 
  
  s->type         = type;
  s->show         = 1;
  s->active       = 1;
  s->free         = 0;
  s->collision    = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionVal = -1;
  s->collisionHero = 0;   // alerts if hero contacts bomb
  
  
  // rectangle used to see if bomb has rolled into a blockable sprite
  s->boundRec.x  = 11;
  s->boundRec.y  = 15;
  s->boundRec.w  = s->w - 11;
  s->boundRec.h  = s->h - 15;
  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 20;
  s->wBoundRec.y  = 15;
  s->wBoundRec.w  = s->w - 20;
  s->wBoundRec.h  = s->h - 15;
  
  s->UpdateSpritePosition = SMC_UpdateBombPosition;

  return(status);
}

static int InitSeriesSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status  = 0;
  int *info   = (int*) setup;
  
  // Use passed in info to determine the type of sprite series (0 or 1)
  // And the image to display for the given series
  if (info)
  {
    s->misc     = info[0];  // ID of image to use for this series
    s->frmCount = (unsigned char) info[1];  // number of sprites to create
    s->fDel     = (unsigned short) info[2];  // del before creating next sprite
  }
  // suplly default values in case things didnt't work out
  else  
  {
    s->misc     = RM_IMG_BOUNCE_BOMB_SPRITE;
    s->frmCount = 10;
    s->fDel     = 25;
  }
  
  // This sprite is used to create a series of the sepcified sprite
  // The image for this sprite is never displayed, but it still needed
  // since the standard DrawSprite function will try to reference it
  //  We can arbitrarily use the bomb sprite for this purpose
  s->img          = RM_GetImage(RM_IMG_BOMB_SPRITE);
  s->h            = 1;
  s->w            = 1;
  s->xPos         = xPos;
  s->yPos         = 1;
  s->zPos         = 1;
  s->type         = SM_BACKGROUND; // set to BG to avoid col detection
  s->show         = 0;
  s->active       = 1;
  s->free         = 0;
  s->weaponInUse  = 0;    // Sprite has no weapon
  s->collision    = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionVal = 0;
  
  s->isMoving = 1;
  //s->isMoving    = 0;        // Set to 1 when hero reaches sprite
  s->fDelCur     = s->fDel;  // Current frame delay (reset after reaching fDel)
  s->frmIndex    = 0;        // Number of bombs created (stop at frmCount)
  
  s->UpdateSpritePosition = SMC_UpdateSeriesPosition;
  
  return(status);
}

static int InitArcherSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status     = 0;
  
  s->numImages   = 24;
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  s->xPos        = xPos;
  s->yPos        = s->groundLevel;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 2;  // used to annimate death sequence
  s->fDel        = 3;  // overriden in Update Poistion Callback
  s->cDel        = 0;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->cDelCur     = 0;
  
  s->curDir      = MM_WEST;    
  s->weaponInUse = 1;
  s->isMoving    = 0;  
  s->isJumping   = 0;
  s->xVel        = -1;  // ensures sprite jumps in correct dir when attacked
  s->yVel        = -15; // used in death sequence
  s->yVelCur     = 0;
  s->gravity     = 2;
  s->type        = type;

  // frmOrder not used in this sequence, we just use the frame index instead
  s->frmIndex        = 0;
  s->frmCount        = 11;  // 11 frames facing east, 11 frames facing west
  s->frmOrder[0][0]  = 0;
  s->frmOrder[0][1]  = 1;
  s->frmOrder[0][2]  = 2;
  s->frmOrder[0][3]  = 3;
  s->frmOrder[0][4]  = 4;
  s->frmOrder[0][5]  = 5; 
  s->frmOrder[0][6]  = 6;
  s->frmOrder[0][7]  = 7;
  s->frmOrder[0][8]  = 8;
  s->frmOrder[0][9]  = 9;
  s->frmOrder[0][10] = 10;
  s->frmOrder[0][11] = 11; // Collision Image (EAST)
  
  s->frmOrder[1][0]  = 12;
  s->frmOrder[1][1]  = 13;
  s->frmOrder[1][2]  = 14;
  s->frmOrder[1][3]  = 15;
  s->frmOrder[1][4]  = 16;
  s->frmOrder[1][5]  = 17;
  s->frmOrder[1][6]  = 18;
  s->frmOrder[1][7]  = 19;
  s->frmOrder[1][8]  = 20;
  s->frmOrder[1][9]  = 21;
  s->frmOrder[1][10] = 22;
  s->frmOrder[1][11] = 23; // Collision Image (WEST)
  
  s->show         = 1;
  s->active       = 1;
  s->free         = 0;
  s->collision    = 0;    
  s->collisionVal = -1;
  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 25;
  s->wBoundRec.y  = 20;
  s->wBoundRec.w  = s->w - 25;
  s->wBoundRec.h  = s->h - 10;
  
  s->boundRec.x   = 30;
  s->boundRec.y   = 10;
  s->boundRec.w   = s->w - 30;
  s->boundRec.h   = s->h - 10;
  
  s->UpdateSpritePosition = SMC_UpdateArcherPosition;

  return(status);
}

static int InitArrowSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status = 0;
  int *info  = (int*) setup;
  
  // Only place this sprite can be created from is the archer sprites update 
  // function.  Thus the archer is in charge of passing in this value and 
  // freeing it if needed
  if (info)
  {
    s->curDir = *info;
    s->curDir = (s->curDir == MM_EAST)?MM_EAST:MM_WEST; // Force east/west val
  }  
  // suplly default value
  else
  {
    s->curDir = MM_WEST;
  } 
   
  s->numImages   = 2;
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->xPos        = xPos;
  s->yPos        = yPos;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 0;
  s->fDel        = 0;  // overriden in Update Poistion Callback
  s->cDel        = 0;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->cDelCur     = 0;
  s->type        = type;
  
  s->weaponInUse = 1;
  s->isMoving    = 1;  
  s->isJumping   = 0;
  
  if (s->curDir == MM_EAST)
  {
    s->curFrm      = 0;  // first frame faces east
    s->xVel        = 7;  
    s->wBoundRec.x = s->w - 15;
    s->wBoundRec.w = s->w;
    
  }
  else
  {
    s->curFrm      = s->h;  // second frame faces west
    s->xVel        = -5;    
    s->wBoundRec.x = 0;
    s->wBoundRec.w = 15;
  }
  
  s->yVel        = 0;
  s->yVelCur     = 0;
  
  s->show          = 1;
  s->active        = 1;
  s->free          = 0;
  s->collision     = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
  s->collisionVal  = -1;
  s->collisionHero = 0;
  
  // rectangle used to see if hero has been hit
  // X cordinates are filled out above
  s->wBoundRec.y  = 0;
  s->wBoundRec.h  = s->h;
  
  s->UpdateSpritePosition = SMC_UpdateArrowPosition;

  return(status);
}

static int InitExplosionSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status = 0;
  
  s->numImages   = 16;  // firts 8 images are of bomb rolling
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->xPos        = xPos;
  s->yPos        = yPos;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->yDel        = 0;
  s->fDel        = 3;
  s->cDel        = 0;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->cDelCur     = 0;
  s->misc        = 0;  // 0=SFX not played, 1=SFX played
  
  s->weaponInUse = 0;  // sprite does not have a weapon
  s->frmIndex    = 8;  // first explosion frame
  s->frmCount    = 16; // last explosion frame
  s->curFrm      = s->frmIndex * s->h;  // start at first explosion frame
  
  s->type         = type;
  s->show         = 1;
  s->active       = 1;
  s->free         = 0;
  s->collision    = COLLISION_ACKNOWLEDGED;  // sprite ignores hero attacks
 
  
  s->UpdateSpritePosition = SMC_UpdateExplosionPosition;
  return(status);
}

int InitBicycleSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status = 0;
  int *dir   = (int *) setup;
  
  // If id was set to generic bicycle image, randomly chose a 
  // bicycle to display (1=normal or 2=wheelie)
  if (id == RM_IMG_BICYCLE_SPRITE)
  {
    // if random, lean toward wheelie bicycle
    if (MM_RandomNumberGen(0, 2) == 1)  
      id = RM_IMG_BICYCLE1_SPRITE;
    else
      id = RM_IMG_BICYCLE2_SPRITE;
  }
  
  if (dir)
  {
    s->curDir = *dir;
    s->curDir = (s->curDir == MM_EAST)?MM_EAST:MM_WEST; // Force east/west val
  }
  else
  {
    // ensure bicycle comes from the west most of the time
    s->curDir = (MM_RandomNumberGen(0, 3)==1)?MM_EAST:MM_WEST;       
  }
  
  s->numImages   = 4;
  s->img         = RM_GetImage(id); 
  s->h           = s->img->h / s->numImages;
  s->w           = s->img->w;
  s->groundLevel = MM_SCREEN_HEIGHT - s->h - 10;
  if (yPos)
    s->yPos      = yPos;
  else
    s->yPos      = s->groundLevel;
  s->zPos        = zPos;
  s->xDel        = 0;
  s->fDel        = 4;
  s->cDel        = 0;
  s->xDelCur     = 0;
  s->yDelCur     = 0;
  s->fDelCur     = 0;
  s->cDelCur     = 0;
  s->misc        = 0;  // SFX is played when value hits 20
  
  if (s->curDir == MM_EAST)
  {
    s->xPos = (-1 * s->w) + 3;  
    s->xVel = 6.5;
  }
  else
  {
    s->xPos = 478;
    s->xVel = -5;
  }

  // Used to make sprite jump when hit by hero
  s->yDel           = 2;
  s->isJumping      = 0;
  s->yVel           = -15;
  s->yVelCur        = 0;
  s->gravity        = 2;
                    
  s->weaponInUse    = 1;
  s->isMoving       = 1; 

  s->type           = type;
  s->show           = 1;
  s->active         = 1;
  s->free           = 0;
  s->collision      = 0;   
  s->collisionVal   = -1;
  
  s->frmCount       = 2;
  s->frmIndex       = 0;
  s->frmOrder[0][0] = 0;
  s->frmOrder[0][1] = 1;
  s->frmOrder[1][0] = 2;  
  s->frmOrder[1][1] = 3;
  
  // rectangle used to see if hero has been hit
  s->wBoundRec.x  = 10;
  s->wBoundRec.y  = 15;
  s->wBoundRec.w  = s->w - 10;
  s->wBoundRec.h  = s->h - 10;
  
  s->boundRec.x   = 10;
  s->boundRec.y   = 10;
  s->boundRec.w   = s->w - 10;
  s->boundRec.h   = s->h - 10;
  
  if (s->type == SM_BACKGROUND)
  {
    s->xPos   = xPos;
    s->curFrm = s->frmOrder[(s->curDir-1)][0] * s->h;
    s->UpdateSpritePosition = SMC_UpdateBackgroundSpritePosition;
  }
  else
  {
    s->UpdateSpritePosition = SMC_UpdateBicyclePosition;
  }

  return(status);
}

static int InitLevelCompleteSprite(Sprite *s, float xPos, int yPos, float zPos, int id, int type, void *setup)
{
  int status;
  status = InitBackgroundSprite(s, xPos, yPos, zPos, id, type, setup);
  s->h   = s->img->h / 2;
  s->UpdateSpritePosition  = SMC_UpdateLevelCompleteSprite;
  return(status);
}

// Special interface functions that are used to create special sprites from
// from within other classes

//-----------------------------------------------------------------------------
// Name:     SM_CreateScreenshotSprite
// Summary:  Interface to special function that is used by main to create the
//           sprite that gets displayed on screen when a screenhot is in
//           progress
// Inputs:   ID - Id of sprite to create (in-progress or complete text)
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void SM_CreateScreenshotSprite(unsigned int id)
{
  int index = GetNextFreeSprite();
  
  // Verify a free sprite was available
  if (index >= 0)
  {
    // Initialize in-progress or complete sprite
    if (id == RM_SCREENSHOT_1_TXT)
      InitScreenShot1Sprite(&_sprites[index], 0, 0, 0, id, SM_BACKGROUND, 0);
    else if (id == RM_SCREENSHOT_2_TXT)
      InitScreenShot2Sprite(&_sprites[index], 0, 0, 0, id, SM_BACKGROUND, 0);
    else
      index = -1;
  }
  
  // Add to draw list if above went ok
  if ( index >= 0 )
  {
    _sprites[index].DrawImage = SM_DrawSprites;
    DL_Add((void*) &_sprites[index]);
  }
}

//-----------------------------------------------------------------------------
// Name:     SM_DestroyScreenShotText
// Summary:  Interface to special function that is used by main to remove
//           the screenshot text sprites from the screen if they are currently 
//           being displayed
//           progress
// Inputs:   ID - Id of sprite to create (in-progress or complete text)
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void SM_DestroyScreenShotText()
{
  SDL_Surface *s1 = RM_GetImage(RM_SCREENSHOT_1_TXT); 
  SDL_Surface *s2 = RM_GetImage(RM_SCREENSHOT_2_TXT); 
  if (_screenShotTextSpritePtr && 
      (_screenShotTextSpritePtr->img == s1 || _screenShotTextSpritePtr->img == s2))
  {
    SM_DestroySprite(_screenShotTextSpritePtr);
    _screenShotTextSpritePtr = 0;
  }
}

//-----------------------------------------------------------------------------
// Name:     SM_CreateLevelCompleteSprite
// Summary:  Interface to special function that is used by hero_manager to 
//           create the sprite that gets displayed on screen whenlevel 1 is 
//           completed.
// Inputs:   Frame number.  A value of 1 displays first text message, a value 
//           of 2 displays second text message.
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void SM_CreateLevelCompleteSprite(unsigned int frm)
{
  // Create sprite if it does not allready exist
  if (_levelCompleteTextSpritePtr == 0)
  {
    int index = GetNextFreeSprite();
    if (index >= 0)  // Verify a free sprite was available
    {
      // store sprite pointer into global variable
      _levelCompleteTextSpritePtr = &_sprites[index];
      // itilize sprite, add it to draw list
      InitLevelCompleteSprite(_levelCompleteTextSpritePtr, 0, 0, 12, 
                              RM_LEVEL1_COMPLETE_TXT, SM_BACKGROUND, 0);  
      _sprites[index].DrawImage = SM_DrawSprites;
      DL_Add((void*) &_sprites[index]);
    }
  }

  // If sprite allready exists, set its frame
  if (_levelCompleteTextSpritePtr)
  {
    if (frm == 0) // Says level complete
    {
      _levelCompleteTextSpritePtr->curFrm = 0;
    }
    else // says WEEEEE
    {
      _levelCompleteTextSpritePtr->curFrm  = _levelCompleteTextSpritePtr->h;
    }
  }
}

//-----------------------------------------------------------------------------
// Name:     SM_CreateRandomSprite
// Summary:  Interface to special function that is used by Hero Manager class
//           to create a random moving sprite if the hero sits still for too 
//           long (gotta shake things up a bit)
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void SM_CreateRandomSprite()
{
   #define NUM_RAND_SPRITES 5
   int id = MM_RandomNumberGen(0, NUM_RAND_SPRITES-1);
   static int map[NUM_RAND_SPRITES] = { 
                                        0,
                                        EMPLOYEE_SPRITE, 
                                        RM_IMG_BOUNCE_BOMB_SPRITE, 
                                        RM_IMG_BOMB_SPRITE,
                                        RM_IMG_BICYCLE_SPRITE
                                      };
   if (id == 0)
     id = MM_RandomNumberGen(BASKETBALL_SPRITE, SOCCERBALL_SPRITE);
   else
     id = map[id];     
   
   SM_CreateSprite(0, 0, 9, id, SM_SPRITE, 0);
}



