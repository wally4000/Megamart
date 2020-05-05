//-----------------------------------------------------------------------------
//  Class:
//  Hero Manager
//
//  Description:
//  This Class implements the hero sprite (Logan Bogan).  It updates his 
//  position and processes the controlls eneterd by the user used to guide our
//  hero along his journey
//-----------------------------------------------------------------------------


#include "hero_manager.h"
#include "bg_manager.h"
#include "power_manager.h"
#include "SDL_image.h"
#include "resource_manager.h"
#include "dl_manager.h"
#include "sprite_manager.h"

// Private Functions
static void  UpdateHeroDeathSequence();
static void  UpdateHeroBeginRegenerate();
static void  UpdateHeroRegenerate();
static void  UpdateHeroHurt();
static void  UpdateHeroFinishLevel();
static int   InitHero();
static float UpdateHeroPosition();
static int   DrawHero(void *hv);
static void  CollisionOverride();

// Private Data 
typedef float (*HM_HeroUpdatePositionCallback) ();
typedef int   (*HM_HeroDrawCallback) ();

typedef struct HERO_SPRITE_STRUCT
{

  // ************************** Lined List Structure **************************
  // First 3 elements of sprite structure must be these 3 or Draw List 
  // Class will not work correctly
  void *prev;
  void *next;
  float zPos;
  MM_DrawImageFunction DrawImage;
  // **************************************************************************
  
  int numImages;
  SDL_Surface *img, *curImg;
  SDL_Surface *duckImg;
  SDL_Surface *jumpImg;
  SDL_Surface *deathImg;
  SDL_Surface *collisionImg;
  SDL_Surface *weaponImg;
  SDL_Surface *victoryImg;
  Mix_Chunk *hit;
  int h;
  int w;
  float xPos;
  int yPos;
  int xDel;
  int yDel;
  int fDel;
  int cDel;
  int walkFDel;
  int runFDel;
  int isDucking;
      
  int xDelCur;
  int yDelCur;
  int fDelCur;
  int cDelCur;
  
  int isJumping;
  int isMoving;
  int isRunning;
  float xVel;
  int yVel;
  int yVelCur;
  int gravity;
  int groundLevel;
  int groundLevelCur;
  int jmpFrm;
  int curImgFrm;
  int show;
  
  int hasWeapon;
  int weaponInUse;
  int weaponFrm;
  int weaponFrmCnt;
  int weaponDir;
  int weaponJitter;
  int weaponYOffset;
  int weaponXOffsetEast;
  int weaponXOffsetWest;
  int weaponFrmDel;
  int weaponDelCur;
  
  short collision;
  unsigned char collisionDir;
  int collisionInProgress;
  unsigned char colDir;
  int curDir;
  int frmCount;
  int frmIndex;
  int frmOrder[2][8];
  HM_HeroUpdatePositionCallback UpdatePositionCallback;
  
  SDL_Rect wSrcRec, wDstRec, boundRec;
  SDL_Rect wBoundRec[2];
  
} HeroSprite;

// collision staes of hero
#define HERO_HURT                1
#define HERO_CONTINUE            2
#define HERO_GAME_OVER           3
#define HERO_BEGIN_REGENERATE    4
#define HERO_REGENERATE          5
#define HERO_BEGIN_FINISH_LEVEL  6
#define HERO_FINISH_LEVEL        7

static SDL_Surface *_scr;
static HeroSprite  _hero;
static HeroSprite  _heroOverride;
static HeroSprite  *_hcp;
static int _scrollWestAllowed;
static int _isVulnerable;
static int _idleHeroTime;
static int _idleHeroTimeLimit;

//------------------------------------------------------------------------------
// Name:     HM_Init
// Summary:  Called 1 time to initilize Hero Manager Class
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void HM_Init()   
{
  _scr                       = MM_GetScreenPtr();
  _heroOverride.weaponImg    = _hero.weaponImg    = 0;
  _heroOverride.collisionImg = _hero.collisionImg = 0;
  _isVulnerable              = 1;
  _hcp = 0;
}
   
//------------------------------------------------------------------------------
// Name:     HM_InitLevel
// Summary:  Initilizes Hero Manager for the specified level
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int HM_InitLevel(int level)
{
  int status         = 0;
  _scrollWestAllowed = 0;
  _hcp               = &_hero;
  _idleHeroTime      = 0;
  _idleHeroTimeLimit = MM_RandomNumberGen(200, 600);

  // If vulnerability flag is set to 2, make hero invulnerable by setting
  // flag to 0
  if (_isVulnerable == 2)  
    _isVulnerable = 0;
  // any other value (only 0/1 should be allowed), just make hero vulnerable
  else  
    _isVulnerable = 1; 
  
  if      ( level == MM_LEVEL1 )
  {
    // hero cannot be added to draw list from within InitHero function.  This 
    // is beacuse we need to call init function before hero regeneration 
    // process begins, and we don't want the init function to add hero to 
    // the draw list 2x.
    status = InitHero();
    DL_Add((void *) &_hero);
  }
  else if ( level == MM_LEVEL2 )
  {
    status = 2;
  }
  else if ( level == MM_LEVEL3 )
  {
    status = 2;
  }
  else if ( level == MM_LEVEL4 )
  {
    status = 2;
  }
  else
  {
    status = 1; 
  }
  
  return(status); 
}

// Wrapper around Update hero callback function.  If we want multiple hero's
// this function can allways be called by main to get access to the current
// hero structure's update position function
float HM_UpdateHeroPosition() {  return(_hero.UpdatePositionCallback());  }


//------------------------------------------------------------------------------
// Name:     HM_<Movement_Functons)
// Summary:  Functions used to make hero respond to user controlls.  The _hcp
//           pointer is used in allfunctions instead of the global _hero 
//           structure.  This allows us to use a temporary structure when we
//           want to disable user controlls over the hero (like when a collision 
//           occurs or the death sequence takes place).  Once the given sequence
//           is over, the user input values will have been tracked in the 
//           temprorary structure and can be used to update the hero's movkent 
//           when the user reghains control.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void HM_MoveLeft()
{
  _idleHeroTime  = 0;
  _hcp->frmIndex = 0;
  _hcp->isMoving = _hcp->curDir = MM_WEST;
  if (_hcp->xVel > 0)
    _hcp->xVel = _hcp->xVel * -1;
    
  if (_hcp->isDucking == 1)
    _hcp->isMoving = 0;
}

void HM_MoveRight()
{
  _idleHeroTime  = 0;
  _hcp->frmIndex = 0;
  // Set flag denoting sprite is moving
  _hcp->isMoving = _hcp->curDir = MM_EAST;         
  if (_hcp->xVel < 0)       // update sprite direction if necessary
    _hcp->xVel = _hcp->xVel * -1;  
  
  if (_hcp->isDucking == 1)
    _hcp->isMoving = 0;
  
}

void HM_StopMoving()
{
  if (_hcp->isDucking != 1)
  {
    _hcp->isMoving = 0;
    _hcp->xDelCur  = 0;
    _hcp->frmIndex = 1;
    _hcp->fDelCur  = _hcp->fDel - 1; 
  }
}

void HM_StartRunning()          
{
  _hcp->fDel      = _hcp->runFDel;
  _hcp->isRunning = 1;
  _hcp->xVel      = (_hcp->xVel > 0)?6.1:-6.1;
  if (_hcp == &_hero)
    BG_SetCeilingFloorSpeed(_hcp->isRunning);
}

void HM_StopRunning()          
{
  _hcp->fDel      = _hcp->walkFDel;
  _hcp->xVel      = (_hcp->xVel > 0)?4.5:-4.5;
  _hcp->isRunning = 0;
  if (_hcp == &_hero)
    BG_SetCeilingFloorSpeed(_hcp->isRunning);
}

void HM_Jump()          
{
  // set jumping velocity if not allready jumping
  if (_hcp->isJumping == 0 && _hcp->isDucking == 0)   
  { 
    if (_hcp == &_hero)  // only play SFX if user has control of hero
      RM_PlaySound(RM_SFX_HERO_JUMP);
    _hcp->yVelCur   = _hcp->yVel;
    _hcp->isJumping = 1;      // it's allways safe to set jump to true
    _hcp->curImg    = _hcp->jumpImg;
    _hcp->weaponYOffset     = -11;
  }
}

void HM_Duck()
{
  if (_hcp->isDucking == 0 && _hcp->isJumping == 0 )
  {
    HM_StopMoving();
    _hcp->isDucking      = 1;
    _hcp->curImg         = _hcp->duckImg;
    _hcp->frmOrder[0][0] = 0;
    _hcp->frmOrder[1][0] = 1;
    _hcp->frmCount       = 1;
    _hcp->frmIndex       = 0;
    _hcp->weaponYOffset  = 15;
    _hcp->boundRec.y     = 60;
  }
  else if (_hcp->isJumping == 1)
  {
    _hcp->isDucking = 2;
  }
}

void HM_StopDuck()
{
  if (_hcp->isDucking == 1)
  {
    _hcp->curImg         = _hcp->img;
    _hcp->frmOrder[0][0] = 0;
    _hcp->frmOrder[1][0] = 4;
    _hcp->frmCount       = 4;
    _hcp->frmIndex       = 1;
    _hcp->weaponYOffset  = -5;
    _hcp->boundRec.y     = 0;
  }
  _hcp->isDucking      = 0;
}

void HM_UseWeapon()          
{
  if (_hcp->hasWeapon)
    _hcp->weaponInUse = 1;
}

//------------------------------------------------------------------------------
// Name:     HM_GetCollisionInfo
// Summary:  Returns the private info regarding the hero's bounding rectangle 
//           information to the Sprite Manager class
// Inputs:   None
// Outputs:  All parametrs to this function
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void HM_GetCollisionInfo(int *hXPos, int *hYPos, int *heroDir, 
                         SDL_Rect *hBoundRec, int *hWeaponInUse, 
                         SDL_Rect *hWBoundRec)
{
  *hXPos        = (int) _hero.xPos;
  *hYPos        = _hero.yPos;
  *heroDir      = _hero.curDir;
  *hBoundRec    = _hero.boundRec;
  *hWeaponInUse = _hero.weaponInUse;
  *hWBoundRec   = _hero.wBoundRec[_hero.curDir-1];
}

//------------------------------------------------------------------------------
// Name:     HM_SetCollision
// Summary:  Allows Sprite Manager to set the collison infoamtion when hero is 
//           attacked
// Inputs:   1.  Amount of health to add / remove from hero
//           2.  Direction of movement of sprite who attacked hero ( 0 if sprite
//               does not move EAST or WEST)
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void HM_SetCollision(short val, unsigned char dir) 
  { _hero.collision = val; _hero.collisionDir = dir;}

// Simple interface function to allow sprite Manager class to know her's 
// current direction.  Mainly used to Sprite's can be thrown in proper direction 
//when hero attacks them
int  HM_GetCurrentDir()                            
  { return(_hero.curDir);  }


//------------------------------------------------------------------------------
// Name:     InitHero
// Summary:  Initilzes hero
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int InitHero()
{
  int retStatus    = 0;
  int status       = 0;

  _hero.img          = RM_GetImage(RM_IMG_HERO);
  _hero.collisionImg = RM_GetImage(RM_IMG_HERO_HURT);
  _hero.deathImg     = RM_GetImage(RM_IMG_HERO_DEATH);
  _hero.duckImg      = RM_GetImage(RM_IMG_HERO_DUCK);
  _hero.jumpImg      = RM_GetImage(RM_IMG_HERO_JUMP);
  _hero.weaponImg    = RM_GetImage(RM_IMG_HERO_WEAPON);

  // temporary until a victory image is created!
  _hero.victoryImg   = RM_GetImage(RM_IMG_HERO_VICTORY);

  if (_hero.img)
  {
    _hero.curImg                 = _hero.img;
    _hero.UpdatePositionCallback = UpdateHeroPosition; 
    _hero.DrawImage              = DrawHero;

    if (status == 0)
    {
      _hero.numImages = 8;
      _hero.h = _hero.img->h / _hero.numImages;
      _hero.w = _hero.img->w;
      _hero.groundLevel = MM_SCREEN_HEIGHT - _hero.h - 10;
      _hero.groundLevelCur = _hero.groundLevel;
      _hero.xPos = (float) _hero.img->w;
      _hero.yPos = _hero.groundLevelCur;
      _hero.zPos = 11;
      _hero.isDucking = 0;
      _hero.xDel = 0;
      _hero.yDel = 1;
      _hero.walkFDel = 6;
      _hero.runFDel  = 3;
      _hero.fDel = 6;
      _hero.cDel = 80;
      
      _hero.xDelCur = 0;
      _hero.yDelCur = 0;
      _hero.fDelCur = 0;
      _hero.cDelCur = 0;
      
      _hero.isMoving  = 0;
      _hero.isRunning = 0;
      _hero.isJumping = 0;
      _hero.xVel      = 4.5;
      _hero.yVel      = -17;
      _hero.yVelCur   = 0;
      _hero.gravity   = 2;
      _hero.show      = 1;
      
      _hero.collisionInProgress = 0;
      _hero.collision      = 0; 
      _hero.curImgFrm      = 0;
      _hero.jmpFrm         = 0;
      _hero.curDir         = MM_EAST;
      _hero.frmCount       = 4;
      _hero.frmIndex       = 0;
      _hero.frmOrder[0][0] = 0;
      _hero.frmOrder[0][1] = 1;
      _hero.frmOrder[0][2] = 2;
      _hero.frmOrder[0][3] = 3;
      
      _hero.frmOrder[1][0] = 4;
      _hero.frmOrder[1][1] = 5;
      _hero.frmOrder[1][2] = 6;
      _hero.frmOrder[1][3] = 7;
      
      _hero.weaponDir    = 1;
      _hero.weaponFrmDel = 0;
      _hero.weaponDelCur = 0;
      _hero.weaponJitter = 1;
      _hero.weaponInUse = 0;
      _hero.weaponFrm = 0;
      _hero.hasWeapon = 1;
      _hero.weaponYOffset = -5;
      _hero.weaponXOffsetEast = 9;
      _hero.weaponXOffsetWest = -42; // was 44
      _hero.weaponFrmCnt = 10;
      _hero.wSrcRec.x = 0;
      _hero.wSrcRec.y = 0;
      _hero.wSrcRec.w = _hero.weaponImg->w;
      _hero.wSrcRec.h = _hero.weaponImg->h / _hero.weaponFrmCnt;
      
      _hero.wDstRec.x = _hero.xPos + (_hero.w / 2);
      _hero.wDstRec.y = _hero.yPos;
      _hero.wDstRec.w = 0;
      _hero.wDstRec.h = 0;
      
      // Rectangle used to see if hero has been hit by enemy
      _hero.boundRec.x = 10;
      _hero.boundRec.y = 0;
      _hero.boundRec.w = _hero.w-10;
      _hero.boundRec.h = _hero.h;
      
      // East
      _hero.wBoundRec[0].x = (_hero.w/4) * 3; // MIN offset from xPos
      _hero.wBoundRec[0].w = _hero.wBoundRec[0].x + 75; // MAX offset from xPos
      _hero.wBoundRec[0].y = 0;
      _hero.wBoundRec[0].h = 90; 
      
      // West
      _hero.wBoundRec[1].w = _hero.w/4;  // MAX offset from xPos  
      _hero.wBoundRec[1].x = - 30; // MIN offset from xPos
      _hero.wBoundRec[1].y = 0;
      _hero.wBoundRec[1].h = 90; 
    }
    else
    {
      retStatus = 2; 
    }
  }
  else
  {
    retStatus = 1;
  }
  return(retStatus);
}

//------------------------------------------------------------------------------
// Name:     UpdateHeroHurt
// Summary:  Updates hero's porition when hero has been attacked by a sprite
//           This annimates the sequence where the hero flies backwards 
//           through the air.  During this time, the user cannot control the
//           hero's movements.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void UpdateHeroHurt()
{
  // return control to user when jump seq is over, but hero is 
  // still invulnerable
  if (_hero.isJumping == 0 && _hcp != &_hero)  
  {
    _hero.curImg    = _hero.img;
    _hero.hasWeapon = 1;  // Re-Activate hero's weapon
    _hcp            = &_hero;
    _hero.isMoving  = _heroOverride.isMoving;
    _hero.curDir    = _heroOverride.curDir;
    _hero.xVel      = _heroOverride.xVel;
    _hero.isRunning = _heroOverride.isRunning;
    _hero.yVel      = _heroOverride.yVel;
    _hero.fDelCur   = 0;
    if (_heroOverride.isDucking)
      HM_Duck();
      
    BG_SetCeilingFloorSpeed(_hero.isRunning);
  }
  // reset variables and make hero vulnerable again when collsion is over
  else if ( _hero.cDelCur++ > _hero.cDel) 
  {
    _hero.cDelCur   = 0;
    _hero.show      = 1;
    // reset in case any additional col occured while sprite was invulnerable
    _hero.collision = 0;  
    _hero.collisionInProgress = 0;
  }
  // Make hero blink whilst he is still invulnerable
  // collision still in progress
  else if (_hcp == &_hero && (_hero.cDelCur % 3) == 0) 
  {
    _hero.show = 0;
  }
  else if (_hcp == &_hero)
  {
    _hero.show = 1;
  }
}

//------------------------------------------------------------------------------
// Name:     UpdateHeroRegenerate
// Summary:  Makes hero re-appear after he dies.  Hero falls down from top of
//            screen and is invulnerable for a brief period of time
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void UpdateHeroRegenerate()
{
  // return control to user when jump seq is over, but hero is 
  // still invulnerable
  if (_hero.isJumping == 0 && _hcp != &_hero)  
  {
    _hero.curImg    = _hero.img;
    _hero.hasWeapon = 1;  // Re-Activate hero's weapon
    _hcp            = &_hero;
    _hero.isMoving  = _heroOverride.isMoving;
    _hero.curDir    = _heroOverride.curDir;
    _hero.xVel      = _heroOverride.xVel;
    _hero.isRunning = _heroOverride.isRunning;
    _hero.yVel      = _heroOverride.yVel;
    _hero.fDelCur   = 0;
    BG_SetCeilingFloorSpeed(_hero.isRunning);
  }
  
  if ( _hero.cDelCur++ > _hero.cDel) 
  {
    // reset in case col occured while sprite was invulnerable
    _hero.collision = 0;  
    _hero.cDelCur   = 0;
    _hero.show      = 1;
    _hero.collisionInProgress = 0;
  }
  // Make hero blink whilst he is still invulnerable, but not while jumping
  else if ((_hero.cDelCur % 2) == 0) 
  {
    _hero.show = _hero.show?0:1;      
  }  
}

void UpdateHeroBeginRegenerate()
{
  PM_ResetHealth();
  InitHero();
  _hero.collisionInProgress = HERO_REGENERATE;
  _hero.cDelCur   = 0;
  _hero.isJumping = 1;
  _hero.xPos      = 75;
  _hero.yPos      = 0 - _hero.h;
  _hero.yVelCur   = 0;
}

//------------------------------------------------------------------------------
// Name:     UpdateHeroDeathSequence
// Summary:  Annimates hero's death sequence, determines if game should 
//           contunie or end (if extra lifes are present or not).  User control
//           is overriden during this sequence.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void UpdateHeroDeathSequence()
{
  if (_hero.isJumping == 0) // if jump sequence is over
  {
    // check to see if this is the first frame after jump sequence
    // if it is, the cur image will not yet be set to the death image
    // sequence.  A jump sequence will ALLWAYS preceed the death sequence
    if (_hero.curImg != _hero.deathImg)  
    {
      // initialze death sequence
      _hero.curImg         = _hero.deathImg;
      _hero.cDelCur        = 0;
      _hero.cDel           = 85;
      _hero.isMoving       = 0;
      _hero.frmOrder[0][0] = 0; // frame seq. for death is slightly different
      _hero.frmOrder[0][1] = 2;
      _hero.frmOrder[0][2] = 3;
      _hero.frmOrder[0][3] = 4;
      _hero.frmOrder[0][4] = 5;
      
      _hero.frmOrder[1][0] = 1;
      _hero.frmOrder[1][1] = 2;
      _hero.frmOrder[1][2] = 3;
      _hero.frmOrder[1][3] = 4;
      _hero.frmOrder[1][4] = 5;
      _hero.frmIndex       = 0;  
      _hero.hasWeapon      = 0;  // hide hero's weapon
      _hero.numImages      = 6;
      _hero.frmCount       = 5;
      // hero's dimensions are different for this sequence, which is the 
      // reason for the total hacks needed below
      _hero.h              = _hero.curImg->h / _hero.numImages;
      _hero.w              = _hero.curImg->w;
      _hero.fDel           = 13;
      _hero.fDelCur        = 0;
      // **************** BEGIN TOTAL HACK SECTION ****************
      // This ensures the SM_AdjustHeroPosition returns a new 
      // ground level position, and does not adjust the hero's X position
      _hero.boundRec.h     = 0;   
      // This ensures the hero's y velocity is not updated
      _hero.yDelCur        = -1;  
      // This ensures the new image of our hero is displayed at the 
      // proper Y location on the screen (Hero's feet are located
      // 40 pixels lower in death sequence images)
      _hero.yPos           = _hero.groundLevelCur - 45; 
      // **************** END TOTAL HACK SECTION ****************
      _hero.curImgFrm = 
              _hero.frmOrder[_hero.curDir-1][_hero.frmIndex] * _hero.h;
    }
    else // update frame in death sequence
    {
      if (_hero.fDelCur++ >= _hero.fDel)  // increment frame
      {
        _hero.fDelCur = 0;
        _hero.frmIndex++;
        if (_hero.frmIndex >= _hero.frmCount) // set to last frame
        {
          _hero.frmIndex = _hero.frmCount - 1;
        }
      }
      if (_hero.frmIndex == (_hero.frmCount-1) ) // if at last frame
      {
        // Hold final frame in for cDel frames (for drametic effect)
        if (_hero.cDelCur++ > _hero.cDel) 
        {
          // If we are in game over state, set flag to end the game
          if (_hero.collisionInProgress == HERO_GAME_OVER)
            MM_SetGameState(MM_STATE_GAME_OVER);    
          else  // else, set flag to enter the hero regeneration state
            _hero.collisionInProgress = HERO_BEGIN_REGENERATE;
        }
      }
      // allways set the current frame just to be safe
      _hero.curImgFrm = 
          _hero.frmOrder[_hero.curDir-1][_hero.frmIndex] * _hero.h;
    }
  }
}

//------------------------------------------------------------------------------
// Name:     UpdateHeroFinishLevel
// Summary:  Annimates sequence that takes place when level is complete.  
//           User control is blocked out.  Also changes game state when 
//           sequence is complete so main knows its time to move on to final 
//           level.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void UpdateHeroFinishLevel()
{
  // if hero is in a jump, stop his forward motion and wait for jump 
  // sequence to finish
  if (_hero.isJumping) 
  {
    // remove control from user, and halt hero's x velocity
    _hcp       = &_heroOverride;
    _hero.xVel = 0;
  }
  // If not in a jump sequqnce, and we are in HERO_BEGIN_FINISH_LEVEL mode
  // initialize variables needed to enter finish level state 
  else if (_hero.collisionInProgress == HERO_BEGIN_FINISH_LEVEL)
  {
    // create Dialog at top of screen
    PM_DisableDrawing();  // Make PM stop drawing extra lives and health
    SM_CreateLevelCompleteSprite(0); // 0 value means first dialog frame
    // disable generate used to create sprites when hero does not move
    // for X frames.  We set the incactivity time limit to a really big value.
    _idleHeroTimeLimit        = 1000000;  
    _hero.collisionInProgress = HERO_FINISH_LEVEL;
    _hcp              = &_heroOverride;
    _hero.curImg      = _hero.victoryImg;
    _hero.curImgFrm   = 0;   // only 1 image is in victory image (VI)
    _hero.hasWeapon   = 0;   // weapon is contained in VI
    _hero.xVel        = 0; 
    _hero.cDel        = 260; // delay used to hold current pose
    _hero.cDelCur     = 0; 
    _hero.isMoving    = 0; 
    _hero.show        = 1;  // make sure we show hero
    _hero.w           = _hero.img->w;    // adjust height/width if needed
    _hero.h           = _hero.curImg->h; // VI may have different dimensions
    _hero.yPos        = MM_SCREEN_HEIGHT - _hero.h - 10;
  }
  // If current image is victory image, execute code to have hero hold pose
  // for X frames
  else if (_hero.curImg == _hero.victoryImg)
  {
    // after holding pose for X frames, initilize code that will make hero
    // exit stage right
    if ( _hero.cDelCur++ > _hero.cDel)
    {
      // Create next set of dialog at top of screen
      SM_CreateLevelCompleteSprite(1); // 1 value means second frame of dialog
      _hero.cDel      = 120; // how long to hold empty screen before
      _hero.cDelCur   = 0;   // changing game state
      _hero.hasWeapon = 1;  
      _hero.curDir    = MM_EAST;
      _hero.frmIndex  = 0;
      _hero.fDelCur   = 0;
      _hero.xVel      = 4.5;
      _hero.isMoving  = 1; 
      _hero.curImg    = _hero.img;
      _hero.h         = _hero.img->h / _hero.numImages;
      _hero.w         = _hero.img->w;
      _hero.curImgFrm = 
           _hero.frmOrder[_hero.curDir-1][_hero.frmIndex] * _hero.h;
      _hero.yPos      = MM_SCREEN_HEIGHT - _hero.h - 10;
    }
  }
  // if current image is default hero image, it is time to have hero
  // exit stage right
  else if (_hero.curImg == _hero.img)
  {
    // chose the correct frame to display.  The normal update hero position
    // code will adjust the frmIndex value.  We must adjust the curImgFrm 
    // value here because it will not be updated in main because we have
    // overriden user control of the hero
    _hero.curImgFrm = _hero.frmOrder[_hero.curDir-1][_hero.frmIndex] * _hero.h;
    if (_hero.xPos > MM_SCREEN_WIDTH)
    {
      _hero.show     = 0;
      _hero.isMoving = 0;
      // when delay has expired, set game state to show the credits
      if ( _hero.cDelCur++ > _hero.cDel)
      {
        MM_SetGameState(MM_STATE_LEVEL_COMPLETE);
      }
    }
  }
}

//------------------------------------------------------------------------------
// Name:     CollisionOverride
// Summary:  Determines what collision override function shold be used to 
//           update hero's proition when a collision has occurred.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void CollisionOverride()
{
  // Do one of the following iff a collision is allready in progress
  if      (_hero.collisionInProgress == HERO_HURT)
  {
    UpdateHeroHurt();
  }
  else if (_hero.collisionInProgress == HERO_CONTINUE)
  {
    UpdateHeroDeathSequence();
  }
  else if (_hero.collisionInProgress == HERO_GAME_OVER)
  {
    UpdateHeroDeathSequence();
  }
  else if (_hero.collisionInProgress == HERO_BEGIN_REGENERATE)
  {
    UpdateHeroBeginRegenerate();
  }
  else if (_hero.collisionInProgress == HERO_REGENERATE)
  {
    UpdateHeroRegenerate();
  }
  else if (_hero.collisionInProgress == HERO_FINISH_LEVEL || 
           _hero.collisionInProgress == HERO_BEGIN_FINISH_LEVEL)
  {
    UpdateHeroFinishLevel();
  }
  // Collision just occured, so set up needed variables and choose the type 
  // of collision to carry out
  else if (_isVulnerable && _hero.collision < 0)
  {
    int extraLives;
    int power;
    unsigned char colDir;
    if (_hero.isDucking)
      HM_StopDuck();
    RM_PlaySound(RM_SFX_HERO_HIT);
    //Mix_PlayChannel(-1, _hero.hit, 0);
    PM_AdjustPower(_hero.collision, &power, &extraLives);

    _hero.curImg  = _hero.collisionImg;
    colDir        = _hero.collisionDir;
    _hero.colDir  = _hero.curDir;
    _heroOverride = _hero; // backup current hero values
    // point hero controler pointer to temp hero structure, this allows user 
    // input to be tracked while hero's controls are being overwritten
    _hcp          = &_heroOverride;  
    
    // Make hero do as we wish for the collision seq
    if (colDir != _hero.curDir && colDir != 0)
      _hero.curDir = colDir;
    else if (colDir == 0)
      _hero.curDir = (_hero.curDir==MM_EAST)?MM_WEST:MM_EAST;
    
    _hero.curImgFrm = _hero.h * ((_hero.colDir==MM_EAST)?0:1);
    // hide hero's weapon while hero is hurt 
    // (hurt image conatins the bat allready)
    _hero.hasWeapon = 0;  
    
    if (_hero.isJumping == 0)   // set jumping velocity if not allready jumping
      _hero.yVelCur    = _hero.yVel / 1.5;
    
    _hero.isJumping     = 1;      // make him jump
    _hero.isMoving      = 1;      // Make him move
    _hero.xVel          = (_hero.curDir==MM_EAST)?5:-5; // change his velocity
    _hero.isRunning     = 0;      // Make hero walk
    _hero.fDelCur       = 0;      // Set frame to display
    BG_SetCeilingFloorSpeed(_hero.isRunning);
    
    if (power == 0) // Hero is out of power
    {
      if (extraLives == 0) // hero has no extra lives
      { // start procedure used to tell player game is over
        _hero.collisionInProgress = HERO_GAME_OVER;
      }
      else // hero has extra lives remaining
      { // start procedure used to allow player to continue
        _hero.collisionInProgress = HERO_CONTINUE;
      }
    }
    else
    {
      _hero.collisionInProgress = HERO_HURT;
    }
  }
  // else hero is invulnerable!
  
  // If hero received power up, allow it to take effect only if the hero
  // has not just entered one of the 2 death states
  if (_isVulnerable && _hero.collision >= 0 && 
      _hero.collisionInProgress != HERO_GAME_OVER && 
      _hero.collisionInProgress != HERO_CONTINUE)
  {
    int extraLives;
    int power;
    // Play a Sound
    // Adjust Power Meeter
    if (_hero.collision >= 100)
      PM_AdjustLives(1);
    else
      PM_AdjustPower(_hero.collision, &power, &extraLives);
    
    _hero.collision = 0;
  }
  else if (_isVulnerable == 0)
  {
    // reset collision so we don't enter this code again and again
    _hero.collision = 0;
  }
  
}

//------------------------------------------------------------------------------
// Name:     UpdateHeroPosition
// Summary:  Function used to update hero's on screen porition
// Inputs:   None
// Outputs:  None
// Returns:  The number of pixels the hero has moved.  This value is retruned
//           to represent the amount the background should scroll.  
// Cautions: None
//------------------------------------------------------------------------------
float UpdateHeroPosition()
{
  int bgEndReached = 0;
  float moveBg     = 0;
  int newGroundPos;

  // Handle collision sequence if a coillision has occured or is in progress
  if (_hero.collisionInProgress || _hero.collision)
  {
    CollisionOverride();
  }
  
  // Do the following if hero is currently moving
  if (_hero.isMoving)
  {
    // Do calc to see if hero's current image should change
    if (_hero.fDelCur++ >= _hero.fDel)
    { 
      _hero.fDelCur = 0;
      _hero.frmIndex++;
      if (_hero.frmIndex >= _hero.frmCount)
        _hero.frmIndex = 0;
      
      if (_hero.isJumping == 0)
      {
        if (_hero.weaponJitter == 1)
          _hero.weaponJitter = -1;
        else
          _hero.weaponJitter = 1;
      }
      else
      {
         _hero.weaponJitter = 0;
      }
    }
    
    // check to see if it is time to change sprite's x position
    if (_hero.xDelCur++ >= _hero.xDel)
    {
      _hero.xDelCur = 0;
      _hero.xPos += _hero.xVel;
      
      bgEndReached = BG_EndReached(0);
      if (_hero.curDir == MM_EAST )
      {
        // If we are at the end of the background
        if (bgEndReached != MM_EAST && _hero.xPos >= HERO_MIDPOINT_EAST )
        {
          _hero.xPos = HERO_MIDPOINT_EAST;  // keep sprite at midpoint
          moveBg = _hero.xVel;
        }
      }
      else if (_scrollWestAllowed && bgEndReached != MM_WEST && 
               _hero.xPos <= HERO_MIDPOINT_WEST )
      {
        _hero.xPos = HERO_MIDPOINT_WEST;  // keep sprite at midpoint
        moveBg = _hero.xVel;
      }
    
      // If sprite reaches end of level, and has reached the specified 
      // X cordinate, set mode to end the current level
      if (bgEndReached == MM_EAST && 
          _hero.xPos > ((MM_SCREEN_WIDTH/2) - _hero.w + 20)) 
      {
        if ( _hero.collisionInProgress != HERO_FINISH_LEVEL)
          _hero.collisionInProgress = HERO_BEGIN_FINISH_LEVEL;
        //_hero.xPos = MM_SCREEN_WIDTH - _hero.w;
      }
      // Prevent hero from exiting stage left
      else if (_hero.xPos < 0) // to far west
      {
        _hero.xPos    = 0;
      }
    }
  }
  // If hero is not moving, count how many frames he has been still and
  // activate a random sprite if he has been still too long
  else if (_idleHeroTime++ >= _idleHeroTimeLimit)
  {
    // set random timeout limit
    _idleHeroTimeLimit = MM_RandomNumberGen(200, 600); 
    _idleHeroTime      = 0;  // reset timer
    SM_CreateRandomSprite(); // activate a random sprite to wake hero up
  }
  
  

  // Adjust xPos, moveBg & groundLevel here
  newGroundPos = SM_AdjustHeroPosition(_hero.yPos, _hero.w, 
                                       &_hero.boundRec, &_hero.xPos, &moveBg);
  if (newGroundPos == 0 && _hero.groundLevelCur != _hero.groundLevel)
  {
    if (_hero.isJumping == 0)
    {
      HM_Jump();
      _hero.yVelCur = 0;
    }
    _hero.groundLevelCur = _hero.groundLevel;
  }
  else if (newGroundPos != 0)
  {
    _hero.groundLevelCur = newGroundPos;
  }    
  
  // Code to make sprite jump
  if (_hero.isJumping && _hero.yDelCur++ >= _hero.yDel)
  {
    _hero.yDelCur = 0;
    
    if ( _hero.yPos + _hero.yVelCur < _hero.groundLevelCur )
    {
      _hero.yPos    += _hero.yVelCur;
      _hero.yVelCur += _hero.gravity;
    }
    else
    {
      _hero.isJumping = 0;
      _hero.yPos      = _hero.groundLevelCur;
      _hero.curImg    = _hero.img;
      _hero.weaponYOffset     = -5;
      if (_hero.isDucking == 2)
      {
        _hero.isDucking = 0;
        HM_Duck();
      }
    }
  }
  
  // Code to annimate weapon sequence, and set flag to denote when 
  // weapon can and cannot cause Van Damage
  if (_hero.hasWeapon) 
  {
    int direction, frmOffset, curWepFrm;
    if ( _hero.curDir == MM_EAST )
    {
      _hero.wDstRec.x = _hero.xPos + _hero.weaponXOffsetEast;
    }
    else
    {
      _hero.wDstRec.x = _hero.xPos + _hero.weaponXOffsetWest;
    }
    _hero.wDstRec.y = _hero.yPos + _hero.weaponJitter + _hero.weaponYOffset;
    direction       = _hero.curDir-1;  // convert to 0 for east, 1 forwest
    // 0 is base for facing east frames, 7 is base for facing west frames
    frmOffset       = direction * (_hero.weaponFrmCnt/2);  
    curWepFrm       = frmOffset + _hero.weaponFrm;
    _hero.wSrcRec.y = _hero.wSrcRec.h * curWepFrm;
    
    if (_hero.weaponInUse)
    {
      if (_hero.weaponDelCur++ >= _hero.weaponFrmDel)
      {
        _hero.weaponFrm += _hero.weaponDir;
        _hero.weaponDelCur = 0;
      }
      if (_hero.weaponFrm >= (_hero.weaponFrmCnt/2))
      //if (_hero.weaponFrm >= _hero.weaponFrmCnt)
      {
        _hero.weaponDir = -1;
        _hero.weaponFrm += _hero.weaponDir;
      }

      if (_hero.weaponFrm > 2)
        _hero.weaponInUse = 2;
      else
        _hero.weaponInUse = 1;
        
      if (_hero.weaponFrm == 0 && _hero.weaponDir == -1)
      {
        _hero.weaponDir   = 1;
        _hero.weaponInUse = 0;
      }
      
    }
  }
  
  // if User has control of hero, use jump image or walk/run image
  if (_hcp == &_hero)
  {
    if (_hero.isJumping)
    {
      _hero.curImgFrm = (_hero.curDir-1) * _hero.h;
    }
    else
    {
      // if not jumping, simply calculate frame offset using normal 
      // walk/run sequence
      _hero.curImgFrm = _hero.frmOrder[_hero.curDir-1][_hero.frmIndex] * _hero.h;
    }
  }
  
  return(moveBg);
}

//------------------------------------------------------------------------------
// Name:     DrawHero
// Summary:  Function used to draw hero to the screen
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int DrawHero(void *hv)
{
  int status = 0;
  
  //Create an initialize static SDL rectanges for the screen and sprite
  static SDL_Rect sprRec = { 0, 0, 0, 0 };
  static SDL_Rect scrRec = { 0, 0, 0, 0 };
  scrRec.x = _hero.xPos;
  scrRec.y = _hero.yPos;
  sprRec.w = _hero.w;
  sprRec.h = _hero.h;
  
  if ( _hero.show )
  { 
    sprRec.y = _hero.curImgFrm;
    status   = SDL_BlitSurface(_hero.curImg, &sprRec, _scr, &scrRec);      
  
    if (_hero.hasWeapon)
      status = SDL_BlitSurface(_hero.weaponImg, &_hero.wSrcRec, 
                               _scr,            &_hero.wDstRec);
  }
 
  return(status);
}

// Interface function used to allow Sprite Manager class to know 
// where the hero is on screen.
float  HM_GetXPos() { return(_hero.xPos); }

// setting this flag to 2 infomat HM_InitLevel function that this value 
// should be reset to 0 for game play to make hero invulnerable.
// By using this method, we do not have to explicity activate / de-activate
// the hero's vulnerability after each game.  It will be set to the correct
// value automatically in the HM_InitLevel function each time a game is 
// started. 
void HM_HeroInvulnerable() { _isVulnerable = 2; }


// Function called from main that sets flag saying for a single frame
// make hero draw to screen even if he shoudl be blinking.  This is used to
// ensures hero is allways drawn to screen when a screenshot takes place.
void HM_ShowHero () { _hero.show = 1; }
