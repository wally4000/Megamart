#ifndef __CC_MANAGER_H__
#define __CC_MANAGER_H__
#include "common.h"

#define CC_TRIANGLE          0
#define CC_CIRCLE            1
#define CC_CROSS             2
#define CC_SQUARE            3
#define CC_LEFT_TRIGGER      4
#define CC_RIGHT_TRIGGER     5
#define CC_DOWN              6
#define CC_LEFT              7
#define CC_UP                8
#define CC_RIGHT             9
#define CC_SELECT           10
#define CC_START            11

unsigned int _CCCtrlJump;
unsigned int _CCCtrlDuck;
unsigned int _CCCtrlRun;
unsigned int _CCCtrlAttack;
unsigned int _CCCtrlScreenShot;
unsigned int _CCCtrlPause;
unsigned int _CCCtrlMoveLeft;
unsigned int _CCCtrlMoveRight;


// Public
void CC_Init();
void CC_ConfigureControls(SDL_Event *event);


#endif
