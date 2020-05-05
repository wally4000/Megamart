#ifndef __DL_MANAGER_H__
#define __DL_MANAGER_H__
#include "common.h"


typedef struct DL_LinkedListNode
{
  void *prev;
  void *next;
  float key;
  MM_DrawImageFunction DrawImage;
} DL_LinkedListNode;


// Public
void DL_Init();
int  DL_InitLevel(unsigned int level);
void DL_ClearList();
void DL_Remove(DL_LinkedListNode *cur);
void *DL_GetCurrentData();
void DL_ResetCurrentToStart();
void DL_Add(DL_LinkedListNode *s);
int  DL_Next();
void DL_DrawImages();

#endif
