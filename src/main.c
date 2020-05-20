//-----------------------------------------------------------------------------
//                              MEGA-MART or:
//   How I Learned to Stop Living in Poverty and Love the corporation
//
//   Copyright 2006 LBG Productions
//
//   Version:
//   2.57 (This marks the 257 revision of the program source code)
//
//   Description:
//   This program is written for the Sony PSP using the freely available PSP
//   SDK.  For licensing information regarding this source code please see
//   the file mega_license.tx
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Module:
//  Main
//
//  Description:
//  This file initializes the resources needed by the game and enters the main
//  loop used to control program flow.
//-----------------------------------------------------------------------------

// PSP Specific Libs
#include <pspdebug.h>
#include <pspkernel.h>
#include <pspdisplay.h>

// Standard Libs
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "png.h"

// SDL Libs
#include "SDL/SDL_main.h"
#include "SDL/SDL_thread.h"
#include "SDL/SDL_mutex.h"
#include "SDL/SDL_framerate.h"
#include "SDL/SDL_mixer.h"

// Proprietary Libs
#include "common.h"
#include "hero_manager.h"
#include "sprite_manager.h"
#include "map_manager.h"
#include "power_manager.h"
#include "bg_manager.h"
#include "resource_manager.h"
#include "dl_manager.h"
#include "menu_manager.h"
#include "cc_manager.h"
#include "sce_graphics.h"

// Structure used by PNG library to copy entire PNG image to memory
// rather than to a file.
typedef struct ScreenShotInfo
{
  unsigned char buf[512 * 272 * 3];
  unsigned int totalBytes;
  void *bufPtr;
} ScreenShotInfo;

// Private Functions (only main should call them)
static void InitializeLevel(unsigned int gameLevel, SDL_Event *event);
static void RunLevelOne(SDL_Event *event);
static void VblankHandlerThread();
static void SaveImage(const char* fileName, unsigned short* data,
                      int width, int height, int lineSize, int saveAlpha);
static void TakeScreenShot();
static void ScreenShotThread();
static void MyFlush(png_structp ctx);
static void MyWrite(png_structp ctx, png_bytep area, png_size_t size);

// Global Data
static int            _flip;
static unsigned int   _gameState;
static unsigned short _scrShotBuf[512 * MM_SCREEN_HEIGHT];
static unsigned short _scrShotInProgress;
static SDL_Surface    *_scr;
static SDL_sem        *_sem;
static SDL_sem        *_scSem;
static SDL_Joystick   *_joystick;
static void           *_scrBuf1;
static void           *_scrBuf2;
static int             _shotCount;

//------------------------------------------------------------------------------
// Name:     main
// Summary:  Where it all happens
// Inputs:   1.  Number or arguments
//           2.  Char Pointer to each arg
// Outputs:  None
// Returns:  Program exit status
// Cautions: None
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  SDL_Event event;

  int status             = 0;
  unsigned int gameLevel = MM_LEVEL1;
  Mix_Chunk *ding        = NULL;

  // Setup Audo used by game
  int audio_rate         = 22050;
  Uint16 audio_format    = AUDIO_U8;
  int audio_channels     = 1;
  int audio_buffers      = 4096;

  // Setup global variables
  _gameState             = MM_STATE_MAIN_MENU;
  _scrShotInProgress     = 0;
  _flip                  = 0;
  _sem                   = SDL_CreateSemaphore(0);
  _scSem                 = SDL_CreateSemaphore(0);
  _joystick              = 0;
  _shotCount             = 1;

  // create threads used to swap screenbuffers and take screenshots
  SDL_CreateThread(VblankHandlerThread, 0);
  SDL_CreateThread(ScreenShotThread, 0);

  // Initialize SDL.
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO |
               SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0)
    EH_Error(EH_SEVERE, "SDL_Init Failed.");

  // Open Joystick
  _joystick = SDL_JoystickOpen(0);
  if (!(_joystick))
    EH_Error(EH_SEVERE, "SDL_JoystickOpen Failed.");

  // Setup Display Area
  _scr = SDL_SetVideoMode(MM_SCREEN_WIDTH, MM_SCREEN_HEIGHT,
                          15, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWACCEL);

  // Verify screen was setup correctly
  if (!(_scr))
    EH_Error(EH_SEVERE,
             "SDL_SetVideoMode failed.\nReturn Error=\n[%s]\n",
             SDL_GetError());

  // Open Audio Device
  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
     EH_Error(EH_SEVERE,
              "Mix_OpenAudio failed.\nReturn Error=\n[%s]\n",
               SDL_GetError());

  // Initialize True type Font support
  if(TTF_Init()==-1)
     EH_Error(EH_SEVERE,
              "Mix_OpenAudio failed.\nReturn Error=\n[%s]\n",
              TTF_GetError());

  // Get pointers to screen buffer and back buffer used by SDL
  _scrBuf1 = _scr->pixels;
  SDL_FillRect(_scr, 0, 0);
  SDL_Flip(_scr);
  _scrBuf2 = _scr->pixels;
  SDL_FillRect(_scr, 0, 0);
  SDL_Flip(_scr);

  // Ding wave is played by MENU_DrawMain function.  The sound cannot be
  // freed within this function because it will be playing even after the
  // function exits (the user selects "start game" and this sound is played
  // as the function exits).  to combat this issue, we simply load it 1 time
  // in main and let it stay in memory throughout the entire game.
  ZIP_OpenZipFile(ZIP_MAIN);
  ding = ZIP_LoadMusic("ding.wav");
  ZIP_CloseZipFile();

  // Initialze Mega-Mart "Classes"
  srand(time(NULL));
  EH_Init();
  CC_Init();
  SCE_Init();
  BG_Init();
  DL_Init();
  SM_Init();
  PM_Init();
  HM_Init();
  RM_Init();
  MUNU_Init(argv[0]); // argv[0] should be path and name of this program

  // Main Controll Loop (where all the majick takes place)
  while (_gameState != MM_STATE_EXIT)
  {
    if (_gameState == MM_STATE_MAIN_MENU)
    {
      gameLevel  = MENU_DrawMain(&event, gameLevel, ding);
      _gameState = MM_STATE_INITIALIZE;
    }
    if (_gameState == MM_STATE_INITIALIZE)
    {
      // functions within here will set global _gameState variable
      InitializeLevel(gameLevel, &event);
    }
    if (_gameState == MM_STATE_GAME_OVER)
    {
      if (gameLevel != MM_LEVEL_FINAL) // skip game over text for final level
        MENU_DrawGameOver(&event);
      _gameState = MM_STATE_MAIN_MENU;
      gameLevel  = MM_LEVEL1;
    }
    if (_gameState == MM_STATE_LEVEL_COMPLETE)
    {
      if (gameLevel == MM_LEVEL1)
      {
        gameLevel  = MM_LEVEL_FINAL;
        _gameState = MM_STATE_INITIALIZE;
      }
      else if (gameLevel == MM_LEVEL_FINAL)
      {
        gameLevel  = MM_LEVEL_CREDITS;
        _gameState = MM_STATE_INITIALIZE;
      }
      else if (gameLevel == MM_LEVEL_CREDITS ||
               gameLevel == MM_LEVEL_HIDDEN1 )
      {
        gameLevel  = MM_LEVEL1;
        _gameState = MM_STATE_MAIN_MENU;
      }
    }
  }

  // Clean up your mess before exiting
  if(_joystick)
    SDL_JoystickClose(_joystick);
  SDL_Quit();
  return(status);
}

//------------------------------------------------------------------------------
// Name:     RunLevelOne
// Summary:  Function used control level one
// Inputs:   event - Pointer to event structure used for input
// Outputs:  None
// Returns:  None
// Cautions: _gameState can be changed by functions called within
//           HM_UpdateHeroPosition.  MENU_DrawPauseGame returns a gamestate
//           value.  This function has 2 exit points.  The main while loop can
//           end, or the user can exit level from the game paused screen.
//------------------------------------------------------------------------------
void RunLevelOne(SDL_Event *event)
{
  float moveBg = 0;
  _flip        = 1;

  // Create Framerate manager
  FPSmanager fpsMan;
  SDL_initFramerate(&fpsMan);
  SDL_setFramerate(&fpsMan, 50);

  while (_gameState == MM_STATE_RUNNING) // Level 1 Main Loop
  {

    while (SDL_PollEvent(event))
    {
      switch (event->type)
      {
        case SDL_JOYBUTTONDOWN:
          if(event->jbutton.button == _CCCtrlMoveRight)
            HM_MoveRight();

          if(event->jbutton.button == _CCCtrlMoveLeft)
            HM_MoveLeft();

          if(event->jbutton.button == _CCCtrlAttack)
            HM_UseWeapon();

          if(event->jbutton.button == _CCCtrlRun)
            HM_StartRunning();

          if(event->jbutton.button == _CCCtrlJump)
            HM_Jump();

          if (event->jbutton.button == _CCCtrlScreenShot)
            TakeScreenShot();

          if(event->jbutton.button == _CCCtrlDuck )
            HM_Duck();

          if (event->jbutton.button == _CCCtrlPause)
          {
            _gameState = MENU_DrawPauseGame(event, _gameState);
            if (_gameState == MM_STATE_MAIN_MENU) // user chose to quit game
            {
              // wait for VBLANK thread to finish
              // see NOTE at end of this function for details.
              SDL_SemWait(_sem);

              // do not pass go, do not draw anything else to screen.
              // it is now safe to return to main and let it draw menus
              return;  // EXIT POINT 2
            }
          }
          break;  // END Joystick Button Down

        case SDL_JOYBUTTONUP:
          if(event->jbutton.button == _CCCtrlMoveRight ||
             event->jbutton.button == _CCCtrlMoveLeft )
             HM_StopMoving();

          if(event->jbutton.button == _CCCtrlRun )
            HM_StopRunning();

          if(event->jbutton.button == _CCCtrlDuck )
          {
            HM_StopDuck();
            // if user stops ducking, and is puching left or right on
            // joystick, make hero move left or right.  If user pressed left
            // or right while hero was ducking, the command would have been
            // ignored, so we try to account for it now.
            if (SDL_JoystickGetButton(_joystick, _CCCtrlMoveRight))
              HM_MoveRight();
            if (SDL_JoystickGetButton(_joystick, _CCCtrlMoveLeft))
              HM_MoveLeft();
          }

          break;  // End Joystick Button Up

        case SDL_QUIT:
          break;

        default:
          break;
      }
    }

    // Update the sprite and or background position
    SM_DetectCollision();
    moveBg = HM_UpdateHeroPosition();
    SM_UpdateSpritePositions(moveBg);
    BG_UpdatePosition(moveBg);
    MAP_EnableObjects(moveBg);

    // vblank thread will signal when buffers have been swapped.
    // Afterwords it will be safe to draw to backbuffer.
    SDL_SemWait(_sem);
    DL_DrawImages();
    //EH_DrawErrors();  // Activate for debugging

    // manage game framerate
    SDL_framerateDelay(&fpsMan);

    // Set flag to let VBLANK thread know a complete frame has been drawn
    // and it is now safe to swap buffers
    _flip = 1;
  }

  // NOTE: When leaving loop, wait on SEM.  This ensures VBLANK thread has
  // drawn the last of its data to the screen.  It will not draw
  // anything else until this function re-initialises flip to 1.
  SDL_SemWait(_sem);
  // EXIT POINT 1
}

//------------------------------------------------------------------------------
// Name:     InitializeLevel
// Summary:  Loads required resources then starts the specified level
// Inputs:   1.  gameLevel - Game level to start
//           2.  event - Pointer to event structure used for input
// Outputs:  None
// Returns:  None
// Cautions: _gameState can be changed by functions called within RunLevelOne
//------------------------------------------------------------------------------
void InitializeLevel(unsigned int gameLevel, SDL_Event *event)
{
  //Initialize and start level 1
  if (gameLevel == MM_LEVEL1)
  {
    // game intro... Hey, you can just press start to skip it!
    MENU_DrawIntro(event);

    // Only draw load screen if resources for level 1 are not allready loaded
    if (RM_GetLastLevelLoaded() != MM_LEVEL1)
      MENU_DrawLoadScreen();

    ZIP_OpenZipFile(ZIP_MAIN);
    RM_InitLevel (gameLevel);
    BG_InitLevel (gameLevel);
    DL_InitLevel (gameLevel);
    PM_InitLevel (gameLevel);
    SM_InitLevel (gameLevel);
    HM_InitLevel (gameLevel);
    MAP_InitLevel(gameLevel);
    ZIP_CloseZipFile();
    _gameState = MM_STATE_RUNNING;
    RM_PlaySoundLoop(RM_SFX_LEVEL1_MUSIC);
    RunLevelOne(event);
    Mix_HaltChannel(-1);  // stop all music after exiting level 1 loop
  }
  // initialize and start the final level
  else if (gameLevel == MM_LEVEL_FINAL)
  {
    ZIP_OpenZipFile(ZIP_MAIN);
    MAP_InitLevel(gameLevel); // free memory if need be from previous level
    RM_InitLevel (gameLevel); // init resources for credits
    ZIP_CloseZipFile();
    _gameState = MENU_DrawFinalLevel(event);
  }
  // initialize and start the hiddenlevel (concept art)
  else if (gameLevel == MM_LEVEL_HIDDEN1)
  {
    _gameState = MENU_DrawHiddenLevel1(event);
  }
  // initialize and start the credits
  else if (gameLevel == MM_LEVEL_CREDITS)
  {
    ZIP_OpenZipFile(ZIP_MAIN);
    MAP_InitLevel(gameLevel); // free memory if need be from previous level
    RM_InitLevel (gameLevel); // init resources for credits
    ZIP_CloseZipFile();
    _gameState = MENU_DrawCredits(event);
  }
  // Unknown level, present error
  else
  {
    EH_Error(EH_SEVERE, "Invalid game level specified: %i.", gameLevel);
  }
}

//------------------------------------------------------------------------------
// Name:     ScreenShotThread
// Summary:  Thread that runs and is awaken when the user takes a screenshot.
//           By running as a thread, the user can continue to play the game
//           while the screenshot is in progress.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void ScreenShotThread()
{
  char fName[15];
  while (1)
  {
    SDL_SemWait(_scSem);  // wait until a screenshot is taken
    sprintf(fName, "shot%i.png", _shotCount++);  // create unique file name
    SaveImage(fName, &_scrShotBuf[0],
              MM_SCREEN_WIDTH, MM_SCREEN_HEIGHT, 512, 0);

     // destroy "in progress" text if it is still on screen
    SM_DestroyScreenShotText();

    // Create screenshot complete text
    SM_CreateScreenshotSprite(RM_SCREENSHOT_2_TXT);

    // Set flag dentoing it's safe to take another screenshot
    _scrShotInProgress = 0;
  }
}

//------------------------------------------------------------------------------
// Name:     VblankHandlerThread
// Summary:  This thread waits for a VBLANK, then swaps display buffer.
//           It then re-draws the background using the GU and signals the
//           main loop when finished via the _sem semaphore.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: Main sets _flip to true to signal that it is time to swap buffers.
//           This thread signals the _sem sempaphore to tell main it has
//           finished flipping buffers and drawing the background.
//------------------------------------------------------------------------------
void VblankHandlerThread()
{
  int sVal = 0;
  while (1)
  {
    sceDisplayWaitVblankStart();
    if (_flip)
    {
      SDL_Flip(_scr);    // Actually put graphics on screen
      _flip = 0;
      sVal = SDL_SemValue(_sem);
      BG_DrawBackground();         // draw background
      // make this a -1/0/1 sem.  A value of 1 means main will not block
      // 0 means main will block. -1 means main is blocked.  Since sem is
      // only waited on in main thread, it will never go below -1.  The
      // following if statment ensures it never goes above 1.
      if (sVal <= 0)
        SDL_SemPost(_sem);
    }
  }
}

//------------------------------------------------------------------------------
// Name:     MM_GetScreenBuffer
// Summary:  Returns the desired screen buffer to the user.
//           It then re-draws the background using the GU and signals the
//           main loop when finished via the _sem semaphore.
// Inputs:   MM_DRAW_BUFFER for on screen buffer, MM_BACK_BUFFER for the back
//           buffer
// Outputs:  None
// Returns:  Pointer to the specified buffer
// Cautions: _scrBuf1 & _scrBuf2 must be initialized before using this function
//------------------------------------------------------------------------------
void *MM_GetScreenBuffer(unsigned int bufType)
{
  void *buf;
  while (_flip == 1) { SDL_Delay(10); }
  if (bufType == MM_DRAW_BUFFER)  // return buffer holding screen data
    buf = (_scr->pixels == _scrBuf1)?_scrBuf2:_scrBuf1;
  else  // return back buffer
    buf = (_scr->pixels == _scrBuf1)?_scrBuf1:_scrBuf2;
  return(buf);
}

//------------------------------------------------------------------------------
// Name:     MM_RandomNumberGen
// Summary:  creates a random number in the specified range
// Inputs:   1.  Lower limit for number (inclusive)
//           2.  Upper limit for number (inclusive)
// Outputs:  None
// Returns:  Random number in the specified range.  If zero is in the range,
//           a value of 0 may be returend.
// Cautions: None
//------------------------------------------------------------------------------
int MM_RandomNumberGen(int lower, int upper)
{
  int rNum, range;
  int nMin = lower;
  int nMax = upper;
  rNum     = rand();

  if (rNum < 0)  // ensure random number is positave
    rNum *= -1;

  if (nMax < 0)  // adjust ranges if a negative max is provided
  {
    // add amount needed to make max positive to both min and max
    nMin += (-1 * nMax);
    nMax = 0;
  }

  if (nMin < 0)  // adjust ranges if a negative min is given
  {
    // add amount to both min/max needed to make min positive
    nMax += (nMin * -1);
    nMin = 0;
  }

  range = nMax - nMin + 1; // get the allowed range of values
  // module number to ensure its in allowed range.
  rNum  = (rNum % range) + nMin;

  // if min was < 0, subtract it from final value to allow negatives
  if (lower < 0)
    rNum += lower;
  return(rNum);
}

//------------------------------------------------------------------------------
// Name:     TakeScreenShot
// Summary:  Sets in motion the events needed to take a screenshot during game
//           play.  This function prevents the "Taking Screenshot" image from
//           being included in the saved screenshot.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: This should only be called when the user is playing level 1
//------------------------------------------------------------------------------
void TakeScreenShot()
{
  if (_scrShotInProgress == 0)
  {
    _scrShotInProgress = 1; // flag to prevent multiple screen shots at once
    void *pixels       = MM_GetScreenBuffer(MM_BACK_BUFFER);

    // remove "Screenshot Finished" sprite text (if present).  The removal of
    // this possible image is why we re-draw the full buffer from within this
    // function
    SM_DestroyScreenShotText();
    SM_ShowBlinkSprites();
    HM_ShowHero();
    SDL_SemWait(_sem);  // wait until background is drawn
    SDL_SemPost(_sem);  // post, so main thread is not effected
    DL_DrawImages();    // draw full frame
    // copy screen buffer to memory
    memcpy(_scrShotBuf, pixels, sizeof(_scrShotBuf));
    // create "Screenshot Started" sprite
    SM_CreateScreenshotSprite(RM_SCREENSHOT_1_TXT);
    SDL_SemPost(_scSem);  // wake up screenshot thread
  }
}

//------------------------------------------------------------------------------
// Name:     MyWrite
// Summary:  Special function that tells the PNG library to write the PNG
//           data to memory instead of to a file handle.  SaveImage uses the
//           PNG library, which in turn uses this function intrenally.
// Inputs:   1.  PNG structure that holds a pointer to the structure we use
//               to store the given image data
//           2.  area - the PNG data we want to store
//           3.  size - the size of the PNG data
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MyWrite(png_structp ctx, png_bytep area, png_size_t size)
{
  // png_get_io_ptr will return pointer to memory structure we allocated in
  // function SaveImage.  We used png_set_write_fn to ensure this data
  // would be available
  ScreenShotInfo *shotData = (ScreenShotInfo*) png_get_io_ptr(ctx);
  memcpy(shotData->bufPtr, area, size);  // copy data to memory
  shotData->totalBytes += size;          // track total data written to memory
  shotData->bufPtr     += size;          // increment buffer pointer position
}

//------------------------------------------------------------------------------
// Name:     SaveImage
// Summary:  Uses PNG library to store the passed in image to a PNG file.
//           The PNG data is created in memory, then the data is copied from
//           memory to the specified file in 1 swoop
// Inputs:   1.  Name of PNG file to create
//           2.  Pointer to image data (should be 16 bit image data)
//           3.  width of image data
//           4.  height of image data
//           5.  linesize
//           6.  0=don't save alpha info, 1=save alpha info
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void SaveImage(const char* fileName, unsigned short* data, int width,
               int height, int lineSize, int saveAlpha)
{
  png_structp    pngPtr;
  png_infop      infoPtr;
  FILE*          fp;
  ScreenShotInfo *shotData;
  int            i;
  int            x;
  int            y;
  u8*            line;

  if ((fp = fopen(fileName, "wb")) == NULL)
    return;

  pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngPtr)
    return;

  infoPtr = png_create_info_struct(pngPtr);
  if (!infoPtr)
  {
    png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
    return;
  }

  // Allocate memory for structure used to hold our screen shot in memory
  // rumer has it byte alligning by 64 speeds up writes
  shotData = (ScreenShotInfo*) memalign(64, sizeof(ScreenShotInfo));
  if (shotData == 0)
  {
    png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
    return;
  }
  shotData->totalBytes = 0;
  shotData->bufPtr     = shotData->buf;

  // Set write pointer to our write function which stores image to memory
  // instead of to a file
  png_set_write_fn(pngPtr, (void*) shotData, MyWrite, MyFlush);

  png_set_IHDR(pngPtr, infoPtr, width, height, 8,
          saveAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
          PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
          PNG_FILTER_TYPE_DEFAULT);
  png_write_info(pngPtr, infoPtr);
  line = (u8*) malloc(width * (saveAlpha ? 4 : 3));
  for (y = 0; y < height; y++)
  {
    for (i = 0, x = 0; x < width; x++)
    {
      unsigned short color = data[x + y * lineSize];
      int r = (color & 0x1f) << 3;
      int g = ((color >> 5) & 0x1f) << 3 ;
      int b = ((color >> 10) & 0x1f) << 3 ;
      line[i++] = r;
      line[i++] = g;
      line[i++] = b;
      if (saveAlpha)
      {
        int a = color & 0x8000 ? 0xff : 0;
        line[i++] = a;
      }
    }
    png_write_row(pngPtr, line);
    // compression of image data is a CPU intensive process, so force
    // this thread to yield processor to main loop every once in a while
    SDL_Delay(1);
  }

  free(line);
  png_write_end(pngPtr, infoPtr);
  png_destroy_write_struct(&pngPtr, (png_infopp)NULL);

  // Write the entire image from memory to file in a single write
  // this speeds up the overall write process immensly
  fwrite(shotData->buf, shotData->totalBytes, 1, fp );
  fclose(fp);      // close file
  free(shotData);  // free allocated memory
}

// Required for PNG lib, not sure what they want this to do
void         MyFlush(png_structp ctx)            { return; }
void         MM_SetGameState(unsigned int state) {_gameState = state; }
SDL_Surface* MM_GetScreenPtr()                   { return(_scr); }

//------------------------------------------------------------------------------
// Name:     MM_TakeMenuScreenshot
// Summary:  Function used to take a screenshot.  This function pauses the game
//           while the screenshot is being taken.  It's purpose is to allow
//           screenshots during in game menues where a pause does not matter.
// Inputs:   None
// Outputs:  None
// Returns:  None - creates file on MS
// Cautions: None
//------------------------------------------------------------------------------
void MM_TakeMenuScreenshot()
{
  char       fName[15];
  void       *pixels = MM_GetScreenBuffer(MM_DRAW_BUFFER);
  sprintf(fName, "shot%i.png", _shotCount++);  // create unique file name
  SaveImage(fName, pixels, MM_SCREEN_WIDTH, MM_SCREEN_HEIGHT, 512, 0);
}

//------------------------------------------------------------------------------
// Name:     MM_PressAnyKeyToContinue
// Summary:  Blocks program until user presses start button
// Inputs:   SDL_Event pointer
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MM_PressAnyKeyToContinue(SDL_Event *event)
{
  int loop = 1;

  // loop until user presses start
  while (loop)
  {
    while (SDL_PollEvent(event))
    {
      switch (event->type)
      {
        case SDL_JOYBUTTONDOWN:
        if( event->jbutton.button == CC_START    ||
            event->jbutton.button == CC_TRIANGLE ||
            event->jbutton.button == CC_CIRCLE   ||
            event->jbutton.button == CC_CROSS    ||
            event->jbutton.button == CC_SQUARE )
        {
          loop = 0;
        }
        break;
      }
    }

    // sleep for a while so we are not just spinning our wheels
    SDL_Delay(100);
  }
}

//------------------------------------------------------------------------------
// Name:     MM_Abs
// Summary:  Gives abslute value of a number (SDK version abs() did not work)
// Inputs:   Number
// Outputs:  None
// Returns:  Absolute value of number
// Cautions: None
//------------------------------------------------------------------------------
int MM_Abs(int val)
{
  if (val < 0)
    val *= -1;
  return(val);
}

//------------------------------------------------------------------------------
// Name:     MM_GetFreeRam
// Summary:  Debug function used to determine how much free ram is available
// Inputs:   Number
// Outputs:  None
// Returns:  Absolute value of number
// Cautions: None
//------------------------------------------------------------------------------
float MM_GetFreeRam()
{
  float ram = 0;
  int i     = 0;
  int ramAdd[320];

  for(i=0; i<320; i++)
  {
    ramAdd[i] = malloc(100000);
    if(ramAdd[i] == 0) //malloc failed
    {
      ram   = (float) i;
      int z = 0;
      for(z=0; z<i; z++)
      {
        free(ramAdd[z]);
      }
      break;
    }
  }
  return(ram/10);
}

// Secret weapon to be used in case game frame rate is too slow.
// Fortunatly due to some excellent program design & planning this
// WMD was never used.
//scePowerSetClockFrequency(333, 333, 166);
