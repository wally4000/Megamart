#ifndef __MAP_MANAGER_H__
#define __MAP_MANAGER_H__

void MAP_Init();
int  MAP_InitLevel();
int  MAP_DebugReset();
void MAP_EnableObjects(float move);
int  MAP_GetLevelSize(unsigned int level);
void MAP_AddExtraLifeObject(int xPos, int yPos, float zPos);

#endif

