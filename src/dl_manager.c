//-----------------------------------------------------------------------------
//  Class:
//  Draw List Manager
//
//  Description:
//  This class manages the list of sprites that will be drawn to the screen.
//  Sprites are added to the list in sorted order based on their Z position.
//  Small values get drawn first while big values get drawn last.  This allows
//  sprites to be draw on screen in the proper order so that their overlapping
//  behavior can be controlled.  All sprites used in the Power, Hero,& Sprite
//  manager classes are added to this list and drawn from here
//-----------------------------------------------------------------------------


#include "dl_manager.h"

// Private data
static DL_LinkedListNode _head;
static DL_LinkedListNode _tail;
static DL_LinkedListNode *_curPtr;


//-----------------------------------------------------------------------------
// Name:     DL_Init
// Summary:  Initialise the Draw List.  Draw List is used to draw sprites
//           to the screen in the correct order.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Notes:    Small keys are drawn first, bigger keys are drawn last.  
//-----------------------------------------------------------------------------
void DL_Init()
{
  _curPtr    = &_head;  // set current pointer to head
  _head.prev = 0;       // head's previous is set to null
  _tail.next = 0;       // tail's next is set to null
  _head.next = &_tail;  // initialy head points to tail
  _tail.prev = &_head;  // and tail points to head
  _head.key  = 0;       // head gets small value, no smaller keys may exist
  _tail.key  = 1000000; // tail gets big value, bo bigger keys may exist
}

int DL_InitLevel(unsigned int level)
{
  DL_ClearList();
  return(0);
}

//-----------------------------------------------------------------------------
// Name:     DL_Add
// Summary:  Adds an entry to the draw list
// Inputs:   Pointer to the entry to be added (does not really ahve to be of
//           type DL_LinkedListNode.  All that matters is that the first 3 
//           elements of the passed in structure are prev, next, and key).
// Outputs:  None
// Returns:  None
// Cautions: Passed in structre need not be of type DL_LinkedListNode.  It IS
//           required that the first 3 elements of the passed in structre be 
//           the exact same 3 elements that appear in the DL_LinkedListNode 
//           structure).
//-----------------------------------------------------------------------------
void DL_Add(DL_LinkedListNode *s)
{
  DL_LinkedListNode *tmp = _head.next;
  
  // NOTE: the key value of the current node is used to determined the node's
  // value, I.E. where it gets inserted into the list.
  // Search list for first node that is bigger or equal to node we are adding.
  // Small numbers get placed at head of list.  If new node is equal to an 
  // existing node (or series of nodes), the new node will be placed as the
  // last one in the list.  
  while (s->key >= tmp->key)
  {
    tmp = tmp->next;
  }
  
  // arrange pointers of new node and the prev/next nodes
  s->next   = tmp;
  s->prev   = tmp->prev;
  tmp->prev = s;
  tmp       = s->prev;
  tmp->next = s;
}  

//-----------------------------------------------------------------------------
// Name:     DL_Next
// Summary:  Points Iterator to the next element in the linked list
// Inputs:   None
// Outputs:  None
// Returns:  0 if end of list is reached (I.E. no more data), 1 otherwise.
// Cautions: When end of list is reached, iterator is set to head of list. 
//           Thus, DL_Next must be called to set it to 1st element in list.
//-----------------------------------------------------------------------------
int DL_Next()
{
  int ret             = 0;
  DL_LinkedListNode *tmp = _curPtr;
  if (tmp->next == &_tail)  // end of list is reached, reset list
  {
    DL_ResetCurrentToStart();
    ret = 0;
  }
  else  // not at end of list, simply move to next element
  {
    _curPtr = tmp->next;
    ret = 1;
  }
  return(ret);
}

//-----------------------------------------------------------------------------
// Name:     DL_ResetCurrentToStart
// Summary:  Points Iterator to head/start of linked list
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: DL_Next must be called to move from head to 1st element in list.
//-----------------------------------------------------------------------------
void DL_ResetCurrentToStart()
{
  _curPtr = &_head;
}

//-----------------------------------------------------------------------------
// Name:     DL_GetCurrentData
// Summary:  Returns the data stored at the current node in the list
// Inputs:   None
// Outputs:  None
// Returns:  0 on failure or no data, an address to the data on success
// Cautions: None
//-----------------------------------------------------------------------------
void *DL_GetCurrentData()
{
  return(_curPtr); 
}

//-----------------------------------------------------------------------------
// Name:     DL_Remove
// Summary:  Removes passed in node from the list
// Inputs:   Pointer to node that must be removed
// Outputs:  None
// Returns:  0 on success, non zero on failure
// Cautions: If current node is the node being removed, the list iterator 
//           (_curPtr) will be moved back 1, to the elemnt immedaitly before
//           it in the list.  If this is the head, LL_Next must be called to 
//           get the 1st element in the adjusted list.
//-----------------------------------------------------------------------------
void DL_Remove(DL_LinkedListNode *cur)
{
  // adjust Iterator if the node it points to is being removed
  if (_curPtr == cur)
    _curPtr = cur->prev;
  
  // re-position pointers of current node's next and previous nodes
  // to each other
  DL_LinkedListNode *prevNode = cur->prev;
  DL_LinkedListNode *nextNode = cur->next;
  
  // verify a previous and next node pointer exist 
  // if we attempt to remove the same element 2X this problem could occur
  if (prevNode)
    prevNode->next = nextNode;
  if (nextNode)
    nextNode->prev = prevNode;
  
  cur->prev = 0;
  cur->next = 0;
  cur->key  = 5;
}

//-----------------------------------------------------------------------------
// Name:     DL_ClearList
// Summary:  Removes all nodes (except head and tail) from linked list.  
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void DL_ClearList()
{
  DL_Init();
}

//-----------------------------------------------------------------------------
// Name:     DL_DrawImages
// Summary:  Loops through the drawlist and calls the callback function used
//           to draw each sprite to the screen
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//-----------------------------------------------------------------------------
void  DL_DrawImages()
{
  DL_LinkedListNode *s;
  while (DL_Next())
  {
    s = DL_GetCurrentData();
    s->DrawImage((void*)s);
  }
}

