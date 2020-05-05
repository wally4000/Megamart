//-----------------------------------------------------------------------------
//  Class:
//  Resource Manager
//
//  Description:
//  This class manages all resources (sound effects & images) used by the 3 
//  levels in the game (Level 1, Final Level, and the Credits).  This class
//  will automatically allocate and free the memory used by each level's 
//  resources so the user does not have to worry about doing so.  
//  
//  Note: Most (but not all) of the functions in the Menu Manager class 
//  DO NOT use this class.  As such, those functions are responsible for 
//  allocating and freeing the memory for any resource they use.
//-----------------------------------------------------------------------------

#include "resource_manager.h"
#include "zip_manager.h"
#include "sce_graphics.h"

typedef struct LoadResStruct
{
  char           name[30]; 
  unsigned short id;
  unsigned int   level;
  unsigned short format;
  unsigned int   flags;
} LoadResStruct;

#define TRANSP_FORMAT   2
#define SCREEN_FORMAT   1
#define NATIVE_FORMAT   0

static Mix_Chunk    *_sounds[NUM_SOUNDS];
static SDL_Surface  *_images[NUM_IMAGES];
static int          _permImagesLoaded;
static int          _lastLevelLoaded;

// Trying to keep each line under 80 characters is a lost cause in this file
static void        AddSound(LoadResStruct *info, const char *name, unsigned short id, unsigned int level);
static void        AddImage(LoadResStruct *info, const char *name, unsigned short id, unsigned int level, unsigned short format, unsigned int flags);
static SDL_Surface *LoadImage(LoadResStruct *ptr);
static void        LoadPermanantImages();

//------------------------------------------------------------------------------
// Name:     RM_Init
// Summary:  Called 1X, initializes Resource Manager for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void RM_Init()
{
  int x      = 0;

  for (x=0; x < NUM_IMAGES; x++)
    _images[x] = 0;
    
  for (x=0; x < NUM_SOUNDS; x++)
    _sounds[x] = 0;
  
  // set flag to denoting perminant images have not been loaded
  _permImagesLoaded = 0;
  _lastLevelLoaded  = 0;  
} 

//------------------------------------------------------------------------------
// Name:     RM_InitLevel
// Summary:  Allocates the resources needed for the specified level.  Frees 
//           any allocated resources that are not needed by the current level.
// Inputs:   Level to initialize
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int RM_InitLevel(unsigned int level)
{
  int status = 0;
  int x      = 0;
  int id     = 0;
  LoadResStruct images[NUM_IMAGES];
  LoadResStruct sounds[NUM_SOUNDS];
  _lastLevelLoaded = level;
  
  
  for (x=0; x < NUM_IMAGES; x++)
    AddImage(&images[x], "0", 0, 0, 0, 0);
    
  for (x=0; x < NUM_SOUNDS; x++)
    AddSound(&sounds[x], "0", 0, 0);

  // Only load the permanant images 1 time
  if ( _permImagesLoaded == 0)
  {
    LoadPermanantImages();
    _permImagesLoaded = 1;
  }

  x=0;
  AddImage(&images[x++], "bonus_life.png",  RM_IMG_BONUS_LIFE_SPRITE,  MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "power_up.bmp",    RM_IMG_POWER_UP_SPRITE,    MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "twinkle.bmp",     RM_IMG_TWINKLE_SPRITE,     MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "basketball.bmp",  BASKETBALL_SPRITE,         MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "baseball.bmp",    BASEBALL_SPRITE,           MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "soccerball.bmp",  SOCCERBALL_SPRITE,         MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "grill.png",       GRILL_SPRITE,              MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "employee.bmp",    EMPLOYEE_SPRITE,           MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "tentguy.png",     TENT_GUY_SPRITE,           MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "punchingbag.png", PUNCHING_BAG_SPRITE,       MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "bomb.bmp",        RM_IMG_BOMB_SPRITE,        MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "archer.bmp",      RM_IMG_ARCHER_SPRITE,      MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "arrow.bmp",       RM_IMG_ARROW_SPRITE,       MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "bicycle1.png",    RM_IMG_BICYCLE1_SPRITE,    MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "bicycle2.png",    RM_IMG_BICYCLE2_SPRITE,    MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hockey_east.bmp", RM_IMG_HOCKEY_EAST_SPRITE, MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "hockey_west.bmp", RM_IMG_HOCKEY_WEST_SPRITE, MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "sign.png",        RM_IMG_SIGN_SPRITE,        MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "weights.bmp",     RM_IMG_WEIGHTS_SPRITE,     MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  
  AddImage(&images[x++], "bowling_shelf.bmp",     RM_BOWLING_SHELF_SPRITE,       MM_LEVEL1, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "bowlingball_red.png",   RM_RED_BOWLING_BALL_SPRITE,    MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "bowlingball_green.png", RM_GREEN_BOWLING_BALL_SPRITE,  MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "bowlingball_yellow.png",RM_YELLOW_BOWLING_BALL_SPRITE, MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "bowlingball_blue.png",  RM_BLUE_BOWLING_BALL_SPRITE,   MM_LEVEL1, NATIVE_FORMAT, 0);
                                                                    
  AddImage(&images[x++], "tent_green.bmp",    TENT_GREEN,           MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "tent_blue.bmp",     TENT_BLUE,            MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "tent_red.bmp",      TENT_RED,             MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "tent_grey.bmp",     TENT_GREY,            MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "tent_purple.bmp",   TENT_PURPLE,          MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "tent_orange.bmp",   TENT_ORANGE,          MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  
  AddImage(&images[x++], "tent3.bmp",         L1S1_TENT3,           MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "arnold.bmp",        ARNOLD_SPRITE,        MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  
  AddImage(&images[x++], "shelfa.bmp",        GEN_RANDOM_SHELF,     MM_LEVEL1, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "shelfb.bmp",        GEN_SHELF_B_SPRITE,   MM_LEVEL1, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "shelfc.bmp",        GEN_SHELF_C_SPRITE,   MM_LEVEL1, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "shelfd.bmp",        GEN_SHELF_D_SPRITE,   MM_LEVEL1, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "shelf_fall.bmp",    FALLING_SHELF_SPRITE, MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  
  AddImage(&images[x++], "power_meter.png",   RM_IMG_HERO_POWER_METER,  MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "extra_life.png",    RM_IMG_HERO_EXTRA_LIFE,   MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hero.png",          RM_IMG_HERO,              MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hero_weapon.png",   RM_IMG_HERO_WEAPON,       MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hero_hit.png",      RM_IMG_HERO_HURT,         MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hero_death.png",    RM_IMG_HERO_DEATH,        MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hero_duck.png",     RM_IMG_HERO_DUCK,         MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hero_jump.png",     RM_IMG_HERO_JUMP,         MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "hero_victory.png",  RM_IMG_HERO_VICTORY,      MM_LEVEL1, NATIVE_FORMAT, 0);
  
  AddImage(&images[x++], "prog_icon.bmp",        RM_IMG_PROGRESS_ICON,     MM_LEVEL1, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "prog_meter.png",       RM_IMG_PROGRESS_BAR,      MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "l1_end_text.png",      RM_LEVEL1_COMPLETE_TXT,   MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "game_paused.png",      RM_GAME_PAUSED_TXT,       MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "game_over.png",        RM_GAME_OVER_TXT,         MM_LEVEL1|MM_LEVEL_FINAL, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "savingscreenshot.png", RM_SCREENSHOT_1_TXT,      MM_LEVEL1|MM_LEVEL_FINAL, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "screenshotsaved.png",  RM_SCREENSHOT_2_TXT,      MM_LEVEL1|MM_LEVEL_FINAL, SCREEN_FORMAT|TRANSP_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "resume_quit.png",      RM_RESUME_QUIT_TXT,       MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "cursor.png",           RM_CURSOR,                MM_LEVEL1, NATIVE_FORMAT, 0);
  AddImage(&images[x++], "credits.bmp",          RM_IMG_CREDITS,           MM_LEVEL_CREDITS, NATIVE_FORMAT, 0);
  
  
  // Final Level Images
  AddImage(&images[x++], "final_image.bmp",        RM_IMG_FL_IMAGE,          MM_LEVEL_FINAL, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "final_level.bmp",        RM_IMG_FL_SCROLL,         MM_LEVEL_FINAL, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "final_level_txt.bmp",    RM_IMG_FL_TEXT,           MM_LEVEL_FINAL, SCREEN_FORMAT, SDL_SWSURFACE);
  AddImage(&images[x++], "cursor_small.png",       RM_IMG_FL_CURSOR,         MM_LEVEL_FINAL, NATIVE_FORMAT, 0);
  
  x=0;
  AddSound(&sounds[x++], "hero_hit.wav",          RM_SFX_HERO_HIT,          MM_LEVEL1);
  AddSound(&sounds[x++], "hero_jump.wav",         RM_SFX_HERO_JUMP,         MM_LEVEL1);
  AddSound(&sounds[x++], "hit_ball.wav",          RM_SFX_BALL_HIT,          MM_LEVEL1);
  AddSound(&sounds[x++], "grill_falling.wav",     RM_SFX_GRILL_FALLING,     MM_LEVEL1);
  AddSound(&sounds[x++], "bowling_ball_fall.wav", RM_SFX_BOWLING_BALL_FALL, MM_LEVEL1);
  AddSound(&sounds[x++], "level1.wav",            RM_SFX_LEVEL1_MUSIC,      MM_LEVEL1);
  AddSound(&sounds[x++], "power_up.wav",          RM_SFX_POWER_UP,          MM_LEVEL1);
  AddSound(&sounds[x++], "shelf_fall.wav",        RM_SFX_SHELF_FALL,        MM_LEVEL1);
  AddSound(&sounds[x++], "tent_hit.wav",          RM_SFX_TENT_HIT,          MM_LEVEL1);
  AddSound(&sounds[x++], "screenshot.wav",        RM_SFX_SCREENSHOT,        MM_LEVEL1);
  AddSound(&sounds[x++], "employee_hit.wav",      RM_SFX_EMPLOYEE_HIT,      MM_LEVEL1);
  AddSound(&sounds[x++], "bomb.wav",              RM_SFX_EXPLOSION,         MM_LEVEL1);
  AddSound(&sounds[x++], "arrow.wav",             RM_SFX_ARROW,             MM_LEVEL1);
  AddSound(&sounds[x++], "bicycle.wav",           RM_SFX_BICYCLE,           MM_LEVEL1);
  AddSound(&sounds[x++], "bicycle_bell.wav",      RM_SFX_BICYCLE_BELL,      MM_LEVEL1);
  AddSound(&sounds[x++], "game_over.wav",         RM_SFX_GAME_OVER,         MM_LEVEL1|MM_LEVEL_FINAL);

  AddSound(&sounds[x++], "menu_select.wav",       RM_SFX_MENU_SELECT,       MM_LEVEL_FINAL);
  AddSound(&sounds[x++], "final_level.wav",       RM_SFX_FINAL_MUSIC,       MM_LEVEL_CREDITS|MM_LEVEL_FINAL);
  AddSound(&sounds[x++], "final_winner.wav",      RM_SFX_FINAL_WIN,         MM_LEVEL_FINAL);
  AddSound(&sounds[x++], "creditsm.wav",          RM_SFX_CREDITS_MUSIC,     MM_LEVEL_CREDITS);
  
  for (x=0; x < NUM_IMAGES - NUM_PERM_IMAGES; x++)
  {
    id = images[x].id;
    if ( id != 0 && ((images[x].level & level) == 0) && _images[id] != 0)
    {
      SDL_FreeSurface(_images[id]);
      _images[id] = 0;
    }
  }
  
  // Delete sounds not used in this level
  for (x=0; x < NUM_SOUNDS; x++)
  {
    id = sounds[x].id;
    if ( id != 0 && ((sounds[x].level & level) == 0) && _sounds[id] != 0)
    {
      Mix_FreeChunk(_sounds[id]);
      _sounds[id] = 0;
    }
  }

  // load images for this level that have not allready been loaded
  for (x=0; x < NUM_IMAGES - NUM_PERM_IMAGES; x++)
  {
    id = images[x].id;
    if ( (images[x].level & level) && _images[id] == 0)
      _images[id] = LoadImage(&images[x]);
  }

  // load sounds for this level that have not allready been loaded
  for (x=0; x < NUM_SOUNDS; x++)
  {
    id = sounds[x].id;
    if ( (sounds[x].level & level) && _sounds[id] == 0)
      _sounds[id] = ZIP_LoadMusic(sounds[x].name);
  } 
 
  return(status);
}

//------------------------------------------------------------------------------
// Name:     LoadPermanantImages
// Summary:  Loads the images that will remain in RAM for the duration of the
//           game.  Images drawn using SCE functions must be alligned to 16 
//           bytes in memory.  Since for some odd reason the PSP SDK will not 
//           free 16 byte alligned memory, we load the data 1X and let in 
//           remain in memory for the duration of the game.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void LoadPermanantImages()
{
  // Image contains the Walk Ceiling image and the Loop image
  // this image will be loaded into VRAM
  _images[RM_IMG_BG1] = SCE_LoadBackground1("bg1.bmp");
  
  // image contains the Run Ceiling and Walk Floor
  _images[RM_IMG_BG2] = SCE_LoadBackground2("bg2.bmp");
  
  // Run Floor Images (in 7 different files), load them into VRAM
  _images[RM_IMG_RF1] = SCE_LoadBackground1("rf1.bmp");
  _images[RM_IMG_RF2] = SCE_LoadBackground1("rf2.bmp");
  _images[RM_IMG_RF3] = SCE_LoadBackground1("rf3.bmp");
  _images[RM_IMG_RF4] = SCE_LoadBackground1("rf4.bmp");
  _images[RM_IMG_RF5] = SCE_LoadBackground1("rf5.bmp");
  _images[RM_IMG_RF6] = SCE_LoadBackground1("rf6.bmp");
  _images[RM_IMG_RF7] = SCE_LoadBackground1("rf7.bmp");
}

//------------------------------------------------------------------------------
// Name:     AddImage
// Summary:  Wrapper function used to populate a LoadRes Structure, which is
//           the structure used to determine if a resource should be loaded 
//           for the current level.
// Inputs:   1. info - Resource Structure to store information into
//           2. name - string name of file to load
//           3. id - ID of data (as specified in Resource Manager Header File)
//           4. level - Ored list of levels this resource should be loaded for
//           5. format - Format of image (SDL NATIVE or SCREEN format)
//           6. flags - Flags to use if surface needs to be converted
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void AddImage(LoadResStruct *info, const char *name,      unsigned short id,
              unsigned int level,  unsigned short format, unsigned int flags)
{
  strcpy(info->name, name);
  info->id     = id;
  info->level  = level;
  info->format = format;
  info->flags  = flags;
}

//------------------------------------------------------------------------------
// Name:     AddSound
// Summary:  Wrapper function used to populate a LoadRes Structure, which is
//           the structure used to determine if a resource should be loaded 
//           for the current level.
// Inputs:   Same as AddImage, but format & flags paramaters are ignored
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void  AddSound(LoadResStruct *info, const char *name, unsigned short id,
               unsigned int level)
{
  AddImage(info, name, id, level, 0, 0);
}

//------------------------------------------------------------------------------
// Name:     LoadImage
// Summary:  Loads the image specified in the resource structure and formats
//           it as described by the formatting flags contained in the structure
// Inputs:   LoadResStruct - structure contining info on image to load
// Outputs:  None
// Returns:  SDL_Surface of image loaded into memory
// Cautions: None
//------------------------------------------------------------------------------
SDL_Surface *LoadImage(LoadResStruct *ptr)
{
  SDL_Surface *tmp;
  SDL_Surface *scr = MM_GetScreenPtr();  
  tmp = ZIP_LoadImage(ptr->name);
  
  if (ptr->format & SCREEN_FORMAT)   // convert to screen format
  {
    SDL_Surface *tmp1 = SDL_ConvertSurface(tmp, scr->format, ptr->flags);  
    SDL_FreeSurface(tmp);
    tmp = tmp1;
  }
  
  if (ptr->format & TRANSP_FORMAT) // activate transparent background
    SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, SDL_MapRGB(tmp->format, 0xFF, 0x80, 0x80));      
  
  return(tmp);
}  

//------------------------------------------------------------------------------
// Name:     RM_GetImage
// Summary:  Uses the image ID (as specified in Resource Manager Header File)
//           to find a return the corresponding SDL_Surface to the caller
// Inputs:   Image ID (as specified in Resource Manager Header File)
// Outputs:  None
// Returns:  SDL_Surface pointer to image
// Cautions: If an image is not loaded at the given index, a SEVERE ERROR
//           will occur and stop the game.  This can only mean the user has
//           been screwing around with the resource file.  Shame on him.
//------------------------------------------------------------------------------
SDL_Surface* RM_GetImage(int index)
{
  SDL_Surface *ptr = 0;
  if (index < NUM_IMAGES && index >= 0)
    ptr = _images[index];
  else
    ptr = 0;

  if (ptr == 0)
  {
    EH_Error(EH_SEVERE, "Could not load Image at index %i\n", index); 
  }
  return(ptr);
}

//------------------------------------------------------------------------------
// Name:     RM_GetImageWidth
// Summary:  Wrapper function used to quickly return the width of the 
//           specified image
// Inputs:   Image ID (as specified in Resource Manager Header File)
// Outputs:  None
// Returns:  Width of image (in pixels)
// Cautions: Width of 0 is returned if image at specified ID is not present.
//           But... In most cases the call to RM_GetImageWidth will result 
//           in a SEVERE_ERROR before that can occur.
//------------------------------------------------------------------------------
int RM_GetImageWidth(int index)
{
  int width      = 0;
  SDL_Surface *t = RM_GetImage(index);
  
  if (t)
    width = t->w;
  
  return(width);
}

//------------------------------------------------------------------------------
// Name:     RM_GetSoundFx
// Summary:  Function used to return specified SFX
// Inputs:   SFX ID (as specified in Resource Manager Header File)
// Outputs:  None
// Returns:  Mix_Chunk - Pointer to SFX structure
// Cautions: None
//------------------------------------------------------------------------------
Mix_Chunk* RM_GetSoundFx(int index)
{
  Mix_Chunk *ptr = 0;
  if (index < NUM_SOUNDS && index >= 0)
    ptr = _sounds[index];
  else
    ptr = 0;

  return(ptr);
}

//------------------------------------------------------------------------------
// Name:     RM_GetLastLevelLoaded
// Summary:  Tracks the last level loaded by the resource manager
// Inputs:   None
// Outputs:  None
// Returns:  Last level loaded by resource manager
// Cautions: None
//------------------------------------------------------------------------------
unsigned int RM_GetLastLevelLoaded() 
  { return(_lastLevelLoaded); }


// Simple interface functions that hide / ease the use of SDL_Mixer functions
// for playing/stopping/looping audio clips

// Plays SFX based on given ID
int RM_PlaySound(int index)
  { return(Mix_PlayChannel(-1, _sounds[index], 0)); }

// Plays (in a continuous loop) the SFX based on given ID
int RM_PlaySoundLoop(int index) 
  { return(Mix_PlayChannel(-1, _sounds[index], -1)); }

// stops SFX playing on the given channel
int RM_StopSound(int channel) 
  { return(Mix_HaltChannel(channel)); }

// Pauses SFX playing on the given channel
void RM_PauseSound(int channel)
  { Mix_Pause(channel); }

// Resumes SFX playing on the given channel
void RM_ResumeSound(int channel)
  { Mix_Resume(channel); }

