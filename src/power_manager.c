//-----------------------------------------------------------------------------
//  Class:
//  Power Manager
//
//  Description:
//  This class manages the Health Meeter and Extra Life Meeter used by our 
//  hero
//-----------------------------------------------------------------------------


#include "power_manager.h"
#include "resource_manager.h"
#include "dl_manager.h"
#include "bg_manager.h"
#include "map_manager.h"


// In the realm of the mega-mart, we define infinity as 10
#define INFINITY        10 
#define MAX_LIVES        9
#define BLINK_DURATION 100

static SDL_Surface *_scr;
static SDL_Surface *_imgExtraLives;
static SDL_Surface *_imgPowerMeter;
static SDL_Surface *_imgNum[11];  // Holds images of 0-9 & infinity image

static SDL_Rect    _dstRecEL;
static SDL_Rect    _dstRecELT;
static SDL_Rect    _srcRecPwr;
static SDL_Rect    _dstRecPwr;
static short       _maxPower;
static short       _curPower;
static short       _numLives;
static int         _prevNextFrm;
static int         _infinityFlag;
static int         _imagesLoaded = 0;
static int         _init         = 0;
static DL_LinkedListNode _drawStruct;
static int DrawMeeterDummy();
static unsigned short _blinker;

//------------------------------------------------------------------------------
// Name:     PM_Init
// Summary:  Called 1X, initialses Power Manager for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void PM_Init()
{
  _scr = MM_GetScreenPtr();
  if (_init == 0)
  {
    _init         = 1;
    _imagesLoaded = 0;
    _infinityFlag = 0;
  }
}

//------------------------------------------------------------------------------
// Name:     PM_Init
// Summary:  Used to initilize Power Manager for use in the given level
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int PM_InitLevel(unsigned int level)
{
  _blinker       = BLINK_DURATION;  // ensures meeter does not blink
  _numLives      = 3;
  _maxPower      = 5;
  _curPower      = _maxPower;
  // number of lives remaining image
  _imgExtraLives = RM_GetImage(RM_IMG_HERO_EXTRA_LIFE);   
  // Power Meeter image
  _imgPowerMeter = RM_GetImage(RM_IMG_HERO_POWER_METER);  
  
  if (_infinityFlag)
  {
    _infinityFlag = 0;
    _numLives     = INFINITY;
  }
  
  // only load data if it has not yet been loaded.  Once loaded, this data can
  // pretty much stay in memory for the lifetime of the program
  if (_imagesLoaded == 0)
  {
    ZIP_Font *f1;
    int x;
    char buf[5];
    SDL_Color fgColor = {255,0,0};

    // Load font used to generate text string representing number of 
    // spare lives
    f1 = ZIP_LoadFont(ZIP_FONT1, 12);
    for (x=0; x < 10; x++)  // reate image strings 0 - 9 for extra lives
    {
      sprintf(buf, "x %i", x);
      _imgNum[x] = TTF_RenderText_Blended(f1->f, buf, fgColor);
    }
    ZIP_CloseFont(f1);

    // Load special image used to represent infinity lives
    _imgNum[INFINITY] = ZIP_LoadImage("infinity.png");
    
    // Set location for where to display above images
    _srcRecPwr.x  = 0;
    _srcRecPwr.y  = 0;
    _srcRecPwr.w  = _imgPowerMeter->w;
    _srcRecPwr.h  = _imgPowerMeter->h / 6;
    _dstRecPwr.x  = MM_SCREEN_WIDTH - _imgPowerMeter->w - 5;
    _dstRecPwr.y  = 5;
    _dstRecEL.x   = 5;
    _dstRecEL.y   = 5;
    _dstRecELT.x  = _dstRecEL.x + _imgExtraLives->w - 10;
    _dstRecELT.y  = _dstRecEL.y + _imgExtraLives->h - _imgNum[0]->h; 
    _imagesLoaded = 1; // denotes images loaded
  }
  _drawStruct.key       = 11.1;
  _drawStruct.DrawImage = PM_DrawMeeter;
  DL_Add((void*) &_drawStruct);
  return(0);
}

//------------------------------------------------------------------------------
// Name:     DrawMeeterDummy
// Summary:  Draw function used to ensure Power meeter does not draw anything
//           to screen when the level complete text gets activated in 
//           Hero Manager Class
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int DrawMeeterDummy()
{
  return(0); 
}

//------------------------------------------------------------------------------
// Name:     PM_DisableDrawing
// Summary:  Public function accessed by Hero Manager class to make the Power 
//           Meeter class stop drawing power meeter related data to the screen
//           Hero Manager Class
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void PM_DisableDrawing()
{
  _drawStruct.DrawImage = DrawMeeterDummy;
}

//------------------------------------------------------------------------------
// Name:     PM_DisableDrawing
// Summary:  Public function accessed by Hero Manager class to make the Power 
//           Meeter class stop drawing power meeter related data to the screen
//           Hero Manager Class
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
int PM_DrawMeeter()
{
  int status = 0;

  // if blinker is less than 300, make meeter blink
  if (_blinker < BLINK_DURATION)  
  {
    _blinker++; 
    if (_blinker % 5 == 0)  // blink meeter every half second or so
    {
      // rotate between current power frame, and previous frame
      if ( _srcRecPwr.y == (_curPower * _srcRecPwr.h))
        // previous/next frame
        _srcRecPwr.y = (_curPower+_prevNextFrm) * _srcRecPwr.h;  
      else
        _srcRecPwr.y = _curPower * _srcRecPwr.h;      // current frame
    }
  }
  else  // else no blinking, just draw current frame
  {
    _srcRecPwr.y = _curPower * _srcRecPwr.h;
  }
  
  status += SDL_BlitSurface(_imgExtraLives, 0, _scr, &_dstRecEL);
  status += SDL_BlitSurface(_imgNum[_numLives], 0, _scr, &_dstRecELT);
  status += SDL_BlitSurface(_imgPowerMeter, &_srcRecPwr, _scr, &_dstRecPwr);
    
  return(status);
}

//------------------------------------------------------------------------------
// Name:     PM_AdjustPower
// Summary:  Public function used to allow Hero Manager class to adjust the
//           her's current power level
// Inputs:   value - amount to increase/decrease hero's power by
// Outputs:  1. retPower - current Power level of hero
//           2. retLives - Number of spare lives left after adjusting power  
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void PM_AdjustPower(short value, int *retPower, int *retLives)
{
  int allreadyAtFullPower = (_curPower==_maxPower)?1:0;
  
  _curPower += value;
  if      (_curPower > _maxPower)  // proper bounds checking
  {
    _curPower = _maxPower;
  }
  else if (_curPower < 0)
  {
    _curPower = 0;
  }

  if (_curPower == 0) // If power is 0, remove an extra life.
    PM_AdjustLives(-1);
   
  // make power meeter blink if power was lost
  // NOTE: Frame order for power meeter goes as follow: 
  // Top frame = NO POWER, Bottom frame = FULL POWER
  if (value < 0)   
  {
    _blinker     = 0;
    _prevNextFrm = 1; // toggle between current frame and next frame
  }
  // make power meeter blink if power was gained, and hero was NOT allready
  // at full power before getting power up
  else if (value > 0 && allreadyAtFullPower == 0)
  {
    _blinker     = 0;
    _prevNextFrm = -1; // toggle between current frame and previous frame
  }
   
  *retPower = _curPower;
  *retLives = _numLives;
}

//------------------------------------------------------------------------------
// Name:     PM_AdjustLives
// Summary:  Public function used to allow Hero Manager class to increase
//           or decrease the number of spare lives held by the hero
// Inputs:   count - number of spare lifes to add or subtract from total
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void PM_AdjustLives(int count) 
{ 
  if (_numLives != INFINITY)
  {
    _numLives += count;
    if (_numLives < 0) // allow a minimum of 0 lives
      _numLives = 0;
    else if (_numLives > MAX_LIVES)  // a cat can only have 9 lives
    _numLives = MAX_LIVES;
  }
  
  // Possibly add a bonus life if hero is low on lives
  if (_numLives == 1)
  {
    int rand = (_numLives == 1)?1:3;
    if ( MM_RandomNumberGen(0,rand) == 1 )
    {
      int xPos = BG_GetxPosGlobal();
      int yPos = MM_RandomNumberGen(80, 185);
      xPos     = MM_RandomNumberGen(xPos+550, xPos+1000);
      MAP_AddExtraLifeObject(xPos, yPos, 9);
    }
  }
}

//------------------------------------------------------------------------------
// Name:     PM_ResetHealth
// Summary:  Public function used to allow Hero Manager class to reset the power
//           class when the hero dies and a new life is started
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void PM_ResetHealth() 
{ 
  _curPower = _maxPower; 
  _blinker  = BLINK_DURATION;
}

// Special function used to allow Menu Manager Class to activate the mode
// that gives our hero Infinite spare lives
void PM_ActivateInfinityLives() { _infinityFlag = 1; }

