//-----------------------------------------------------------------------------
//  Class:
//  Controller Configuration Manager
//
//  Description:
//  This file loads the default controller values that map buttons to hero 
//  actions.  If a configuration file exists on disk, this class will load 
//  in the values given in that file.  Otherwise it configures the controller
//  to use the hard coded default button mapping values.
//-----------------------------------------------------------------------------
#include "cc_manager.h"

#define NUM_ACTIONS   8
#define NUM_BUTTONS  13 // 1 extra for "None" button

#define RESTORE_DEFAULTS  NUM_ACTIONS
#define EXIT_SAVE         NUM_ACTIONS+1    
#define EXIT_NO_SAVE      NUM_ACTIONS+2

static int  _dControls[NUM_ACTIONS];
static char *_fileName;
static SDL_Surface *_scr;

static int  VerifyAllActionsSet(int ctrl[]);
static int  VerifyControllerData(int ctrl[]);
static int  LoadControllerData(char *fileName, int ctrl[]);
static void DrawWarning(char *txt, ZIP_Font *f1);
static void AdjustActionButtonMapping(int curIndex, int ctrl[]);

//------------------------------------------------------------------------------
// Name:     CC_Init
// Summary:  Called 1X, initialses CC Manager for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void CC_Init()
{
  int ctrl[NUM_ACTIONS];
  
  _fileName     = "controls.ini";
  _scr          = MM_GetScreenPtr();
  // default controller mappings
  _dControls[0] = CC_CROSS;   // Jump
  _dControls[1] = CC_DOWN;    // Duck
  _dControls[2] = CC_CIRCLE;  // Run
  _dControls[3] = CC_SQUARE;  // Attack
  _dControls[4] = CC_LEFT;    // Move Left
  _dControls[5] = CC_RIGHT;   // Move Right
  _dControls[6] = CC_SELECT;  // Screenshot
  _dControls[7] = CC_START;   // Pause

  
  // Load controller data (supplies default data if none is present)
  LoadControllerData(_fileName, ctrl);
  _CCCtrlJump       = ctrl[0];
  _CCCtrlDuck       = ctrl[1];
  _CCCtrlRun        = ctrl[2];
  _CCCtrlAttack     = ctrl[3];
  _CCCtrlMoveLeft   = ctrl[4];
  _CCCtrlMoveRight  = ctrl[5];
  _CCCtrlScreenShot = ctrl[6];
  _CCCtrlPause      = ctrl[7];
}

void CC_ConfigureControls(SDL_Event *event)
{
  SDL_Color color1    = {0,0,0};
  SDL_Color color2    = {0,0,255};
  int index           = 0; 
  int x               = 0;
  int color           = 0;
  int saveToFile      = 0;
  int tableYOffset    = 40;
  int lineYOffset     = 6;
  int lineXOffset     = 214;
  int lineSpace       = 0;
  int loop            = 1;
  int reDraw          = 1;
  int warnIndex       = 0;
  int dataSize        = sizeof(unsigned int) * NUM_ACTIONS;
  SDL_Rect lineDstRec = {0,0,0,0};
  FILE        *file   = 0;
  ZIP_Font    *f1;
  SDL_Surface *tmp;
  SDL_Surface *bgImg;
  SDL_Surface *resDefImg[2];
  SDL_Surface *ExitSave[2];
  SDL_Surface *ExitNoSave[2];
  int ctrl[NUM_ACTIONS];
  
  // Holds text labels
  SDL_Surface *actionToImg[2][NUM_ACTIONS];
  
  // Holds button Labels
  SDL_Surface *btnToImg[2][NUM_BUTTONS];
  
  // Array of text strings used to generate Action Labels
  char *actionToText[NUM_ACTIONS] = {"Jump", "Duck & Cover", "Run", "Attack", 
                                     "Move Left", "Move Right", "Screenshot", 
                                     "Pause" };
                                        
  // Array of text strings used to generate Button Labels
  char *btnToText[NUM_BUTTONS] = {"Triangle", "Circle", "Cross", "Square", 
                                  "L. Trigger", "R. Trigger", "Down", "Left",
                                  "Up", "Right", "Select", "Start",
                                  "None" };

  // Load reources
  ZIP_OpenZipFile(ZIP_MAIN);
  f1  = ZIP_LoadFont(ZIP_FONT2, 12);
  tmp = ZIP_LoadImage("logo_big.bmp");
  ZIP_CloseZipFile();
  
  TTF_SetFontStyle(f1->f, TTF_STYLE_BOLD);
  // Convert background image to screen format
  bgImg = SDL_ConvertSurface(tmp, _scr->format, SDL_SWSURFACE); 
  SDL_FreeSurface(tmp);
  
  // Create Labels for Actions (one for idle color, one for selected color)
  for (index = 0; index < NUM_ACTIONS; index++)
  {
    actionToImg[0][index] = TTF_RenderText_Blended(f1->f, 
                                                   actionToText[index], color1);
    actionToImg[1][index] = TTF_RenderText_Blended(f1->f, 
                                                   actionToText[index], color2);
    if (actionToImg[0][index]->w > lineSpace)
    {
      lineSpace = actionToImg[0][index]->w;
    }
  }
  lineSpace += 30;
  
  // Create Labels for Buttons
  for (index = 0; index < NUM_BUTTONS; index++)
  {
    btnToImg[0][index] = TTF_RenderText_Blended(f1->f, 
                                                btnToText[index], color1);
    btnToImg[1][index] = TTF_RenderText_Blended(f1->f, 
                                                btnToText[index], color2);
  }
  
  // Create Description Labels
  resDefImg[0]  = 
    TTF_RenderText_Blended(f1->f, "Restore Defaults", color1);
  resDefImg[1]  = 
    TTF_RenderText_Blended(f1->f, "Restore Defaults", color2);
  ExitSave[0]   = 
    TTF_RenderText_Blended(f1->f, "Exit (Save Changes)", color1);
  ExitSave[1]   = 
    TTF_RenderText_Blended(f1->f, "Exit (Save Changes)", color2);
  ExitNoSave[0] = 
    TTF_RenderText_Blended(f1->f, "Exit (Discard Changes)", color1);
  ExitNoSave[1] = 
    TTF_RenderText_Blended(f1->f, "Exit (Discard Changes)", color2);
  
  // Read controller data from file and verify it's valid.  Default values
  // will be returned if a controler configuration file does not exist
  LoadControllerData(_fileName, ctrl);
  
  // Handle user input
  index = 0;
  while (loop)
  {
    while (SDL_PollEvent(event)) 
    {
      switch (event->type) 
      {
        case SDL_JOYBUTTONDOWN:
        if(event->jbutton.button == CC_START    || 
           event->jbutton.button == CC_CIRCLE   || 
           event->jbutton.button == CC_CROSS    ||
           event->jbutton.button == CC_TRIANGLE ||
           event->jbutton.button == CC_SQUARE   ||
           event->jbutton.button == CC_RIGHT    ||
           event->jbutton.button == CC_LEFT )
        {
          reDraw = 1;
          // Anything at an index less than NUM_ACTIONS is an action 
          // that can be configured
          if ( index < NUM_ACTIONS )
          {
            (event->jbutton.button == CC_LEFT)?ctrl[index]++:ctrl[index]--;

            // Bounds checking, Loop to first allowed button value
            if (ctrl[index] >  NUM_BUTTONS-2) 
              ctrl[index] = 0;
            // loop to last allowed Button value
            else if (ctrl[index] < 0)
              ctrl[index] = NUM_BUTTONS-2;
          }
          else if (index == RESTORE_DEFAULTS)
          {
            memcpy(ctrl, _dControls, dataSize);
          }
          else if (index == EXIT_SAVE)
          {
            // verify the user has mapped a button to every action
            warnIndex = VerifyAllActionsSet(ctrl);
            // Less than 0 means current control values are valid
            if (warnIndex < 0)
            {
              loop       = 0;
              saveToFile = 1;
            }
            // 0 or greater means the action at the specified index was 
            // not mapped to a valid button
            else
            {
              DrawWarning(actionToText[warnIndex], f1);
            }
          }
          else if (index == EXIT_NO_SAVE)
          {
            loop = 0;
          }
          else
          {
            EH_Error(EH_SEVERE, 
                     "CC_ConfigureControls: Invalid Index Value=%i\n", index);
          }
        }

        // User scrolls up / down to select a different Action to configure
        if (event->jbutton.button == CC_UP ||
            event->jbutton.button == CC_DOWN )
        {
          reDraw = 1;
          
          // When switching options, always check to see if user
          // selected a button for the current action that was mapped to 
          // another action.  If button is in use by multiple actions, set
          // the button value for all other actions to none
          AdjustActionButtonMapping(index, ctrl);
          
          (event->jbutton.button == CC_UP)?index--:index++;
          
          // bounds checking
          if (index < 0)
            index = EXIT_NO_SAVE; // EXIT_NO_SAVE is biggest allowed index
          else if ( index > EXIT_NO_SAVE)
            index = 0;
        }
        break;
      } // END switch (event->type) 
    }   // END while (SDL_PollEvent(event)) 
    
    // Re-Draw the menu if use did something
    if (reDraw)
    {
      SDL_BlitSurface(bgImg, 0, _scr, 0);
      //SDL_FillRect(_scr, 0, 0);

      // Postion Text to begin where table will be drawn
      lineDstRec.y = tableYOffset;
      // Draw Actions / Button Mappings
      for (x = 0; x < NUM_ACTIONS; x++)
      {
        color = (x == index)?1:0;
        lineDstRec.x = lineXOffset;
        SDL_BlitSurface(actionToImg[color][x], 0, _scr, &lineDstRec);
        lineDstRec.x = lineXOffset + lineSpace;
        SDL_BlitSurface(btnToImg[color][ctrl[x]], 0, _scr, &lineDstRec);
        lineDstRec.y += actionToImg[color][x]->h + lineYOffset;
      }

      lineDstRec.x = lineXOffset;
      // Draw Restore / Save / Exit options
      color = (index == RESTORE_DEFAULTS)?1:0;
      SDL_BlitSurface(resDefImg[color], 0, _scr, &lineDstRec);
      lineDstRec.y += resDefImg[color]->h + lineYOffset;
        
      color = (index == EXIT_SAVE)?1:0;
      SDL_BlitSurface(ExitSave[color], 0, _scr, &lineDstRec);
      lineDstRec.y += ExitSave[color]->h + lineYOffset;
        
      color = (index == EXIT_NO_SAVE)?1:0;
      SDL_BlitSurface(ExitNoSave[color], 0, _scr, &lineDstRec);
      lineDstRec.y += ExitNoSave[color]->h + lineYOffset;
      
      sceDisplayWaitVblankStart();
      SDL_Flip(_scr);  
    }
  }
  
  // Save changes specified by user
  if (saveToFile)
  {
    // make changes active in current game
    _CCCtrlJump       = ctrl[0];
    _CCCtrlDuck       = ctrl[1];
    _CCCtrlRun        = ctrl[2];
    _CCCtrlAttack     = ctrl[3];
    _CCCtrlMoveLeft   = ctrl[4];
    _CCCtrlMoveRight  = ctrl[5];
    _CCCtrlScreenShot = ctrl[6];
    _CCCtrlPause      = ctrl[7];
    
  
    // write changes to configuration file so they will be loaded
    // next time a user plays this awsome game
    file = fopen(_fileName, "wb"); 
    fwrite((void*) ctrl, dataSize, 1, file );
    fclose(file);  // close file
  }
    
  // Free ALL SDL Surfaces
  for (index = 0; index < NUM_ACTIONS; index++)
  {
    SDL_FreeSurface(actionToImg[0][index]);
    SDL_FreeSurface(actionToImg[1][index]);
  }

  for (index = 0; index < NUM_BUTTONS; index++)
  {
    SDL_FreeSurface(btnToImg[0][index]);
    SDL_FreeSurface(btnToImg[1][index]);
  }
  
  SDL_FreeSurface(bgImg);
  SDL_FreeSurface(resDefImg[0]);
  SDL_FreeSurface(resDefImg[1]);
  SDL_FreeSurface(ExitSave[0]);
  SDL_FreeSurface(ExitSave[1]);
  SDL_FreeSurface(ExitNoSave[0]);
  SDL_FreeSurface(ExitNoSave[1]);
  
  ZIP_CloseFont(f1);
}

//------------------------------------------------------------------------------
// Name:     VerifyAllActionsSet
// Summary:  Ensures that all hero actions map to a valid button
// Inputs:   None
// Outputs:  None
// Returns:  -1 on success, invalid button value on error
// Cautions: None
//------------------------------------------------------------------------------
int VerifyAllActionsSet(int ctrl[])
{
  int index;
  int ret = -1;
  
  // Check Each action and make sure it does not map to NONE
  for (index = 0; index < NUM_ACTIONS; index++)
  { 
    // if action maps to the last possible button value (the NONE button)
    // than this action is not mapped.  return the index of the unmapped 
    // action
    if (ctrl[index] == NUM_BUTTONS-1)
    {
      ret = index;
      break;
    }
      
  }
  
  return(ret);
}

//------------------------------------------------------------------------------
// Name:     LoadControllerData
// Summary:  Loads the controller configuration information from the default 
//           controller configuration file
// Inputs:   1.  File name
//           2.  Array to hold values read in from file
// Outputs:  None
// Returns:  0 on success, 1 on error
// Cautions: Default controller values are used if a configuration file does
//           not exist
//------------------------------------------------------------------------------
int LoadControllerData(char *fileName, int ctrl[])
{
  FILE *file;
  int dataSize = sizeof(unsigned int) * NUM_ACTIONS;
  int ret;
  
  //Open file for read
  file = fopen(fileName, "rb"); 
  
  // if file opened, load the data
  if (file)
  {
    fread((void *) ctrl, dataSize, 1, file);
    fclose(file);  // close file
  }
  // if file failed to load, copy in the default controller configuration
  else
  {
    memcpy(ctrl, _dControls, dataSize);
  }
  
  // verify data is valid, set to defaults if it is not valid
  ret = VerifyControllerData(ctrl);
  
  return(ret);
}

//------------------------------------------------------------------------------
// Name:     VerifyControllerData
// Summary:  Verifies each button maps to a single action, verifies the button 
//           value mapped to each action is an allowed value
// Inputs:   Array of controller information to verify
// Outputs:  None
// Returns:  0 on success, 1 on error
// Cautions: None
//------------------------------------------------------------------------------
int VerifyControllerData(int ctrl[])
{
  int index;
  int inner;
  int dataSize = sizeof(unsigned int) * NUM_ACTIONS;
  int fail     = 0;
  int ret      = 0;
  
  if (fail == 0)
  {
    // loop through each action
    for (index = 0; index < NUM_ACTIONS; index++)
    {
      // Verify button value mapped to each action is in range
      // NUM_BUTTONS is 1 greater than actual number of buttons, since we
      // include the "None" button in this count.  When verifying 
      // action button values, the None value is not considered valid
      if (ctrl[index] < 0 || ctrl[index] >= NUM_BUTTONS-1)
      {
        fail = 1;
        break; 
      }
      
      // compare current action against every other action
      for (inner = 0; inner < NUM_ACTIONS; inner++)
      {
        // skip if current action compares to itself
        if (index == inner)
          continue;
        
        // if 2 actions have same button mapped to it, configuration
        // data is not valie
        if ( ctrl[index] == ctrl[inner] )
        {
          fail = 1;
          break;
        }
      }
      if (fail)
        break; // exit loop if failure occured
    }
  }
  
  // If current controller dat is not valid, copy in default 
  // controller values
  if (fail)
  {
    memcpy(ctrl, _dControls, dataSize);
    ret = 1;
  }
  return(ret);
}

//------------------------------------------------------------------------------
// Name:     AdjustActionButtonMapping
// Summary:  When user changes a buttons mapping, this function will compare 
//           the new button mapping to every other action to ensure no other 
//           actions share this value.  If other actions now share the same
//           mapping as the current action, their buton values will be reset 
//           to map to the NONE button (Meaning that action is not currently 
//           mapped to a valid button)
// Inputs:   1. Current Index of action we wish to verify
//           2. Controller data
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void AdjustActionButtonMapping(int curIndex, int ctrl[])
{
  int index = 0;
  
  // loop through each action
  for (index = 0; index < NUM_ACTIONS; index++)
  {
    // skip if current action is being compared to itself
    if (index == curIndex)
      continue;
      
    // if 2 actions have same button mapped to it, and the mapped button
    // is not the "None" button, set the inner value to the "None" button
    if ( ctrl[curIndex] == ctrl[index] && ctrl[index] != NUM_BUTTONS-1)
    {
      ctrl[index] = NUM_BUTTONS-1; // last button label is "None" label
    }
  }
}

//------------------------------------------------------------------------------
// Name:     DrawWarning
// Summary:  Warning message that gets displayed to user if they attempt to 
//           exit the controller configuration menu without mapping a valid
//           button to each hero action.
// Inputs:   1. Text string of the action that needs a button mapped to it
//           2. Fons used to create SDL_Surface of text
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void DrawWarning(char *txt, ZIP_Font *f1)
{
  SDL_Rect    fillRecDst;
  SDL_Rect    text1RecDst;
  SDL_Rect    text2RecDst;
  SDL_Surface *txt1Img;
  SDL_Surface *txt2Img;
  SDL_Color fgColor = {0,0,255};
  char buffer[100];
  
  sprintf(&buffer[0], "Please provide the [%s]", txt);
  
  txt1Img = TTF_RenderText_Blended(f1->f, buffer, fgColor);
  txt2Img = TTF_RenderText_Blended(f1->f, 
                                   "action with a valid value.", fgColor);

  // Determine width of rectangle based on width of logest text string
  if ( txt1Img->w > txt2Img->w)
    fillRecDst.w = txt1Img->w + 10;
  else
    fillRecDst.w = txt2Img->w + 10;

  // Determine height of rectangle
  fillRecDst.h = (txt1Img->h * 2) + 10;
  
  // Center horizontaly
  fillRecDst.x = ((MM_SCREEN_WIDTH-145)/2) - (fillRecDst.w/2) + 145; 
  
  // Center vertically
  fillRecDst.y = (MM_SCREEN_HEIGHT/2) - (fillRecDst.h/2);
  
  // Center text 1
  text1RecDst.x = ((MM_SCREEN_WIDTH-145)/2) - (txt1Img->w/2) + 145; 
  text1RecDst.y = fillRecDst.y + 5;
  
  // Center text 2
  text2RecDst.x = ((MM_SCREEN_WIDTH-145)/2)  - (txt2Img->w/2) + 145; 
  text2RecDst.y = text1RecDst.y + txt1Img->h;
  
  // Draw data
  SDL_FillRect(_scr, &fillRecDst, SDL_MapRGB(_scr->format, 128, 128, 128));
  SDL_BlitSurface(txt1Img, 0, _scr, &text1RecDst);
  SDL_BlitSurface(txt2Img, 0, _scr, &text2RecDst);
  
  // Flip buffers
  sceDisplayWaitVblankStart();
  SDL_Flip(_scr);  
  
  SDL_FreeSurface(txt1Img);
  SDL_FreeSurface(txt2Img);
  
  // wait 3 seconds
  SDL_Delay(3000);
}

