//-----------------------------------------------------------------------------
//  Class:
//  Background Manager
//
//  Description:
//  This class draws and updates the paralax scrolling background.  It also
//  runs in a thread as to allow the GU to draw the background in parallel 
//  with main as it updates sprite positions and such
//-----------------------------------------------------------------------------

#include "bg_manager.h"
#include "map_manager.h"
#include "resource_manager.h"
#include "sce_graphics.h"


#define LOOP_RATE      2.0

// Size of individual layers in each image
#define LOOP_HEIGHT           151
#define FLOOR_HEIGHT           47
#define CEILING_HEIGHT         74

// Contained in BG1
#define WALK_CEILING_YOFFSET    0
#define LOOP_YOFFSET          740      

// Contained in BG2
#define RUN_CEILING_YOFFSET     0 
#define WALK_FLOOR_YOFFSET    518

// Contained in RF1-7
#define RUN_FLOOR_YOFFSET       0


static SDL_Surface *_scr;
static SDL_Surface *_bgImg1;
static SDL_Surface *_bgImg2;
static SDL_Surface *_rFloorImg[7];
static SDL_Surface *_loopImg;
static SDL_Surface *_floorImg;
static SDL_Surface *_ceilingImg;

static SDL_Rect    _loop1SrcRec,   _loop1DstRec;
static SDL_Rect    _loop2SrcRec,   _loop2DstRec;
static SDL_Rect    _floorSrcRec,   _floorDstRec;
static SDL_Rect    _ceilingSrcRec, _ceilingDstRec;

static int _rLayerFrmCount;
static int _wLayerFrmCount;
static int _layerFrmCount;
static int _layerFrm;
static int _levelSize;
static int _floorYOffset;
static int _ceilingYOffset;
static int _updateRate      = 0;
static float _xPosLoop      = 0.0; 
static float _xPosGlobal    = 0.0; 


// Private Function
static int BG_SetCeilingFloorMode();
static int InitLevel1();

//------------------------------------------------------------------------------
// Name:     BG_Init
// Summary:  Called 1X to initilize BG manager for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void BG_Init()
{
  _scr = MM_GetScreenPtr();
}

//------------------------------------------------------------------------------
// Name:     BG_Init
// Summary:  Initilizes the specified level
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int BG_InitLevel(unsigned int level)
{
  int status = 0;
  
  // Load and optimze background images
  // _bgImg1 contains ceiling_walk and loop image
  // _bgImg2 contains walk_floor, run_ceiling, and run_floor image
  _bgImg1          = RM_GetImage(RM_IMG_BG1);
  _bgImg2          = RM_GetImage(RM_IMG_BG2);
  _loopImg         = _bgImg1;
  _rFloorImg[0]    = RM_GetImage(RM_IMG_RF1);
  _rFloorImg[1]    = RM_GetImage(RM_IMG_RF2);
  _rFloorImg[2]    = RM_GetImage(RM_IMG_RF3);
  _rFloorImg[3]    = RM_GetImage(RM_IMG_RF4);
  _rFloorImg[4]    = RM_GetImage(RM_IMG_RF5);
  _rFloorImg[5]    = RM_GetImage(RM_IMG_RF6);
  _rFloorImg[6]    = RM_GetImage(RM_IMG_RF7);
  
  // initialize with walk values
  _floorImg        = _bgImg2;
  _ceilingImg      = _bgImg1;
  _floorYOffset    = WALK_FLOOR_YOFFSET;
  _ceilingYOffset  = WALK_CEILING_YOFFSET;
                   
  _levelSize       = MAP_GetLevelSize(level);
  _updateRate      = 0;
  _xPosLoop        = 0.0; 
  _xPosGlobal      = 0.0; 
                   
  _rLayerFrmCount  = 7;
  _wLayerFrmCount  = 10;
  _layerFrmCount   = _wLayerFrmCount;
  _layerFrm        = 0;
  
  _floorSrcRec.x   = 0;
  _floorSrcRec.y   = _floorYOffset;
  _floorSrcRec.w   = MM_SCREEN_WIDTH;
  _floorSrcRec.h   = FLOOR_HEIGHT;
  _floorDstRec.x   = 0;
  _floorDstRec.y   = CEILING_HEIGHT + LOOP_HEIGHT;
  
  _ceilingSrcRec.x = 0;
  _ceilingSrcRec.y = _ceilingYOffset;
  _ceilingSrcRec.w = MM_SCREEN_WIDTH;                  
  _ceilingSrcRec.h = CEILING_HEIGHT;
  _ceilingDstRec.x = 0;
  _ceilingDstRec.y = 0;
  
  _loop1SrcRec.x   = 0;
  _loop1SrcRec.y   = LOOP_YOFFSET;
  _loop1SrcRec.w   = MM_SCREEN_WIDTH;
  _loop1SrcRec.h   = LOOP_HEIGHT;
                   
  _loop1DstRec.x   = 0;
  _loop1DstRec.y   = CEILING_HEIGHT;
  _loop1DstRec.w   = 0;
  _loop1DstRec.h   = 0;
                   
  _loop2SrcRec.x   = 0;
  _loop2SrcRec.y   = LOOP_YOFFSET;
  _loop2SrcRec.w   = 0;
  _loop2SrcRec.h   = LOOP_HEIGHT;
                   
  _loop2DstRec.x   = 0;
  _loop2DstRec.y   = CEILING_HEIGHT;
  _loop2DstRec.w   = 0;
  _loop2DstRec.h   = 0;  

  return(status);
}

//------------------------------------------------------------------------------
// Name:     BG_UpdatePosition
// Summary:  Updates the position of the scrolling background image and the
//           parallax flor and ceiling images
// Inputs:   dir - amount to scroll background by
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int BG_UpdatePosition(float dir)
{
  int endReached = 0;
  if ((int)dir != 0)
  {
    endReached = BG_EndReached(dir);
    if ( endReached == MM_EAST)
    {
      _xPosGlobal = _levelSize - MM_SCREEN_WIDTH;
    } 
    else if (endReached == MM_WEST)
    {
      _xPosGlobal = 0;
    }
    else
    {
      _xPosGlobal += dir;

      // Code used to LOOP an image in the background
      _xPosLoop += dir / LOOP_RATE;
      
      // current x position has gone past end of image
      if ( _xPosLoop >= MM_SCREEN_WIDTH)                   
      {
        // determine how far past end of image xpos is, reset xpos to this value
        _xPosLoop        = _xPosLoop - MM_SCREEN_WIDTH;    
        _loop1SrcRec.x   = _xPosLoop;
        // reset width to full screen again
        _loop1SrcRec.w   = MM_SCREEN_WIDTH;               
        // start drawing image on screen at position 0 again
        _loop1DstRec.x   = 0;                          
        // reset rec 2, it won;t need to draw for a while
        _loop2DstRec.w   = 0;                          
        _loop2SrcRec.w   = 0;
      }
      // current xpos has moved from beginning of image to end of image
      else if ((int)_xPosLoop < 0)
      {
        _xPosLoop       = MM_SCREEN_WIDTH + _xPosLoop;
        _loop1SrcRec.x  = _xPosLoop;
        // Set width accordingly
        _loop1SrcRec.w  = MM_SCREEN_WIDTH - _loop1SrcRec.x;  
        // Set xpos of filler rec, it will begin where main rec ends
        _loop2DstRec.x  = _loop1SrcRec.w;                     
        // Set width of what filler rec will draw to screen 
        // This will be the start of the image, drawn on the right half 
        // of the screen
        _loop2SrcRec.w  = MM_SCREEN_WIDTH - _loop2DstRec.x;     
      }
      // Current xpos plus screen width goes beyond the end of the current image
      else if ( (int)_xPosLoop + MM_SCREEN_WIDTH > MM_SCREEN_WIDTH)
      {
        _loop1SrcRec.x = (int)_xPosLoop;
        // Determine how much of the ending half of the image can be displayed
        _loop1SrcRec.w = MM_SCREEN_WIDTH - _loop1SrcRec.x;   
        // Set xpos of filler rec, used to draw start of image after the end 
        // of the image
        _loop2DstRec.x = _loop1SrcRec.w;                      
        // Set width of what filler rec will draw to screen
        _loop2SrcRec.w = MM_SCREEN_WIDTH - _loop2DstRec.x;      
      }
      // else, rec 1 is in a position to draw entire image.  
      // To be safe, reset the following values
      else  
      {
        _loop1SrcRec.x = (int)_xPosLoop;
        // needed for case where if 
        // (_loop1SrcRec.x + MM_SCREEN_WIDTH > _bg.scrollImg->w) 
        // becomes false by hero moving left
        _loop1SrcRec.w = MM_SCREEN_WIDTH;  
        // this ensures these 2 variables contain the correct values
        _loop2SrcRec.w = 0;             
      }
      
      // Update floor and ceiling frames
      if ( dir > 0 )
        _layerFrm++;
      else
        _layerFrm--;
      
      if ( _layerFrm >= _layerFrmCount)
        _layerFrm = 0;
      else if( _layerFrm < 0 )
        _layerFrm = _layerFrmCount-1;

      if(_updateRate != -1)
      {
        BG_SetCeilingFloorMode(); 
      }
      
      // the run floor sequence is broken into 7 different files 
      // (7 different SDL images)
      if (_layerFrmCount == _rLayerFrmCount)
      {
        // all files begin at y offset of 0
        _floorSrcRec.y = _floorYOffset;  
        // point floor pointer to image file with correct floor image
        _floorImg      = _rFloorImg[_layerFrm]; 
      }
      // walk floor seq in in a single file, so calculate y offset of current
      // frame using the standard method
      else
      {
        _floorSrcRec.y   = (_layerFrm * FLOOR_HEIGHT   ) + _floorYOffset;
      }
      _ceilingSrcRec.y = (_layerFrm * CEILING_HEIGHT ) + _ceilingYOffset;
    }
  }
  return(0);
}

//------------------------------------------------------------------------------
// Name:     BG_SetCeilingFloorSpeed
// Summary:  Sets flag that will make update position function switch the 
//           current floor and celiling images between the Walking/Running sets
// Inputs:   isRunning - 0 measn use walking set, 1 means use running set
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int BG_SetCeilingFloorSpeed(int isRunning)
{
  _updateRate = isRunning;
  return(0);
}

//------------------------------------------------------------------------------
// Name:     BG_SetCeilingFloorMode
// Summary:  Called from update position function to switch the current floor 
//           and celiling images between the Walking/Running sets
// Inputs:   None - _updateRate global variable determines walk/run mode
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int BG_SetCeilingFloorMode()
{
  if ( _updateRate )
  {
    if(_layerFrm > 6) 
       _layerFrm = 6;

    _floorImg       = _rFloorImg[_layerFrm];
    _ceilingImg     = _bgImg2;
    _floorYOffset   = RUN_FLOOR_YOFFSET;
    _ceilingYOffset = RUN_CEILING_YOFFSET;
    _layerFrmCount  = _rLayerFrmCount;
  }
  else
  {
    _floorImg       = _bgImg2;
    _ceilingImg     = _bgImg1;
    _floorYOffset   = WALK_FLOOR_YOFFSET;
    _ceilingYOffset = WALK_CEILING_YOFFSET;
    _layerFrmCount  = _wLayerFrmCount;
  }
  
  _updateRate = -1;
  return(0);
}

//------------------------------------------------------------------------------
// Name:     BG_EndReached
// Summary:  Function used to determine if the end of the screen has been 
//           reached after the hero moves the specified number of pixels.  
//           When moving WEST, the end is reached once the hero moves beyond 
//           the midpoint.  When moving EAST, the end is reached when the hero
//           reaches the end of the level.
// Inputs:   dir - the number of pixels hero will move (positive value for 
//           moving EAST, negative value for moving WEST)
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int BG_EndReached(float dir)
{
  int endReached = 0;
  
  if (_xPosGlobal + dir + MM_SCREEN_WIDTH >= _levelSize )
  {
    endReached  = MM_EAST;
  } 
  else if (_xPosGlobal + dir <= 0)
  {
    endReached  = MM_WEST;
  }
  
  return(endReached);
}

//------------------------------------------------------------------------------
// Name:     BG_DrawBackground
// Summary:  Draws the background to the screen
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: This function uses SCE functions to draw the background images so
//           proper clipping is vital.
//------------------------------------------------------------------------------
void BG_DrawBackground()
{
  // These functions can be used to draw the BG if this game is ported.  For 
  // Now we draw the background using SCE function to increase performance
  //SCE_BlitSurface(_floorImg,   &_floorSrcRec,   _scr, &_floorDstRec);
  //SCE_BlitSurface(_ceilingImg, &_ceilingSrcRec, _scr, &_ceilingDstRec);
  //SCE_BlitSurface(_loopImg, &_loop1SrcRec, _scr, &_loop1DstRec);
  //SCE_BlitSurface(_loopImg, &_loop2SrcRec, _scr, &_loop2DstRec);
  
  sceGuStart(GU_DIRECT, _SCEBgList);
  sceGuCopyImage(GU_PSM_5551, _floorSrcRec.x, _floorSrcRec.y, 
                 _floorSrcRec.w, _floorSrcRec.h, 
                 _floorImg->w,  _floorImg->pixels, 
                 _floorDstRec.x, _floorDstRec.y, 512, _scr->pixels);
  
  sceGuCopyImage(GU_PSM_5551, _ceilingSrcRec.x, _ceilingSrcRec.y, 
                 _ceilingSrcRec.w, _ceilingSrcRec.h, 
                 _ceilingImg->w,  _ceilingImg->pixels, 
                 _ceilingDstRec.x, _ceilingDstRec.y, 512, _scr->pixels);
                              
  if (_loop1SrcRec.x >=0 && _loop1SrcRec.x <  MM_SCREEN_WIDTH &&
      _loop1SrcRec.w > 0 && _loop1SrcRec.w <= MM_SCREEN_WIDTH &&
      _loop1DstRec.x >=0 && _loop1DstRec.x <  MM_SCREEN_WIDTH)
    sceGuCopyImage(GU_PSM_5551, _loop1SrcRec.x, _loop1SrcRec.y, 
                  _loop1SrcRec.w, _loop1SrcRec.h, 
                  _loopImg->w,   _loopImg->pixels, 
                  _loop1DstRec.x, _loop1DstRec.y, 512, _scr->pixels);
                 
  if (_loop2SrcRec.x >=0 && _loop2SrcRec.x <  MM_SCREEN_WIDTH &&
      _loop2SrcRec.w > 0 && _loop2SrcRec.w <= MM_SCREEN_WIDTH &&
      _loop2DstRec.x >=0 && _loop2DstRec.x <  MM_SCREEN_WIDTH)
    sceGuCopyImage(GU_PSM_5551, _loop2SrcRec.x, _loop2SrcRec.y, 
                   _loop2SrcRec.w, _loop2SrcRec.h, 
                   _loopImg->w,   _loopImg->pixels, 
                   _loop2DstRec.x, _loop2DstRec.y, 512, _scr->pixels);
  sceGuFinish();
  sceGuSync(0,0);
}

//------------------------------------------------------------------------------
// Name:     BG_DrawBackground
// Summary:  Wrapper function to allow other classes access to the 
//           global x position value
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: Used for debugging, kept for sentimental reasons
//------------------------------------------------------------------------------
float BG_GetxPosGlobal()
{
  return(_xPosGlobal);
}


//------------------------------------------------------------------------------
// Name:     BG_DrawBackground
// Summary:  Debug function used by MAP_DebugReset to reset the background
//           to the start of the level
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: Used for debugging, kept for sentimental reasons
//------------------------------------------------------------------------------
void BG_SetXPosGlobal(float x)
{
  _xPosGlobal = x;
}
