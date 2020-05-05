//-----------------------------------------------------------------------------
//  Class:
//  Menu Manager
//
//  Description:
//  This class handles the drawing of all in game user menues.  This file also
//  implements the game's final level, as the final level is a series of menus
//  which require user input.
//-----------------------------------------------------------------------------

#include "menu_manager.h"
#include "resource_manager.h"
#include "hero_manager.h"
#include "cc_manager.h"
#include "map_manager.h"
#include "bg_manager.h"
#include "SDL_framerate.h"
#include "power_manager.h"
#include <pspkernel.h>


static SDL_Surface *_scr;
static char        _homeDir[255];

//------------------------------------------------------------------------------
// Name:     MUNU_Init
// Summary:  Initializes Menu Manager, called 1 time and 1 time only
// Inputs:   Pointer to a string that contains the path to this program's
//           home directory.
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MUNU_Init(char *path)
{
  int x    = 0;
  _scr     = MM_GetScreenPtr();
  strcpy(_homeDir, path);
  
  // Start at end of string and look backwords for a /.  Everything after
  // first / is the name of this program.  We only want the path to this 
  // program
  for (x=strlen(_homeDir); x >= 0; x--)
  {
     if ( _homeDir[x] == '/')
     {
       _homeDir[x] = 0;
       break;
     }
  }
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawMain
// Summary:  Draws main game menu and accepts user input
// Inputs:   1. SDL_Event pointer
//           2.  Current game level
//           3.  Ding wave.  This sound is loaded in main, and passed in 
//               because MENU_DrawMain plays this sound just before it returns 
//               to main.  Thus it is not safe to free this sound 
//               effects until after it plays, but the SDL function used to
//               check if an SFX has stopped playing does not work.  Thus, we 
//               simply let this SFX remain in memory throughout the game.
//           home directory.
// Outputs:  None
// Returns:  Game Level - Selections in sub-menues may result in the game level
//           being changed
// Cautions: Beware of the Ding wave
//------------------------------------------------------------------------------
unsigned int MENU_DrawMain(SDL_Event *event, unsigned int gameLevel, 
                           Mix_Chunk *ding)
{
  Mix_Chunk   *music  = NULL;
  Mix_Chunk   *select = NULL;
  SDL_Surface *startScreenImg;
  SDL_Surface *textImg;
  SDL_Surface *cursorImg;
  SDL_Surface *versionImg;
  SDL_Surface *tmp;
  int         channel;
  int         chanSel       = -1;
  int         chanDing      = 5;
  int         loop          = 1;
  int         selection     = 1;
  int         showVersion   = 0;
  SDL_Rect    versionRecDst = {433, 142, 0, 0};  
  SDL_Rect    textRecDst    = {186, 163, 0, 0};
  SDL_Rect    cursorRecDst1 = {142, 153, 0, 0};
  SDL_Rect    cursorRecDst2 = {160, 180, 0, 0};
  SDL_Rect    *cursorRecDst = &cursorRecDst1;
  
  ZIP_OpenZipFile(ZIP_MAIN);
  music          = ZIP_LoadMusic("theme.wav");
  select         = ZIP_LoadMusic("menu_select.wav");
  channel        = Mix_PlayChannel(-1, music, 0);
  tmp            = ZIP_LoadImage("startscreen.bmp");
  textImg        = ZIP_LoadImage("start_text.png");
  cursorImg      = ZIP_LoadImage("cursor.png");
  versionImg     = ZIP_LoadImage("version.png");
  startScreenImg = SDL_ConvertSurface(tmp, _scr->format, SDL_SWSURFACE);  
  SDL_FreeSurface(tmp);
  ZIP_CloseZipFile();
  
  // draw main menu
  while (loop)
  {
    SDL_BlitSurface(startScreenImg, 0, _scr, 0);
    SDL_BlitSurface(textImg,        0, _scr, &textRecDst);
    SDL_BlitSurface(cursorImg,      0, _scr, cursorRecDst);
    if (showVersion)
      SDL_BlitSurface(versionImg, 0, _scr, &versionRecDst);
          
    sceDisplayWaitVblankStart();
    SDL_Flip(_scr);
      
    // poll for user input
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
          Mix_PlayChannel(chanDing, ding, 0);
          if (selection == 1)
          {
            loop = 0;
          }
          else if (selection == 2)
          {
            gameLevel = MENU_DrawOptions(event, startScreenImg, cursorImg,
                                         gameLevel, select, ding);
          }
        }  
     
        if(event->jbutton.button == CC_UP)
        {
          cursorRecDst = &cursorRecDst1;
          if (selection != 1)
            chanSel = Mix_PlayChannel(-1, select, 0);
          selection    = 1;
        }  
        if(event->jbutton.button == CC_DOWN)
        {
          cursorRecDst = &cursorRecDst2;
          if (selection != 2)
            chanSel = Mix_PlayChannel(-1, select, 0);
          selection    = 2;
        }
        
        // toggle display of version number (hidden feature if you will)
        if(event->jbutton.button == CC_RIGHT_TRIGGER)
          showVersion = showVersion?0:1;
        
        break;
      }
    }
  }

  Mix_HaltChannel(channel);
  if (chanSel != -1)  // only halt channel if it was started
    Mix_HaltChannel(chanSel);
  Mix_FreeChunk(music);
  Mix_FreeChunk(select);
  
  SDL_FreeSurface(startScreenImg);
  SDL_FreeSurface(textImg);
  SDL_FreeSurface(cursorImg);
  SDL_FreeSurface(versionImg);
  return(gameLevel);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawOptions
// Summary:  Draws the options menu and accepts user input
// Inputs:   1. SDL_Event pointer
//           2. Pointer to Main Menu Image (will use it to draw a faded 
//              backgroiund)
//           3. Cursor Image - for user selection
//           4. gamelevel - current game level
//           5. select - cursor movement SFX
//           6. ding - User selection sfx
// Outputs:  None
// Returns:  Game Level - Selections in sub-menues may result in the game level
//           being changed
// Cautions: None
//------------------------------------------------------------------------------
unsigned int MENU_DrawOptions(SDL_Event *event, SDL_Surface *startScreenImg,
                              SDL_Surface *cursorImg, unsigned int gameLevel, 
                              Mix_Chunk *select, Mix_Chunk *ding)
{
  ZIP_Font *f1; 
  SDL_Surface *enterCodeImg;
  SDL_Surface *viewScreenShotsImg;
  SDL_Surface *mainMenuImg;
  SDL_Surface *configMenuImg;
  int selection             = 0;
  int textImgYOffset        = 0;
  int textHeight            = 0;
  int loop                  = 1;
  SDL_Color fgColor         = {0,0,255};
  SDL_Rect cursorRecDst     = {0,0,0,0};
  SDL_Rect dstRec           = {100,0,0,0}; 
  
  ZIP_OpenZipFile(ZIP_MAIN);
  f1 = ZIP_LoadFont(ZIP_FONT2, 25);
  ZIP_CloseZipFile();

  // Create options text
  mainMenuImg        = TTF_RenderText_Blended(f1->f, "Main Menu", fgColor);
  enterCodeImg       = TTF_RenderText_Blended(f1->f, "Enter Code", fgColor);
  configMenuImg      = TTF_RenderText_Blended(f1->f, 
                                              "Configure Controls", fgColor);
  viewScreenShotsImg = TTF_RenderText_Blended(f1->f, 
                                              "View Screenshots", fgColor);
  
  // fade start screen image
  SDL_SetAlpha(startScreenImg, SDL_SRCALPHA, 100);  
  
  // Set text location on screen
  textImgYOffset = (MM_SCREEN_HEIGHT - (enterCodeImg->h * 4)) / 2;
  textHeight     = enterCodeImg->h;
  cursorRecDst.x = dstRec.x - cursorImg->w - 5;
  
  // Draw options to user
  while (loop)
  {
    SDL_FillRect(_scr, 0, 0);
    SDL_BlitSurface(startScreenImg, 0, _scr, 0);

    dstRec.y = textImgYOffset + (0*textHeight);;
    SDL_BlitSurface(mainMenuImg, 0, _scr, &dstRec);

    dstRec.y = textImgYOffset + (1*textHeight);
    SDL_BlitSurface(enterCodeImg, 0, _scr, &dstRec);

    dstRec.y = textImgYOffset + (2*textHeight);
    SDL_BlitSurface(viewScreenShotsImg, 0, _scr, &dstRec);    

    dstRec.y = textImgYOffset + (3*textHeight);
    SDL_BlitSurface(configMenuImg, 0, _scr, &dstRec);    

    cursorRecDst.y = textImgYOffset + (selection * textHeight);
    SDL_BlitSurface(cursorImg, 0, _scr, &cursorRecDst);
      
    sceDisplayWaitVblankStart();
    SDL_Flip(_scr);
     
    // Poll for user input
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
        
        // Select current option
        if( event->jbutton.button == CC_START    || 
            event->jbutton.button == CC_TRIANGLE ||
            event->jbutton.button == CC_CIRCLE   ||
            event->jbutton.button == CC_CROSS    || 
            event->jbutton.button == CC_SQUARE )
        {
          Mix_PlayChannel(-1, ding, 0);
          if (selection == 0)
          {
            loop = 0;
          }
          else if (selection == 1)
          {
            gameLevel = MENU_DrawEnterCode(event, gameLevel);
          }
          else if (selection == 2)
          {
            MENU_DrawViewScreenshots(event);
          }
          else if (selection == 3)
          {
            CC_ConfigureControls(event);
          }
        }  
        
        // Move cursor
        if(event->jbutton.button == CC_UP)
        {
          Mix_PlayChannel(-1, select, 0);
          selection--;
        }  
        if(event->jbutton.button == CC_DOWN)
        {
          Mix_PlayChannel(-1, select, 0);
          selection++;
        }
        break;
      }
    }
    
    // bounds checking on selection
    if (selection > 3)
      selection = 0;
    else if (selection < 0)
      selection = 3;
  }
  
  ZIP_CloseFont(f1);
  SDL_FreeSurface(enterCodeImg);
  SDL_FreeSurface(viewScreenShotsImg);
  SDL_FreeSurface(mainMenuImg);
  SDL_FreeSurface(configMenuImg);
  SDL_SetAlpha(startScreenImg, SDL_SRCALPHA, 255);  
  return(gameLevel);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawViewScreenshots
// Summary:  Menu used to navigate through screenshots saved by user
// Inputs:   SDL_Event pointer
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MENU_DrawViewScreenshots(SDL_Event *event)
{
  int         dfd;
  char        *buffer[500];
  char        buf[500];
  int         strLen;
  SceIoDirent *dir;
  ZIP_Font    *f1;
  int count            = 0;
  int loop             = 1;
  int newImage         = 1;
  int infoFlag         = 1;
  int index            = 0;
  SDL_Surface *img     = 0;
  SDL_Surface *infoImg = 0;
  SDL_Surface *txtImg  = 0; 
  SDL_Surface *nameImg = 0;
  SDL_Color fgColor    = {255,0,0};
  SDL_Rect txtRecDst   = {0,0,0,0};
  SDL_Rect nameRecDst  = {0,0,0,0};
  
  // if not static, PSP will just up and die
  static SceIoDirent dir1;
  dir    = &dir1;
  dfd    = sceIoDopen(_homeDir);
  buf[0] = 0;
  
  // Read through directory and compile a list of PNG files
  if (dfd > 0)
  {
    while ( sceIoDread(dfd, dir) )
    {
      strLen = strlen(dir->d_name);
      if (strLen > 4)
      {
        if ( dir->d_name[strLen-4] == '.' && 
            (dir->d_name[strLen-3] == 'p' || dir->d_name[strLen-3] == 'P') &&
            (dir->d_name[strLen-2] == 'n' || dir->d_name[strLen-2] == 'N') &&
            (dir->d_name[strLen-1] == 'g' || dir->d_name[strLen-1] == 'G') )
        {
          buffer[count] = (char *) malloc(strlen(dir->d_name));
          strcpy(buffer[count], dir->d_name);
          count++;
        }
      }
    }
  }
  
  // If no PNG files were present, display message to user
  if (count == 0)
  {
    SDL_Rect dstRec;
    ZIP_OpenZipFile(ZIP_MAIN);
    img = ZIP_LoadImage("no_shots.bmp");
    ZIP_CloseZipFile();
    if (img)
    {
      unsigned int color = SDL_MapRGB(_scr->format, 33, 10, 48);
      dstRec.x = (MM_SCREEN_WIDTH / 2) - (img->w / 2);
      dstRec.y = (MM_SCREEN_HEIGHT / 2) - (img->h / 2);
      SDL_FillRect(_scr, 0, color);
      SDL_BlitSurface(img, 0, _scr, &dstRec);
      sceDisplayWaitVblankStart();
      SDL_Flip(_scr);
      MM_PressAnyKeyToContinue(event);
      SDL_FreeSurface(img);
    }
    else
    {
      EH_Error(EH_INFO, "No Screenshots were present.\n"); 
      MM_PressAnyKeyToContinue(event);
    }
  }
  // If PNGs are present, display them to user
  else
  {
    ZIP_OpenZipFile(ZIP_MAIN);
    f1 = ZIP_LoadFont(ZIP_FONT1, 12);
    ZIP_CloseZipFile();
    
    // Create surface used to toggle information about current image to user
    infoImg = SDL_CreateRGBSurface(SDL_SWSURFACE, MM_SCREEN_WIDTH, 38, 
              _scr->format->BitsPerPixel,  _scr->format->Rmask, 
              _scr->format->Gmask, _scr->format->Bmask, _scr->format->Amask);
    SDL_FillRect(infoImg, 0, 0);
    SDL_SetAlpha(infoImg, SDL_SRCALPHA, 225);

    // Loop used to process user input and display selected image
    while (loop)
    {
      // User has selected a new image, update the image information window
      if(newImage)
      {
        // Free old image and image information
        if (img)
          SDL_FreeSurface(img);
        if (txtImg)
          SDL_FreeSurface(txtImg);
        if (nameImg)
          SDL_FreeSurface(nameImg); 
          
        if (index < 0)
          index = count-1;
        else if (index >= count)
          index = 0;
        
        // Load new image and create new image information
        newImage = 0;
        img      = IMG_Load(buffer[index]);  
        sprintf(buf, "Image: %s/%s", _homeDir, buffer[index]);
        txtImg  = TTF_RenderText_Blended(f1->f, "Left Trigger=Prev    Start=Main Menu    Select=Toggle Info    Right Trigger=Next", fgColor);
        nameImg = TTF_RenderText_Blended(f1->f, buf, fgColor);
        txtRecDst.x  = (MM_SCREEN_WIDTH - txtImg->w) / 2;
        nameRecDst.x = (MM_SCREEN_WIDTH - nameImg->w) / 2;
        nameRecDst.y = txtImg->h - 5;
      }

      // Draw image to screen, draw image info to screen if user wants it
      SDL_BlitSurface(img, 0, _scr, 0);
      if (infoFlag)
      {
        SDL_BlitSurface(infoImg, 0, _scr, 0);
        SDL_BlitSurface(txtImg,  0, _scr, &txtRecDst);
        SDL_BlitSurface(nameImg, 0, _scr, &nameRecDst);
      }
      
      sceDisplayWaitVblankStart();
      SDL_Flip(_scr);
      
      // Poll for user input
      while (SDL_PollEvent(event)) 
      {
        switch (event->type) 
        {
          case SDL_JOYBUTTONDOWN:
          if(event->jbutton.button == CC_START    || 
             event->jbutton.button == CC_TRIANGLE ||
             event->jbutton.button == CC_CIRCLE   ||
             event->jbutton.button == CC_CROSS    || 
             event->jbutton.button == CC_SQUARE )
          {
            loop = 0;
          }  
       
          if(event->jbutton.button == CC_LEFT_TRIGGER)
          {
            index--;
            newImage = 1;
          }  
          if(event->jbutton.button == CC_RIGHT_TRIGGER)
          {
            newImage = 1;
            index++;
          }
          
          // toggle image info on/off
          if(event->jbutton.button == CC_SELECT)
          {
            infoFlag = (infoFlag == 1)?0:1;
          }
          break;
        }  // END switch
      }    // END while (SDL_PollEvent(event)) 
    }      // END while (loop)
    
    // Free all image information
    ZIP_CloseFont(f1);
    SDL_FreeSurface(img);
    SDL_FreeSurface(txtImg);
    SDL_FreeSurface(nameImg); 
    SDL_FreeSurface(infoImg); 
  } 
  
  // Free memory allocated to hold image names
  for (index=0; index < count; index++)
    free(buffer[index]);
    
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawGameOver
// Summary:  Draws game over message and waits for user to press start to 
//           continue
// Inputs:   SDL_Event pointer
// Outputs:  None
// Returns:  None
// Cautions: this function expects the current level to have the 
//           RM_GAME_OVER_TXT image loaded by the resource manager
//------------------------------------------------------------------------------
unsigned int MENU_DrawGameOver(SDL_Event *event)
{
  // line size, time screen height,m times 2 since we are in 16 bit mode
  unsigned int bSize     = 512 * MM_SCREEN_HEIGHT * 2;  
  unsigned short *scrBuf = (unsigned short*) malloc(bSize);
  
  RM_PlaySound(RM_SFX_GAME_OVER);
  
  if (scrBuf)
  {
    SDL_Surface *s           = RM_GetImage(RM_GAME_OVER_TXT);
    SDL_Surface *progBarImg  = RM_GetImage(RM_IMG_PROGRESS_BAR);
    SDL_Surface *progIconImg = RM_GetImage(RM_IMG_PROGRESS_ICON);
    SDL_Rect progBarRecDst   = {134,6,0,0};  
    SDL_Rect progIconRecDst  = {0,17,0,0}; 
    int percentComplete      = 0;
    int progLineLen          = 150;            
    float percent            = 0;
    int progXOffset          = 9;        
    int progYOffset          = 11;         
  
    progBarRecDst.x = (MM_SCREEN_WIDTH/2) -  (progBarImg->w/2);
    
    // Set X location of progress icon on progress bar
    percent           = BG_GetxPosGlobal() / MAP_GetLevelSize(MM_LEVEL1);
    progIconRecDst.x  = progXOffset + progBarRecDst.x + (percent * progLineLen);    
    progIconRecDst.y  = progYOffset + progBarRecDst.y;

    // Get what's on screen and copy it to a local buffer
    void *pixels   = MM_GetScreenBuffer(MM_DRAW_BUFFER);
    memcpy(scrBuf, pixels, bSize);
    
    // Create SDL Surface using pixel data from above
    SDL_Surface *scr  = SDL_CreateRGBSurfaceFrom(
                        scrBuf, MM_SCREEN_WIDTH, MM_SCREEN_HEIGHT, 
                        _scr->format->BitsPerPixel, _scr->pitch, 
                        _scr->format->Rmask, _scr->format->Gmask, 
                        _scr->format->Bmask, _scr->format->Amask);
    SDL_Rect dstRec;
    dstRec.x = (MM_SCREEN_WIDTH/2) -  (s->w/2);
    dstRec.y = (MM_SCREEN_HEIGHT/2) - (s->h/2);
    
    // Draw final image from level, game over text, and progress bar to screen
    SDL_BlitSurface(scr, 0, _scr, 0);  
    SDL_BlitSurface(s, 0, _scr, &dstRec);  
    SDL_BlitSurface(progBarImg,  0, _scr, &progBarRecDst);  
    SDL_BlitSurface(progIconImg, 0, _scr, &progIconRecDst);  

    SDL_Flip(_scr);
    SDL_FreeSurface(scr);
    free(scrBuf);
  }

  MM_PressAnyKeyToContinue(event);
  RM_StopSound(-1);
  return(MM_STATE_MAIN_MENU);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawPauseGame
// Summary:  Loops until the user presses the 'Start' button on the game pad
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
unsigned int MENU_DrawPauseGame(SDL_Event *event, unsigned int gameState)
{
  int loop                 = 1;
  unsigned int bSize       = 512 * MM_SCREEN_HEIGHT * 2;  
  int selection            = 0;
  int curYOffset           = 0;
  int progXOffset          = 9;        
  int progYOffset          = 11;         
  float percent            = 0;
  int percentComplete      = 0;
  int progLineLen          = 150;          
  SDL_Rect dstRec          = {0,0,0,0};
  SDL_Rect optionsRecDst   = {0,0,0,0};
  SDL_Rect cursorRecDst    = {0,0,0,0};
  SDL_Rect progBarRecDst   = {134,6,0,0};  
  SDL_Rect progIconRecDst  = {0,17,0,0};   
  SDL_Surface *progBarImg  = 0;
  SDL_Surface *progIconImg = 0;
  SDL_Surface *cursorImg   = 0;
  SDL_Surface *optionsImg  = 0;
  SDL_Surface *pausedImg   = 0;
  SDL_Surface *scr         = 0;
  void        *pixels      = 0;
  
  
  unsigned short *scrBuf = (unsigned short*) malloc(bSize);
  
  if (scrBuf)
  {
    pausedImg   = RM_GetImage(RM_GAME_PAUSED_TXT);
    optionsImg  = RM_GetImage(RM_RESUME_QUIT_TXT);
    cursorImg   = RM_GetImage(RM_CURSOR);
    progBarImg  = RM_GetImage(RM_IMG_PROGRESS_BAR);
    progIconImg = RM_GetImage(RM_IMG_PROGRESS_ICON);
    
    pixels = MM_GetScreenBuffer(MM_DRAW_BUFFER);
    memcpy(scrBuf, pixels, bSize);
    scr  = SDL_CreateRGBSurfaceFrom(scrBuf, MM_SCREEN_WIDTH, MM_SCREEN_HEIGHT, 
              _scr->format->BitsPerPixel, _scr->pitch, _scr->format->Rmask, 
              _scr->format->Gmask, _scr->format->Bmask, _scr->format->Amask);
              
    progBarRecDst.x = (MM_SCREEN_WIDTH/2) -  (progBarImg->w/2);
    
    // Set X location of progress icon on progress bar
    percent           = BG_GetxPosGlobal() / MAP_GetLevelSize(MM_LEVEL1);
    progIconRecDst.x  = progXOffset + progBarRecDst.x + (percent * progLineLen);    
    progIconRecDst.y  = progYOffset + progBarRecDst.y;
    
    // Center Paused Image Horizontally, 
    // Offset Vertical placement by given amount
    dstRec.x = (MM_SCREEN_WIDTH/2) -  (pausedImg->w/2);
    dstRec.y = (MM_SCREEN_HEIGHT/2) - ((pausedImg->h + optionsImg->h + 5)/2);
    
    // Place the Options Image relative to the Paused Image Vertically
    // Center paused image horizontally
    optionsRecDst.x = (MM_SCREEN_WIDTH/2) -  (optionsImg->w/2);
    optionsRecDst.y = dstRec.y + pausedImg->h + 30;
    
    // Place cursor relative to options image
    cursorRecDst.x  = optionsRecDst.x - 10 - cursorImg->w;
    cursorRecDst.y  = optionsRecDst.y;
    curYOffset      = optionsImg->h / 2;
    SDL_SetAlpha(scr, SDL_SRCALPHA, 80);  // apply alpha fading to image
    RM_PauseSound(-1);  // pause all sounds if above stuff worked
  }
  else
  {
    loop = 0; 
  }
  
  while (loop)       // loop until user presses start
  {
    SDL_FillRect(_scr, 0, 0);
    SDL_BlitSurface(scr,         0, _scr, 0);  
    SDL_BlitSurface(pausedImg,   0, _scr, &dstRec);  
    SDL_BlitSurface(optionsImg,  0, _scr, &optionsRecDst);  
    SDL_BlitSurface(cursorImg,   0, _scr, &cursorRecDst);  
    SDL_BlitSurface(progBarImg,  0, _scr, &progBarRecDst);  
    SDL_BlitSurface(progIconImg, 0, _scr, &progIconRecDst);  
    
    sceDisplayWaitVblankStart();
    SDL_Flip(_scr);
    
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
        
        // check to see if user changes directions or stops moving
        if(event->jbutton.button == _CCCtrlMoveRight)
          HM_MoveRight();
        else if(event->jbutton.button == _CCCtrlMoveLeft)
          HM_MoveLeft();
        
        // Check to see if user is scrolling through options
        if(event->jbutton.button == CC_UP)
          selection = 0;
        else if(event->jbutton.button == CC_DOWN)
          selection = 1;
        
        // Check to see if user pressed the duck button
        if(event->jbutton.button == _CCCtrlDuck )
          HM_Duck();
        
        // Make hero run if user holds down run button
        if(event->jbutton.button == _CCCtrlRun)
            HM_StartRunning();
        
        if(event->jbutton.button  == _CCCtrlPause  ||
           event->jbutton.button  == CC_START      || 
            event->jbutton.button == CC_TRIANGLE   ||
            event->jbutton.button == CC_CIRCLE     ||
            event->jbutton.button == CC_CROSS      || 
            event->jbutton.button == CC_SQUARE)
        {
          loop = 0;
          if (selection == 1)
            gameState = MM_STATE_MAIN_MENU;
          else
            RM_ResumeSound(-1);
        }    
        
        break;
        
        case SDL_JOYBUTTONUP:
        if(event->jbutton.button == _CCCtrlMoveLeft || 
           event->jbutton.button == _CCCtrlMoveRight )
          HM_StopMoving();
        
        // Check to see if user released the duck button
        if(event->jbutton.button == _CCCtrlDuck )
          HM_StopDuck();
          
        if(event->jbutton.button == _CCCtrlRun )
          HM_StopRunning();  
        
        break;
      }
    }
    
    if (selection == 0)
    {
      cursorRecDst.x  = optionsRecDst.x - 10 - cursorImg->w;
      cursorRecDst.y  = optionsRecDst.y;
    }
    else
    {
      cursorRecDst.x  = optionsRecDst.x - cursorImg->w;
      cursorRecDst.y  = optionsRecDst.y + curYOffset;
    }
  }
  
  if (scr) // free faded surface
    SDL_FreeSurface(scr);
    
  if (scrBuf) // free pixel data
    free(scrBuf);
    
  return(gameState);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawEnterCode
// Summary:  Menu used to allow user to enter a secret code!
// Inputs:   1. SDL_Event pointer
//           2. The current game level
// Outputs:  None
// Returns:  Current game level.  As a result of a code being eneterd this 
//           value can change
// Cautions: None
//------------------------------------------------------------------------------
unsigned int MENU_DrawEnterCode(SDL_Event *event, unsigned int gameLevel)
{
  int w, h, x;
  int startText;
  SDL_Surface *digits, *t1, *t2, *t3, *block;
  unsigned int secretCode = 0;
  int validCodeGiven = 0;
  int index = 0;
  int loop  = 1;
  int alpha = 0;
  char dig[7];  // 7th digit is reserved for null character
  int amount = 5;
  SDL_Rect srcRec, dstRec;
  SDL_Rect t1DstRec, t2DstRec;
  int channel;
  Mix_Chunk   *m1 = 0;
  Mix_Chunk   *m2 = 0;
 
  
  // Did you think it would be this easy to find the secret codes??? BAH!
  // Be a real MAN and play the game from start to finish.  Then you'll have
  // your code.  These are only DECOYS!
  #define NUM_SECRET_CODES 5
  unsigned int codes[NUM_SECRET_CODES][2];
  codes[0][0] = 1; 
  codes[0][1] = 0;  // special case, make's hero Invincible!
  codes[1][0] = 2; 
  codes[1][1] = MM_LEVEL_FINAL;
  codes[2][0] = 3; 
  codes[2][1] = MM_LEVEL_CREDITS;
  codes[3][0] = 4; 
  codes[3][1] = MM_LEVEL_HIDDEN1;
  codes[4][0] = 5;  
  codes[4][1] = 0;  // special case, infinity extra lives
  
  for (x=0; x < 7; x++)
  {
    dig[x] = 0;
  }
  
  ZIP_OpenZipFile(ZIP_MAIN);
  digits = ZIP_LoadImage("digits.png");
  t1     = ZIP_LoadImage("entercode.png");
  t2     = ZIP_LoadImage("pressstart.png");
  t3     = ZIP_LoadImage("access.png");
  m1     = ZIP_LoadMusic("granted.wav");
  m2     = ZIP_LoadMusic("denied.wav");
  ZIP_CloseZipFile();
  w = digits->w / 10;
  h = digits->h;
  startText = (MM_SCREEN_WIDTH/2) - (w*3); 
  srcRec.x = 0;
  srcRec.y = 0;
  srcRec.w = w;
  srcRec.h = h;
  dstRec.x = 0;
  dstRec.y   = (MM_SCREEN_HEIGHT/2) - (h/2);
  t1DstRec.x = (MM_SCREEN_WIDTH/2)  - (t1->w / 2);
  t2DstRec.x = (MM_SCREEN_WIDTH/2)  - (t2->w / 2);
  t1DstRec.y = 40;
  t2DstRec.y = 200;

  block = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 
          _scr->format->BitsPerPixel, _scr->format->Rmask, 
          _scr->format->Gmask, _scr->format->Bmask, _scr->format->Amask);
    
  SDL_FillRect(block, 0, 0);
  
  while (loop)       // loop until user presses start
  {
    
    SDL_FillRect(_scr, 0, 0);
    for (x=0; x < 6; x++)
    {
      srcRec.x = w * dig[x];
      dstRec.x = startText + (x * w);
      SDL_BlitSurface(digits, &srcRec, _scr, &dstRec);
    }
    
    dstRec.x = startText + (index * w);
    SDL_BlitSurface(block, 0, _scr, &dstRec);
    SDL_BlitSurface(t1, 0, _scr, &t1DstRec);
    SDL_BlitSurface(t2, 0, _scr, &t2DstRec);
    
    sceDisplayWaitVblankStart();
    SDL_Flip(_scr);
    alpha += amount;
    if (alpha > 150)
    {
      alpha = 150;
      amount = amount * -1;
    }
    else if (alpha < 0)
    {
      alpha = 0;
      amount = amount * -1;
    }
    SDL_SetAlpha(block, SDL_SRCALPHA, alpha); 

    
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
          if(event->jbutton.button == CC_START    || 
             event->jbutton.button == CC_TRIANGLE ||
             event->jbutton.button == CC_CIRCLE   ||
             event->jbutton.button == CC_CROSS    || 
             event->jbutton.button == CC_SQUARE )
          {
            loop = 0;
          }
          if(event->jbutton.button == CC_RIGHT)
            index++;
          if(event->jbutton.button == CC_LEFT)
            index--;
          if(event->jbutton.button == CC_UP)
            dig[index]++;
          if(event->jbutton.button == CC_DOWN)
            dig[index]--;
          break;
      }
    }
    
    if (index > 5)
      index = 0;
    if (index < 0)
      index = 5;
    
    if (dig[index] > 9)
      dig[index] = 0;
    if (dig[index] < 0)
      dig[index] = 9;
  }
  
  // convert numerical value to a character string
  for (x=0; x < 6; x++)
    dig[x] += 48;  

  // convert character string to an integer
  secretCode = (unsigned int) atoi(dig);
  
  // check and see if a valid code was given
  for (x=0; x < NUM_SECRET_CODES; x++)
  {
    if (secretCode == codes[x][0])
    {
      validCodeGiven = 1;
      if (x == 0)
        HM_HeroInvulnerable();
      else if (x == 4)
        PM_ActivateInfinityLives();
      else
        gameLevel = codes[x][1];
    }
  }
  

  dstRec.y = (MM_SCREEN_HEIGHT/2) - (t3->h / 2 / 2);
  dstRec.x = (MM_SCREEN_WIDTH/2) -  (t3->w / 2 );
  srcRec.y = 0;
  srcRec.x = 0;
  srcRec.w = t3->w;
  srcRec.h = t3->h / 2;

  // Draw Access Denied/Granted image
  if (validCodeGiven)
  {
    channel = Mix_PlayChannel(-1, m1, 0);
    srcRec.y = 0;
  }
  else
  {
    channel = Mix_PlayChannel(-1, m2, 0);
    srcRec.y = t3->h / 2;
  }
  
  SDL_FillRect(_scr, 0, 0);
  SDL_BlitSurface(t1, 0, _scr, &t1DstRec);
  SDL_BlitSurface(t3, &srcRec, _scr, &dstRec);
  SDL_Flip(_scr);
  SDL_Delay(3000); 

  SDL_FreeSurface(block);
  SDL_FreeSurface(digits);
  SDL_FreeSurface(t1);
  SDL_FreeSurface(t2);
  SDL_FreeSurface(t3);
  
  Mix_HaltChannel(channel);
  Mix_FreeChunk(m1);
  Mix_FreeChunk(m2);
  return(gameLevel);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawLoadScreen
// Summary:  Screen that displays a witty quote to hold user over until the 
//           main game level loads
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MENU_DrawLoadScreen()
{
  #define NUM_QUOTES 35
  int i, w, h;
  int rand;
  int length;
  char *quotes[NUM_QUOTES];
  char *p;
  char buffer[500];
  ZIP_Font *f1;
  ZIP_Font *f2;
  ZIP_Font *f3;
  int lastSpace            = 0;
  int start                = 0;
  SDL_Surface *txtImg      = 0;
  SDL_Surface *titleImg    = 0;
  SDL_Surface *subTitleImg = 0;
  SDL_Surface *loadImg     = 0;
  SDL_Color color          = {255,255,255};
  SDL_Rect dst             = {0,0,0,0};

  quotes[0]  = "There is no theory of evolution. Just a list of animals Chuck Norris allows to live.\n-Unknown";
  quotes[1]  = "The chief export of Chuck Norris is Pain.\n-Unknown";
  quotes[2]  = "That's it man.  Game over man... Game over!\n-Bill Paxton (Aliens)";
  quotes[3]  = "They mostly come out at night, mostly.\n-Newt (Aliens)";
  quotes[4]  = "Zed's dead baby... Zed's dead.\n-Bruce Willis (Pulp Fiction)";
  quotes[5]  = "Hello, my name is Inigo Montoya. You killed my father: prepare to die.\n-Inigo Montoya (Princess Bride)";
  quotes[6]  = "When my people come to colonize this planet, you will be on the protected rolls, and no harm will come to you.\n-Dan Aykroyd (Coneheads)";
  quotes[7]  = "I'm sorry, Dave. I'm afraid I can't do that.\n-HAL 9000 (2001 A Space Odyssey)";
  quotes[8]  = "Oh not to worry Charlie, you'll go to Heaven.  All Dog's go to heaven...\n-Whippet Angel (All Dogs Go To Heaven)";
  quotes[9]  = "Well, ain't this place a geographical oddity. Two weeks from everywhere!\n-George Clooney (O Brother Where Art Thou)";
  quotes[10] = "No thank you, Delmar. One third of a gopher would only arouse my appetite without bedding it down.\n-George Clooney (O Brother Where Art Thou)";
  quotes[11] = "We are not enemies, but friends. We must not be enemies. Though passion may have strained it must not break our bonds of affection. The mystic chords of memory, stretching from every battlefield and patriot grave to every living heart and hearthstone all over this broad land, will yet swell the chorus of the Union, when again touched, as surely they will be, by the better angels of our nature.\n-Abraham Lincoln (First Inaugural Address)";
  quotes[12] = "Both parties deprecated war; but one of them would make war rather than let the nation survive; and the other would accept war rather than let it perish. And the war came.\n-Abraham Lincoln (Second Inaugural Address)\n";
  quotes[13] = "It's not a tumor!\n-Arnold Schwarzenegger (Kindergarten Cop)";
  quotes[14] = "Based on the findings of the report, my conclusion was that this idea was not a practical deterrent for reasons which at this moment must be all too obvious.\n-Dr. Strangelove (Dr. Strangelove)";
  quotes[15] = "One hippo, alone once more, misses the other forty-four.\n-Sandra Boynton (Hippos Go Berserk)";
  quotes[16] = "Please God! We need a cab!\n-Randy Quaid (Quick Change)";
  quotes[17] = "This famous linguist once said that of all the phrases in the English language, of all the endless combinations of words in all of history, that Cellar Door is the most beautiful.\n-Drew Barrymore (Donnie Darko)";
  quotes[18] = "He Didn't fall? INCONCEIVABLE.\n-Vizzini\nYou keep using that word. I do not think it means what you think it means.\n-Inigo Montoya (The Princess Bride)";
  quotes[19] = "Hey, we all need friends in here. I could be a friend to you.\n-Boggs (The Shawshank Redemption)";
  quotes[20] = "I have to remind myself that some birds aren't meant to be caged. Their feathers are just too bright. And when they fly away, the part of you that knows it was a sin to lock them up DOES rejoice. Still, the place you live in is that much more drab and empty that they're gone.\n-Morgan Freeman (The Shawshank Redemption)";
  quotes[21] = "These aren't the droids you're looking for.\n-Sir Alec Guinness (Star Wars)";
  quotes[22] = "Aw, but I was going to Tashi station to pick up some power converters!\n-Mark Hamill (Star Wars)";
  quotes[23] = "LET'S ROCK!\n-Vasquez (Aliens)";
  quotes[24] = "You're my boy, Blue!\n-Will Ferrell (Old School)";
  quotes[25] = "What's the matter, Colonel Sandurz? CHICKEN?\n-Dark Helmet (Spaceballs)"; 
  quotes[26] = "You are right, I have always known about man. From the evidence, I believe his wisdom must walk hand and hand with his idiocy. His emotions must rule his brain. He must be a warlike creature who gives battle to everything around him, even himself.\n-Dr. Zaius (Planet Of The Apes)";
  quotes[27] = "Take your stinking paws off me, you damned dirty ape!\n-Charlton Heston (Planet Of The Apes)";
  quotes[28] = "This is for all you new people: I only have one rule. Everyone fights. No one quits. You don't do your job, I'll shoot you myself. You get me?\n-Jean Rasczak (Starship Troopers)";
  quotes[29] = "The Skynet Funding Bill is passed. The system goes on-line August 4th, 1997. Human decisions are removed from strategic defense. Skynet begins to learn at a geometric rate. It becomes self-aware at 2:14 a.m. Eastern time, August 29th. In a panic, they try to pull the plug.\n-Arnold Schwarzenegger (Terminator 2)"; 
  quotes[30] = "First of all, Papa Smurf didn't create Smurfette. Gargamel did. She was sent in as Gargamel's evil spy with the intention of destroying the Smurf village. But the overwhelming goodness of the Smurf way of life transformed her.\n-Jake Gyllenhaal (Donnie Darko)";
  quotes[31] = "Indy! Cover your heart! Cover your heart!\n-Short Round (Indiana Jones and the Temple of Doom)";
  quotes[32] = "Playing with my money is like playing with my emotions, Smokey.\n-Big Worm (Friday)";
  quotes[33] = "No sugar? Damn. Y'all ain't never got two things that match. Either ya got Kool-aid, no sugar. Peanut butter, no jelly. Ham, no burger. Daaamn.\n-Smokey (Friday)";
  quotes[34] = "Oh, the THINKS you can think up if only you try!\n-Dr. Seuss (Oh, The THINKS You Can Think)";
  
  
  ZIP_OpenZipFile(ZIP_MAIN);
  f1 = ZIP_LoadFont(ZIP_FONT2, 30);
  f2 = ZIP_LoadFont(ZIP_FONT2, 12);
  f3 = ZIP_LoadFont(ZIP_FONT2, 13);
  ZIP_CloseZipFile();
  
  rand = MM_RandomNumberGen(0, NUM_QUOTES-1);
  strcpy(buffer, quotes[rand]);
  p = &buffer[0];
  
  titleImg    = TTF_RenderText_Blended(f1->f, "Mega-Mart", color);
  subTitleImg = TTF_RenderText_Blended(f3->f, "or: How I Learned To Stop Living In Poverty And Love The Corporation", color);
  loadImg     = TTF_RenderText_Blended(f3->f, "Now Loading...", color);
  
  SDL_FillRect(_scr, 0, 0);
  dst.x = (MM_SCREEN_WIDTH/2) - (titleImg->w/2);
  SDL_BlitSurface(titleImg, 0, _scr, &dst);
  
  dst.y = titleImg->h;
  dst.x = (MM_SCREEN_WIDTH/2) - (subTitleImg->w/2);
  SDL_BlitSurface(subTitleImg, 0, _scr, &dst);

  dst.y += subTitleImg->h;
  dst.x = (MM_SCREEN_WIDTH/2) - (loadImg->w/2);
  SDL_BlitSurface(loadImg, 0, _scr, &dst);
  
  dst.y += loadImg->h + 20;
  
  length = strlen(p);
  
  for (i=0; i < length+1; i++)
  {
    if (p[i] == ' ')  // break sentance on spaces
    {
      p[i] = 0;  // set end of string to current space
      // get length of current string
      TTF_SizeText(f2->f, &p[start], &w, &h); 
      p[i] = ' '; // turn null back into space
      // if string is too long, print what we have so far
      if (w > MM_SCREEN_WIDTH) 
      {
        // Special check for cases where > SW consecutive characters appear
        if (lastSpace<start) lastSpace = i;
        // find last space that did NOT make the string too long
        p[lastSpace] = 0; 
        txtImg = TTF_RenderText_Blended(f2->f, &p[start], color);
        start = lastSpace+1; // set new start of screen to the very next space
      }
      lastSpace = i; // allways set the last space to the last space seen
    }
    else if (p[i] == '\n') // handle newlines
    {
      p[i] = 0; // set end of string to curent newline
      TTF_SizeText(f2->f, &p[start], &w, &h);
      p[i] = '\n'; // set back to newline
      if (w > MM_SCREEN_WIDTH) // if string is to long
      {
        // Special check for cases where > SW consecutive characters appear
        if (lastSpace<start) lastSpace = i;
        p[lastSpace] = 0;  // find previous space before newline.  
        txtImg = TTF_RenderText_Blended(f2->f, &p[start], color);
        start = lastSpace+1; // set new string start position
        // Set string index back 1 so newline will be processed again
        if (lastSpace != i) i--; 
      }
      else // string was not to long
      {
        p[i] = 0; // set null temrinator to where newline is and print string
        txtImg = TTF_RenderText_Blended(f2->f, &p[start], color);
        start = i+1; // set new start position
      }
    }
    else if (p[i] == 0)  // if end of string is reached...
    {
      TTF_SizeText(f2->f, &p[start], &w, &h);
      // if to long, print from start to previous space
      if (w > MM_SCREEN_WIDTH)  
      {
        // Special check for cases where > SW consecutive characters appear
        if (lastSpace<start) lastSpace = i;
        p[lastSpace] = 0;
        txtImg = TTF_RenderText_Blended(f2->f, &p[start], color);
        start = i = lastSpace+1;
      }
      else  // if not to long, just print what is left
        txtImg = TTF_RenderText_Blended(f2->f, &p[start], color);
    }
    
    // draw only when there is something to draw
    if (txtImg && txtImg->w > 0)
    {
      dst.x = (MM_SCREEN_WIDTH/2) - (txtImg->w/2);
      SDL_BlitSurface(txtImg, 0, _scr, &dst);
      SDL_FreeSurface(txtImg);
      txtImg = 0;
      dst.y += h; // move down to next line
    }
  }
  
  SDL_Flip(_scr);
  ZIP_CloseFont(f1);
  ZIP_CloseFont(f2);
  ZIP_CloseFont(f3);
  SDL_FreeSurface(titleImg);
  SDL_FreeSurface(subTitleImg);
  SDL_FreeSurface(loadImg);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawFinalLevel
// Summary:  Draws and processes user input for the final level of the game
// Inputs:   SDL_Event pointer
// Outputs:  None
// Returns:  Current game level
// Cautions: None
//------------------------------------------------------------------------------
unsigned int MENU_DrawFinalLevel(SDL_Event *event)
{
  #define MAX_OPTIONS  4
  
  SDL_Surface *scrollImg;
  SDL_Surface *textImg;
  SDL_Surface *cursorImg;
  SDL_Surface *finalImg;
  SDL_Rect    scroollSrcRec;
  SDL_Rect    textSrcRec;
  SDL_Rect    textDstRec;
  SDL_Rect    curDstRec;
    
  int curIndex[MAX_OPTIONS];
  int numTxtFrm    = 4;
  int txtFrmHeight = 0;
  int loopFlag     = 1;
  int scrollAmount = 0;
  int index        = 0;
  int txtFrame     = 0;
  int updateFlag   = 0;
  int frmDelCur    = 0;
  int gameState;
  FPSmanager  fpsMan;
  
  scrollImg = RM_GetImage(RM_IMG_FL_SCROLL);
  textImg   = RM_GetImage(RM_IMG_FL_TEXT);
  cursorImg = RM_GetImage(RM_IMG_FL_CURSOR);
  finalImg  = RM_GetImage(RM_IMG_FL_IMAGE);
   
  SDL_FillRect(_scr, 0, 0);

  txtFrmHeight = textImg->h / numTxtFrm;

  curIndex[0] = 6;
  curIndex[1] = 69;
  curIndex[2] = 128;
  curIndex[3] = 171;

  scroollSrcRec.y = 0;
  scroollSrcRec.x = 0;
  scroollSrcRec.w = scrollImg->w;
  scroollSrcRec.h = MM_SCREEN_HEIGHT;
  
  textSrcRec.y = 0;
  textSrcRec.x = 0;
  textSrcRec.w = textImg->w;
  textSrcRec.h = MM_SCREEN_HEIGHT;
  
  textDstRec.y = 0;
  textDstRec.x = MM_SCREEN_WIDTH / 2;
  textDstRec.w = 0;
  textDstRec.h = 0;
  
  curDstRec.y = curIndex[0];
  curDstRec.x = (MM_SCREEN_WIDTH / 2) + 10;
  curDstRec.w = cursorImg->w;
  curDstRec.h = cursorImg->h;
  
  // Create Framerate manager to make image scroll at a plesant speed
  SDL_initFramerate(&fpsMan);
  SDL_setFramerate(&fpsMan, 20);
  
  // Loop to scroll return policy down screen
  RM_PlaySoundLoop(RM_SFX_FINAL_MUSIC);
  scrollAmount = 0;
  frmDelCur    = 0;
  while (loopFlag)
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
          loopFlag = 0;
        
        if(event->jbutton.button == CC_SELECT)
          MM_TakeMenuScreenshot();          
      }
    }
    
    // Hold top of image on screen for 3 seconds before scrolling
    if (scrollAmount == 0 && frmDelCur++ > 60)
    {
      scrollAmount = 1;
      frmDelCur    = 0;  // reset frame dealy counter
    }

    // scroll left side image vertically
    scroollSrcRec.y += scrollAmount;
    
    // If image has scrolled to bottom of screen do the following
    if ( (scrollImg->h - scroollSrcRec.y) < MM_SCREEN_HEIGHT)
    {
      // Position image at very bottom of screen
      scroollSrcRec.y = scrollImg->h - MM_SCREEN_HEIGHT;
      
      // start a counter that holds the current image for 3 seconds
      // before exiting this loop (or until user presses start)
      // NOTE: fpsMan limits loop to 20 frames per second.  After 40 loops
      // 3 seconds will have gone by
      if ( frmDelCur++ > 60 )
        loopFlag = 0;
    }
    
    // Exit this loop and move on to next set of images
    if (loopFlag == 0)
      scroollSrcRec.y = scrollImg->h - MM_SCREEN_HEIGHT;
  
    // Draw images to screen (left side is illustration, right side
    // is text for illustration
    SDL_BlitSurface(scrollImg, &scroollSrcRec, _scr, 0);
    SDL_BlitSurface(textImg, &textSrcRec, _scr, &textDstRec);
    SDL_framerateDelay(&fpsMan);
    sceDisplayWaitVblankStart();
    SDL_Flip(_scr);  
  }

  // Loop to display final level text to user, and allow user to select 
  // option for how to deal with store manager
  updateFlag = 1;  // re-draw screen when set to 1
  index      = 0;  // represents selection made by user
  txtFrame   = 1;  // frame 0 was displayed in previous loop
  loopFlag   = 1; 
  textSrcRec.y = txtFrmHeight * txtFrame;
  while (loopFlag)
  {
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
          if(event->jbutton.button == CC_START        || 
             event->jbutton.button == CC_TRIANGLE     ||
             event->jbutton.button == CC_CIRCLE       ||
             event->jbutton.button == CC_CROSS        || 
             event->jbutton.button == CC_SQUARE       || 
             event->jbutton.button == CC_LEFT_TRIGGER ||
             event->jbutton.button == CC_RIGHT_TRIGGER)
          {
            txtFrame++;
            if (txtFrame > 3) // exit loop after final frame is displayed
              loopFlag  = 0;
            else              // otherwise set flag to have new frame drawn
              updateFlag = 1;
          }
          
          if(event->jbutton.button == CC_SELECT)
            MM_TakeMenuScreenshot();

          
          // Only allow index value to change when on frame 3
          // Frame 3 displays 4 options the user can chose from
          if(txtFrame == 3 && event->jbutton.button == CC_UP)
          {
            RM_PlaySound(RM_SFX_MENU_SELECT);
            updateFlag = 1;
            index--;
          }
          if(txtFrame == 3 && event->jbutton.button == CC_DOWN)
          {
            RM_PlaySound(RM_SFX_MENU_SELECT);
            updateFlag = 1;
            index++;
          }
          break;
      }
    }
    
    // Draw frame is user pressed a button
    if (updateFlag)
    {
      // validate index ranges
      if (index >= MAX_OPTIONS)
        index = 0;
      if (index < 0)
        index = MAX_OPTIONS-1;
      
      curDstRec.y  = curIndex[index];
      updateFlag   = 0;
      textSrcRec.y = txtFrmHeight * txtFrame;
      
      SDL_BlitSurface(textImg, &textSrcRec, _scr, &textDstRec);
      SDL_BlitSurface(scrollImg, &scroollSrcRec, _scr, 0);
      // Draw cursor image only on 3rd text frame
      if (txtFrame == 3)
        SDL_BlitSurface(cursorImg, 0, _scr, &curDstRec);
      sceDisplayWaitVblankStart();
      SDL_Flip(_scr);  
    }
    SDL_Delay(100);
  }

  
  // just to be safe ensure proper bounds on selected value
  if (index >= MAX_OPTIONS)
     index = 0;
  if (index < 0)
    index = MAX_OPTIONS-1;
  
  RM_StopSound(-1);
  
  if (index == 3)
  {
    RM_PlaySound(RM_SFX_FINAL_WIN);
    gameState = MM_STATE_LEVEL_COMPLETE;
  }
  else
  {              
    RM_PlaySound(RM_SFX_GAME_OVER);
    gameState = MM_STATE_GAME_OVER;
  }

  // set source rec to display the image selected by user (tracked by index)
  textSrcRec.x = 0;
  textSrcRec.y = MM_SCREEN_HEIGHT * index;
  textSrcRec.w = MM_SCREEN_WIDTH;
  textSrcRec.h = MM_SCREEN_HEIGHT;
  
  SDL_BlitSurface(finalImg, &textSrcRec, _scr, 0);
  SDL_Flip(_scr);  
  
  
  loopFlag = 1;
  while (loopFlag)
  {
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
          if(event->jbutton.button == CC_START        || 
             event->jbutton.button == CC_TRIANGLE     ||
             event->jbutton.button == CC_CIRCLE       ||
             event->jbutton.button == CC_CROSS        || 
             event->jbutton.button == CC_SQUARE)
            loopFlag = 0;
          
          if(event->jbutton.button == CC_SELECT)
            MM_TakeMenuScreenshot();
          break;
        default:
          break;  
      } // END switch (event->type) 
    }   // END while (SDL_PollEvent(event)) 
    SDL_Delay(100);  
  }     // END while (loopFlag)
    
  // Stop all sounds!
  RM_StopSound(-1);  

  return(gameState);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawHiddenLevel1
// Summary:  Draws and processes user input for the Hidden level
// Inputs:   SDL_Event pointer
// Outputs:  None
// Returns:  Game state
// Cautions: None
//------------------------------------------------------------------------------
unsigned int MENU_DrawHiddenLevel1(SDL_Event *event)
{
  #define NUM_CONCEPTS 6
  char *info[NUM_CONCEPTS][2];
  int loop     = 1;
  int newImage = 1;
  int infoFlag = 1;
  int index    = 0;
  SDL_Surface *img     = 0;
  SDL_Surface *infoImg = 0;
  SDL_Surface *txtImg  = 0; 
  SDL_Surface *nameImg = 0;
  ZIP_Font *f1;
  SDL_Color fgColor   = {255,0,0};
  SDL_Rect txtRecDst  = {0,0,0,0};
  SDL_Rect nameRecDst = {0,0,0,0};
  
  info[0][0] = "concept0.bmp";
  info[0][1] = "Introduction";
  info[1][0] = "concept1.bmp";
  info[1][1] = "Hero Concepts By Alistair Stuart";
  info[2][0] = "concept2.bmp";
  info[2][1] = "Additional Hero Concepts By Alistair Stuart";
  info[3][0] = "concept3.bmp";
  info[3][1] = "Various Hero Concepts";
  info[4][0] = "concept4.bmp";
  info[4][1] = "Hero Concepts by Michael Glasswell";
  info[5][0] = "concept5.bmp";
  info[5][1] = "Additional Hero Concepts by Michael Glasswell";
  
  ZIP_OpenZipFile(ZIP_MAIN);
  f1 = ZIP_LoadFont(ZIP_FONT1, 12);
  infoImg = SDL_CreateRGBSurface(SDL_SWSURFACE, MM_SCREEN_WIDTH, 38, 
            _scr->format->BitsPerPixel,  _scr->format->Rmask, //_scr->pitch,
            _scr->format->Gmask, _scr->format->Bmask, _scr->format->Amask);
  SDL_FillRect(infoImg, 0, 0);
  SDL_SetAlpha(infoImg, SDL_SRCALPHA, 225);
  txtImg = TTF_RenderText_Blended(f1->f, "Left Trigger=Prev    Start=Main Menu    Select=Toggle Info    Right Trigger=Next", fgColor);

  while (loop)
  {
    if(newImage)
    {
      if (img)
        SDL_FreeSurface(img);
      if (nameImg)
        SDL_FreeSurface(nameImg); 
        
      if (index < 0)
        index = NUM_CONCEPTS-1;
      else if (index >= NUM_CONCEPTS)
        index = 0;
      newImage     = 0;
      img          = ZIP_LoadImage(info[index][0]);  
      nameImg      = TTF_RenderText_Blended(f1->f, info[index][1], fgColor);
      txtRecDst.x  = (MM_SCREEN_WIDTH - txtImg->w) / 2;
      nameRecDst.x = (MM_SCREEN_WIDTH - nameImg->w) / 2;
      nameRecDst.y = txtImg->h-5;
    }

    SDL_BlitSurface(img, 0, _scr, 0);
    if (infoFlag)
    {
      SDL_BlitSurface(infoImg, 0, _scr, 0);
      SDL_BlitSurface(txtImg,  0, _scr, &txtRecDst);
      SDL_BlitSurface(nameImg, 0, _scr, &nameRecDst);
    }
    sceDisplayWaitVblankStart();
    SDL_Flip(_scr);
    
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
        if((event->jbutton.button == CC_START        || 
            event->jbutton.button == CC_TRIANGLE     ||
            event->jbutton.button == CC_CIRCLE       ||
            event->jbutton.button == CC_CROSS        || 
            event->jbutton.button == CC_SQUARE))
        {
          loop = 0;
        }  
     
        if(event->jbutton.button == CC_LEFT_TRIGGER)
        {
          index--;
          newImage = 1;
        }  
        if(event->jbutton.button == CC_RIGHT_TRIGGER)
        {
          newImage = 1;
          index++;
        }
        if(event->jbutton.button == CC_SELECT)
        {
          infoFlag = (infoFlag == 1)?0:1;
        }
        break;
      }
    }
  }

  ZIP_CloseZipFile();
  ZIP_CloseFont(f1);
  SDL_FreeSurface(img);
  SDL_FreeSurface(txtImg);
  SDL_FreeSurface(nameImg); 
  SDL_FreeSurface(infoImg); 
  
  return(MM_STATE_LEVEL_COMPLETE);
}

//------------------------------------------------------------------------------
// Name:     MENU_DrawCredits
// Summary:  Draws the intro to the game and processes user input
// Inputs:   SDL_Event pointer
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void MENU_DrawIntro(SDL_Event *event)
{
  #define NUM_FRAMES 12
  #define CLERK       1
  #define HERO        2
  void        *scrPixelPtr;
  void        *dataPtr;
  ZIP_Font    *f1;
  Mix_Chunk   *music;
  SDL_Surface *tmp;
  SDL_Surface *bgImg;
  SDL_Surface *transImg;
  SDL_Surface *clerkImg;
  SDL_Surface *heroImg;
  SDL_Surface *heroHeadImg;
  SDL_Surface *clerkHeadImg;
  SDL_Surface *dialogImg[3];
  
  SDL_Rect    dialogDstRec;
  SDL_Rect    heroSrcRec;
  SDL_Rect    heroDstRec;
  SDL_Rect    clerkSrcRec;
  SDL_Rect    clerkDstRec;
  SDL_Color fgColor  = {255,255,255};
  int x;
  int channel        = -1;
  int numDialogFrms  = 12;
  int curFrm         = 0;
  int updateIntro    = 1;
  int loopFlag       = 1;
  int alpha          = 0;
  int dialogYOffset  = 0;   
  unsigned int bSize = 512 * MM_SCREEN_HEIGHT * 2;  
  char *dialog[NUM_FRAMES][4];
  
  dialog[0][0] = "        Your journey begins at the checkout counter in the Sporting Goods";
  dialog[0][1] = "                         department.  Remember, trust no-one!";
  dialog[0][2] = "            L. Trigger = Previous      Start=Skip Intro    R. Trigger = Next";
  dialog[0][3] = 0;

  dialog[1][0] = "Hey Kid, what can I do for ya?";
  dialog[1][1] = 0;
  dialog[1][2] = 0;
  dialog[1][3] = CLERK;

  dialog[2][0] = "Hello Mr... Jones.  My name is Logan Bogan.  My Uncle Chuck";
  dialog[2][1] = "gave me this karabiner for Christmahanaquanzaka.  It's certified";
  dialog[2][2] = "to hold 2000 lbs. but it broke after holding only 1999 lbs.";
  dialog[2][3] = HERO;

  dialog[3][0] = "So?";
  dialog[3][1] = 0;
  dialog[3][2] = 0;
  dialog[3][3] = CLERK;

  dialog[4][0] = "So... it came from Mega-Mart and I want a refund.  Pip! Pip!";
  dialog[4][1] = 0;
  dialog[4][2] = 0;
  dialog[4][3] = HERO;

  dialog[5][0] = "Sorry Kid, but you'll need to take that thing to Customer";
  dialog[5][1] = "Service, waaaaay over at the other end of the store. We can't";
  dialog[5][2] = "return junk here.";
  dialog[5][3] = CLERK;

  dialog[6][0] = "What???  But that's like... way over there!";
  dialog[6][1] = 0;
  dialog[6][2] = 0;
  dialog[6][3] = HERO;

  dialog[7][0] = "If you want my advice, just buy another one.  Nobody gets their";
  dialog[7][1] = "money back from the Mega-Mart, nobody.";
  dialog[7][2] = 0;
  dialog[7][3] = CLERK;

  dialog[8][0] = "I'll take my chances.";
  dialog[8][1] = 0;
  dialog[8][2] = 0;
  dialog[8][3] = HERO;

  dialog[9][0] = "Hey kid, take this, you're gunna need it...";
  dialog[9][1] = 0;
  dialog[9][2] = 0;
  dialog[9][3] = CLERK;

  dialog[10][0] = "Uh, thanks Mister...";
  dialog[10][1] = 0;
  dialog[10][2] = 0;
  dialog[10][3] = HERO;

  dialog[11][0] = "Oh and kid, good luck. ";
  dialog[11][1] = 0;
  dialog[11][2] = 0;
  dialog[11][3] = CLERK;

  // Load images from ZIP file
  ZIP_OpenZipFile(ZIP_MAIN);
  f1 = ZIP_LoadFont(ZIP_FONT1, 12);
  
  tmp   = ZIP_LoadImage("intro_bg.bmp");
  bgImg = SDL_ConvertSurface(tmp, _scr->format, SDL_SWSURFACE);
  SDL_FreeSurface(tmp);
  
  tmp = ZIP_LoadImage("intro_clerk.bmp");
  clerkImg = SDL_ConvertSurface(tmp, _scr->format, SDL_SWSURFACE);
  SDL_FreeSurface(tmp);
  SDL_SetColorKey(clerkImg, SDL_SRCCOLORKEY, 
                  SDL_MapRGB(clerkImg->format, 0xFF, 0x80, 0x80));
  music        = ZIP_LoadMusic("intro_music.wav");
  heroHeadImg  = ZIP_LoadImage("intro_hero_head.png");
  clerkHeadImg = ZIP_LoadImage("intro_clerk_head.png");
  heroImg      = ZIP_LoadImage("intro_hero_arms.png");
  ZIP_CloseZipFile();
  
  // Make font style BOLD
  TTF_SetFontStyle(f1->f, TTF_STYLE_BOLD);
  
  // Semi-Transparent image to draw dialog over
  transImg = SDL_CreateRGBSurface(SDL_SWSURFACE, MM_SCREEN_WIDTH, 56, 
              _scr->format->BitsPerPixel, _scr->format->Rmask, 
              _scr->format->Gmask, _scr->format->Bmask, _scr->format->Amask);
  SDL_FillRect(transImg, 0, 0);
  SDL_SetAlpha(transImg, SDL_SRCALPHA, 200);
  
  // Set loacation on screen where each image will appear
  clerkSrcRec.x = 0;
  clerkSrcRec.y = 0;
  clerkSrcRec.w = clerkImg->w;
  clerkSrcRec.h = clerkImg->h / 3;
  
  clerkDstRec.x = 65;  
  clerkDstRec.y = 95;  
  
  heroSrcRec.x = 0;
  heroSrcRec.y = 0;
  heroSrcRec.w = heroImg->w;
  heroSrcRec.h = heroImg->h / 3;
  
  heroDstRec.x = 240; 
  heroDstRec.y = 174; 
    
  dialogDstRec.x = heroHeadImg->w + 5;
  dialogDstRec.y = dialogYOffset;
  
  // Initialize dialog images to null
  dialogImg[0] = dialogImg[1] = dialogImg[2] = 0;
  
  while (loopFlag)
  {
    // Poll for user input
    while (SDL_PollEvent(event)) 
    {
      // Process user input
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
        if(event->jbutton.button == CC_LEFT_TRIGGER)   // previous frame
        {
          curFrm--;
          updateIntro = 1;
        }
        if (event->jbutton.button == CC_RIGHT_TRIGGER) // next frame
        {
          curFrm++;
          updateIntro = 1;
        }
        if( event->jbutton.button == CC_START        || 
            event->jbutton.button == CC_TRIANGLE     ||
            event->jbutton.button == CC_CIRCLE       ||
            event->jbutton.button == CC_CROSS        || 
            event->jbutton.button == CC_SQUARE )
        {
          loopFlag = 0;
        }
        if(event->jbutton.button == CC_SELECT)
        {
          MM_TakeMenuScreenshot();
        }
        break;
      }
    }
    
    // Update on screen display if user pressed a key
    if (updateIntro)
    {
      // end intro after last frame
      if (curFrm >= numDialogFrms)
      {
        loopFlag = 0;
        break; 
      }
      else if ( curFrm < 0 ) // User scrolled back to far
      {
        curFrm = 0; // force 1st frame to display
      }
      
      // Update clerk, and hero images/poses
      if (curFrm == 2 || curFrm == 3 || curFrm == 4) 
      {
        heroSrcRec.y  = (1 * heroSrcRec.h);   // hero holding karabiner
        clerkSrcRec.y = 0;                    // normal clerk pose
      }
      else if (curFrm == 5)
      {
        heroSrcRec.y  = (1 * heroSrcRec.h);   // hero holding karabiner
        clerkSrcRec.y = (1 * clerkSrcRec.h);  // clerk pointing east
      }
      else if (curFrm == 9)
      {
        heroSrcRec.y  = 0;                    // hero arm at side
        clerkSrcRec.y = (2 * clerkSrcRec.h);  // clerk holding bat
      }
      else if (curFrm == 10 || curFrm == 11 )
      {
        heroSrcRec.y  = (2 * heroSrcRec.h);   // hero holding bat
        clerkSrcRec.y = 0;                    // normal clerk pose
      }
      else
      {
        heroSrcRec.y  = 0;                    // Hero arm at side
        clerkSrcRec.y = 0;                    // Normal clerk pose
      }
      
      // There are allways 3 lines of text
      // Line will be null if no text is present
      for (x=0; x < 3; x++)
      {
        // free old text image
        if (dialogImg[x]) 
          SDL_FreeSurface(dialogImg[x]);
        
        // Create new text image, or set to null if none exists
        if (dialog[curFrm][x])
          dialogImg[x] = TTF_RenderText_Blended(f1->f, 
                          dialog[curFrm][x], fgColor);
        else
          dialogImg[x] = 0;
      }
      
      // Adjust text location on screen based on frame
      if (curFrm == 0)
        dialogDstRec.x = 0;
      else 
        dialogDstRec.x = heroHeadImg->w + 5;
      dialogDstRec.y   = dialogYOffset;
      
      // Draw images
      SDL_BlitSurface(bgImg,    0, _scr, 0);  // floor, desk celining
      SDL_BlitSurface(transImg, 0, _scr, 0);  // semi-transp. dialog box
      
      // draw text for each line of dialog
      for (x=0; x < 3; x++)
      {
        if (dialogImg[x])
        {  
          SDL_BlitSurface(dialogImg[x], 0, _scr, &dialogDstRec);
          dialogDstRec.y += dialogImg[x]->h - 5;
        }
      }
      
      if (channel < 0)
        channel = Mix_PlayChannel(-1, music, -1);
      
      // Draw clerk or hero head image
      if (dialog[curFrm][3] == CLERK)
        SDL_BlitSurface(clerkHeadImg, 0, _scr, 0);
      else if (dialog[curFrm][3] == HERO)
        SDL_BlitSurface(heroHeadImg, 0, _scr, 0);
      
      SDL_BlitSurface(heroImg,   &heroSrcRec,   _scr, &heroDstRec);
      SDL_BlitSurface(clerkImg,  &clerkSrcRec,  _scr, &clerkDstRec);
      sceDisplayWaitVblankStart();
      SDL_Flip(_scr);      
    }
    else
    {
      // sleep when user is not giving any commands
      SDL_Delay(100);
    }
  }
  
  // Free memory from images
  ZIP_CloseFont(f1);
  SDL_FreeSurface(bgImg);
  SDL_FreeSurface(transImg);
  SDL_FreeSurface(clerkImg);
  SDL_FreeSurface(heroImg);
  SDL_FreeSurface(clerkHeadImg);
  SDL_FreeSurface(heroHeadImg);
  for (x=0; x < 3; x++) //  Free dialog frames
  {
    if (dialogImg[x])
      SDL_FreeSurface(dialogImg[x]);
  }
  
  // Get whats on screen, and copy it to a new SDL image
  scrPixelPtr = MM_GetScreenBuffer(MM_DRAW_BUFFER);
  dataPtr     = malloc(bSize);  // allocate buffer to hold on screen image
  if (dataPtr)
  {
    memcpy(dataPtr, scrPixelPtr, bSize); // copy on screen image to buffer
    bgImg = SDL_CreateRGBSurfaceFrom(dataPtr, 
               MM_SCREEN_WIDTH, MM_SCREEN_HEIGHT, 
               _scr->format->BitsPerPixel, _scr->pitch, _scr->format->Rmask, 
               _scr->format->Gmask, _scr->format->Bmask, _scr->format->Amask);
    if (bgImg)
    {
      // Fade image to black and draw to screen
      for (alpha = SDL_ALPHA_OPAQUE-1; alpha > 0; alpha-=10)
      {
        if (alpha < 0)
           alpha = 0;
        SDL_FillRect(_scr, 0, 0);                  // draw black background
        SDL_SetAlpha(bgImg, SDL_SRCALPHA, alpha);  // make image fade
        SDL_BlitSurface(bgImg, 0, _scr, 0);        // draw faded image
        sceDisplayWaitVblankStart();
        SDL_Flip(_scr);      
      }
      
      // Free sureface and memory allocated to hold pixels
      SDL_FreeSurface(bgImg);
      free(dataPtr);
    }
    else
    {
      free(dataPtr);
    }
  }
  Mix_HaltChannel(channel);
  Mix_FreeChunk(music);
}


//------------------------------------------------------------------------------
// Name:     MENU_DrawCredits
// Summary:  Draws the credits (we came a looong way to get here).
// Inputs:   SDL_Event pointer
// Outputs:  None
// Returns:  Game state
// Cautions: Several SDL surfaces and a TTF are opened via the ZIP_Manager
//           to create the secret code.  These resources must be freed before
//           this function exist.  Also, Codes must be maintained here and
//           in MENU_DrawEnterCode
//------------------------------------------------------------------------------
unsigned int MENU_DrawCredits(SDL_Event *event)
{
  #define     NUM_CODES      5
  SDL_Surface *creditsImg;
  SDL_Surface *codeImg;
  SDL_Surface *codeDescImg;
  ZIP_Font    *f1;
  ZIP_Font    *f2;
  FPSmanager  fpsMan;
  SDL_Color   color            = {255,255,255};
  SDL_Rect    srcRec           = {0,0,0,0};
  SDL_Rect    dstRec           = {0,0,0,0};
  int         codes[NUM_CODES] = {1,2,3,4,5}; // Decoys sucka!
  int         scrollCredits    = 1;
  int         scrollAmount     = 1;
  int         channel          = -1;
  int         code;
  char        strCode[10];
  char        *desc[NUM_CODES] = { "Invincibility", "Skip To Final Level", 
                                   "Skip To Credits", "Hidden Level",  
                                   "Infinite Spare Lives" };
  
  // Load resources via Resource Manager
  creditsImg  = RM_GetImage(RM_IMG_CREDITS);
  
  // Load font used to create Secret Code 
  // This is done here dynamically at run time to keep user from easily
  // discovering all the secret codes
  ZIP_OpenZipFile(ZIP_MAIN);
  f1    = ZIP_LoadFont(ZIP_FONT1, 30);
  f2    = ZIP_LoadFont(ZIP_FONT1, 15);
  ZIP_CloseZipFile();
  TTF_SetFontStyle(f1->f, TTF_STYLE_BOLD);
  TTF_SetFontStyle(f2->f, TTF_STYLE_BOLD);
  
  // Select secret code and create SDL surfaces used to diaply it to user
  code = MM_RandomNumberGen(0, NUM_CODES-1);
  sprintf(strCode, "%06i", codes[code]);  // convert int to 7 byte string
  codeImg     = TTF_RenderText_Blended(f1->f, strCode, color);
  codeDescImg = TTF_RenderText_Blended(f2->f, desc[code], color);
  
  dstRec.x = 0;
  dstRec.y = creditsImg->h - 152;
  dstRec.w = MM_SCREEN_WIDTH;
  dstRec.h = 152;
  
  SDL_FillRect(creditsImg, &dstRec, 0);
  
  // Draw code onto credits image at the proper location
  dstRec.x = (MM_SCREEN_WIDTH/2) - (codeImg->w/2);
  dstRec.y = creditsImg->h - 115;
  SDL_BlitSurface(codeImg, 0, creditsImg, &dstRec);
  
  // Draw code description onto credits image
  dstRec.x  = (MM_SCREEN_WIDTH/2) - (codeDescImg->w/2);
  dstRec.y += codeImg->h - 10;
  SDL_BlitSurface(codeDescImg, 0, creditsImg, &dstRec);

  // reset source and destinations recs for use in scrolling credits
  srcRec.x = srcRec.y = srcRec.w = srcRec.h = 0;
  dstRec.x = dstRec.y = dstRec.w = dstRec.h = 0;
  
  // Create Framerate manager
  SDL_initFramerate(&fpsMan);
  SDL_setFramerate(&fpsMan, 25);

  srcRec.w = creditsImg->w;
  srcRec.h = MM_SCREEN_HEIGHT;
     
  SDL_BlitSurface(creditsImg, &srcRec, _scr, &dstRec);
  sceDisplayWaitVblankStart();
  SDL_Flip(_scr);  
  
  // Check to see if credits music loaded properly. For some odd reason it
  // will not load correctly at times
  if (RM_GetSoundFx(RM_SFX_CREDITS_MUSIC))
  {
    // Start music now that all is loaded
    channel = RM_PlaySoundLoop(RM_SFX_CREDITS_MUSIC); 
  }
  else
  {
    // If credits music did not load, just play the final level theme, 
    // at least it seems to consitently load!
    channel = RM_PlaySoundLoop(RM_SFX_FINAL_MUSIC); 
  }
  
  SDL_Delay(2000);
  
  while (scrollCredits)
  {
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
        // allow user to exit when credits complete
        if( event->jbutton.button == CC_START        || 
            event->jbutton.button == CC_TRIANGLE     ||
            event->jbutton.button == CC_CIRCLE       ||
            event->jbutton.button == CC_CROSS        || 
            event->jbutton.button == CC_SQUARE )
        {
          scrollCredits = 0;
        }  
      }
    }
    
    srcRec.y += scrollAmount;
    if ( (creditsImg->h - srcRec.y) < MM_SCREEN_HEIGHT)
    {
      scrollAmount = 0;
      srcRec.y     = creditsImg->h - MM_SCREEN_HEIGHT;
    }
    
    SDL_BlitSurface(creditsImg, &srcRec, _scr, &dstRec);
    SDL_framerateDelay(&fpsMan);
    sceDisplayWaitVblankStart();
    SDL_Flip(_scr);  
  }
  
  // Free resources used to create secret code
  ZIP_CloseFont(f1);
  ZIP_CloseFont(f2);
  SDL_FreeSurface(codeImg);
  SDL_FreeSurface(codeDescImg);
  
  // stop mucis
  RM_StopSound(channel); 
  
  // Returnnew game state
  return(MM_STATE_LEVEL_COMPLETE);
}      

