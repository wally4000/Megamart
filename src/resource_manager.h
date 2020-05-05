#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__
#include "common.h"
#include "SDL_mixer.h"

void         RM_Init();
int          RM_InitLevel(unsigned int level);
unsigned int RM_GetLastLevelLoaded();
SDL_Surface* RM_GetImage(int index);
int          RM_GetImageWidth(int index);
Mix_Chunk*   RM_GetSoundFx(int index);
int          RM_PlaySound(int index);
int          RM_StopSound(int channel);
int          RM_PlaySoundLoop(int index);
void         RM_PauseSound(int channel);
void         RM_ResumeSound(int channel);


#define NUM_SHELF_SETS            10
#define NUM_IMAGES               100

#define ARNOLD_SPRITE              2     
#define L1S1_TENT3                 3
#define RM_IMG_BOMB_SPRITE         4
#define EMPLOYEE_SPRITE            5
#define BASKETBALL_SPRITE          6  // Must be first ball
#define BASEBALL_SPRITE            7  
#define SOCCERBALL_SPRITE          8  // Must be last ball
#define GRILL_SPRITE               9
#define TENT_GUY_SPRITE           10
#define PUNCHING_BAG_SPRITE       11
#define GEN_RANDOM_SHELF          12
#define GEN_SHELF_B_SPRITE        13
#define GEN_SHELF_C_SPRITE        14
#define GEN_SHELF_D_SPRITE        15
#define FALLING_SHELF_SPRITE      16
#define RM_IMG_WEIGHTS_SPRITE     17
#define RM_IMG_POWER_UP_SPRITE    18
#define RM_IMG_TWINKLE_SPRITE     19

#define RM_IMG_HERO_EXTRA_LIFE    20
#define RM_IMG_HERO               21
#define RM_IMG_HERO_HURT          22
#define RM_IMG_HERO_DEATH         23
#define RM_IMG_HERO_DUCK          24
#define RM_IMG_HERO_JUMP          25
#define RM_IMG_HERO_WEAPON        26
#define RM_IMG_HERO_POWER_METER   27
#define RM_IMG_HERO_VICTORY       28

#define RM_LEVEL1_COMPLETE_TXT    31
#define RM_SCREENSHOT_1_TXT       32
#define RM_SCREENSHOT_2_TXT       33
#define RM_GAME_PAUSED_TXT        34
#define RM_GAME_OVER_TXT          35
#define RM_RESUME_QUIT_TXT        36
#define RM_CURSOR                 37
#define RM_IMG_CREDITS            38
#define RM_IMG_PROGRESS_BAR       39
#define RM_IMG_PROGRESS_ICON      40



#define RM_BOWLING_SHELF_SPRITE        41
#define RM_RED_BOWLING_BALL_SPRITE     42  // RED must allways be first bowling ball sprite
#define RM_GREEN_BOWLING_BALL_SPRITE   43
#define RM_YELLOW_BOWLING_BALL_SPRITE  44
#define RM_BLUE_BOWLING_BALL_SPRITE    45  // BLUE must allways be last bowling ball sprite

#define TENT_GREEN                 50  // must be first
#define TENT_ORANGE                51
#define TENT_RED                   52
#define TENT_GREY                  53
#define TENT_PURPLE                54
#define TENT_BLUE                  55  // must be last

#define RM_IMG_ARCHER_SPRITE       56
#define RM_IMG_ARROW_SPRITE        57
#define RM_IMG_BOUNCE_BOMB_SPRITE  58
#define RM_IMG_SIGN_SPRITE         59
#define RM_IMG_BICYCLE1_SPRITE     60 // normal
#define RM_IMG_BICYCLE2_SPRITE     61 // wheelie
#define RM_IMG_HOCKEY_EAST_SPRITE  62
#define RM_IMG_HOCKEY_WEST_SPRITE  63
#define RM_IMG_BONUS_LIFE_SPRITE   64


#define RM_IMG_BICYCLE_SPRITE      100  // Placeholder
#define RM_IMG_SERIES_SPRITE       101  // Used to create Series sprites


// Images for Final Level
#define RM_IMG_FL_SCROLL           70
#define RM_IMG_FL_TEXT             71
#define RM_IMG_FL_CURSOR           72
#define RM_IMG_FL_IMAGE            73

// These are permanent images.  Place them at the end of the Image list
// Memory allocated for these images should never be freed.  Some will
// be located in VRAM, other will be located on the stack alligned to 
// 16 bytes.  The PSP SDK has an issue where it cannot free 16 byte 
// alligned memory, so we don't even bother trying to free it. We
// simply take the "whadeva" approach.
#define NUM_PERM_IMAGES            9
#define RM_IMG_BG1                 NUM_IMAGES-1
#define RM_IMG_BG2                 NUM_IMAGES-2
#define RM_IMG_RF1                 NUM_IMAGES-3
#define RM_IMG_RF2                 NUM_IMAGES-4
#define RM_IMG_RF3                 NUM_IMAGES-5
#define RM_IMG_RF4                 NUM_IMAGES-6
#define RM_IMG_RF5                 NUM_IMAGES-7
#define RM_IMG_RF6                 NUM_IMAGES-8
#define RM_IMG_RF7                 NUM_IMAGES-9


#define NUM_SOUNDS              25
#define RM_SFX_HERO_HIT          1
#define RM_SFX_BALL_HIT          2
#define RM_SFX_GRILL_FALLING     3
#define RM_SFX_BOWLING_BALL_FALL 4
#define RM_SFX_CREDITS_MUSIC     5
#define RM_SFX_LEVEL1_MUSIC      6
#define RM_SFX_POWER_UP          7
#define RM_SFX_SHELF_FALL        8         
#define RM_SFX_TENT_HIT          9
#define RM_SFX_SCREENSHOT       10  
#define RM_SFX_EMPLOYEE_HIT     11
#define RM_SFX_HERO_JUMP        12
#define RM_SFX_EXPLOSION        13
#define RM_SFX_MENU_SELECT      14
#define RM_SFX_ARROW            15
#define RM_SFX_BICYCLE          16
#define RM_SFX_GAME_OVER        17
#define RM_SFX_BICYCLE_BELL     18

#define RM_SFX_FINAL_MUSIC      19
#define RM_SFX_FINAL_WIN        20

#endif

