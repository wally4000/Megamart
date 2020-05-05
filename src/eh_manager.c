//-----------------------------------------------------------------------------
//  Class:
//  Error Handler Manager
//
//  Description:
//  This class handles all errors that can occur in the game.  If SDL is 
//  initilized, this class will use the SDL TTF class to draw text errors
//  to the screen.  Otherwise the PSP SCE IO functions are used.  This
//  class should only be used to display SEVERE ERRORS to the user,
//  Al other levels of error are mainly used for debugging purposes.  If a
//  SEVERE ERROR occurs, it will halt the game, and the program will need to
//  be re-started.  This class is priceless for displaying text on screen 
//  when testing & debugging as the SCE functions used to print text do not 
//  play nicely with SDL.
//-----------------------------------------------------------------------------


#include <stdarg.h>
#include <stdio.h>
#include "eh_manager.h"
#include "zip_manager.h"
#include "common.h"


#define fprintf(x, args...) pspDebugScreenPrintf(args)
#define printf(args...) pspDebugScreenPrintf(args)

#define MAX_BUFFER_SIZE 500

static SDL_Surface  *_scr;
static char _buffer[MAX_BUFFER_SIZE];
static int _dataFlag;
static SDL_Color _fgColor = {255,0,0};
static SDL_Color _bgColor = {0,0,0};
static int _init          = 0;
static ZIP_Font  *_font;

//------------------------------------------------------------------------------
// Name:     EH_Init
// Summary:  Called 1X, initialses Error Handler Manager for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void EH_Init()
{
  _scr       = MM_GetScreenPtr();
  _buffer[0] = 0; 
  _dataFlag  = 0;
  ZIP_OpenZipFile(ZIP_MAIN);
  _font      = ZIP_LoadFont(ZIP_FONT1, 12);
  ZIP_CloseZipFile();
  _init      = 1;
}

//------------------------------------------------------------------------------
// Name:     EH_Error
// Summary:  Called to store error into buffer so it can be printed to screen 
//           the next time a screen flip occurs. 
// Inputs:   1. Severity of error
//           2. Arguments list (same as is used by sprintf/printf)
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void EH_Error(int severity, const char *format, ...)
{
  char buf [1000];
  va_list opt;
  va_start(opt, format);
  vsprintf(&buf[0], format, opt);  // Create formatted string
  _dataFlag = 1;
  
  // make sure new combined string does not overflow buffer
  if ( strlen(_buffer) + strlen(buf) >= MAX_BUFFER_SIZE)
  {
    // If buffer is overflowed, pre-empt current error with
    // a sever error alerting user to buffer overflow
    int endStrPos;
    strcpy(buf, "BUFFER OVERFLOW!\n");
    // Adjust end of buffer so we have room to append overflow message
    endStrPos = MAX_BUFFER_SIZE - strlen(buf) - 20;
    _buffer[endStrPos] = 0;  // Set new buffer end position
    severity = EH_SEVERE;    // make error a sever error
  }

  if (severity == EH_SEVERE)
  {
    char tmpBuf[MAX_BUFFER_SIZE];
    strcpy(tmpBuf, _buffer);
    sprintf(_buffer, "SEVERE ERROR:\n%s\n", buf);
    strcat(_buffer, tmpBuf);
    EH_DrawErrors();
    SDL_Flip(_scr);
    SDL_Delay(10000);  // Wait 10 seconds before exiting
    exit(0);
  }
  else if (severity == EH_INFO)
  {
    strcat(_buffer, buf);
    SDL_FillRect(_scr, 0, 0);
    EH_DrawErrors();
    SDL_Flip(_scr);
  }
  else
  {
    strcat(_buffer, buf); 
  }
}

//------------------------------------------------------------------------------
// Name:     EH_DrawErrors
// Summary:  Takes error/warning/debug messages that have been buffered and 
//           draws them to the screen.  Wraps text around screen if a line of 
//           text exceeds the PSP screen width.
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: This function should be called just before flipping the draw 
//           buffers.  The images drawn from this class ARE NOT arranged by
//           the Draw List Manager Class, so this function should be called
//           After the drawlist class draws its data to the screen.
//------------------------------------------------------------------------------
void EH_DrawErrors()
{
  int i, w, h;
  int lastSpace  = 0;
  int start      = 0;
  char *p        = _buffer;
  SDL_Surface *s   = 0;
  SDL_Surface *tmp = 0;
  int length     = strlen(_buffer);
  SDL_Rect dst   = { 0, 0, 0,0 };
  
  if (_init == 0)
  {
    pspDebugScreenClear();
    pspDebugScreenSetXY(0,0);
    printf("%s", _buffer);
    return; 
  }
  
  if (_dataFlag == 0)
  {
    return; 
  }
  else
  {
    _dataFlag = 0;
  }
  
  for (i=0; i < length+1; i++)
  {
    if (p[i] == ' ')  // break sentance on spaces
    {
      p[i] = 0;  // set end of string to current space
      // get length of current string
      TTF_SizeText(_font->f, &p[start], &w, &h); 
      p[i] = ' '; // turn null back into space
      // if string is too long, print what we have so far
      if (w > MM_SCREEN_WIDTH) 
      {
        // Special check for cases where > SW consecutive characters appear
        if (lastSpace<start) lastSpace = i;
        // find last space that did NOT make the string too long
        p[lastSpace] = 0; 
        s = TTF_RenderText_Shaded(_font->f, &p[start], _fgColor, _bgColor);
        start = lastSpace+1; // set new start of screen to the very next space
      }
      lastSpace = i; // allways set the last space to the last space seen
    }
    else if (p[i] == '\n') // handle newlines
    {
      p[i] = 0; // set end of string to curent newline
      TTF_SizeText(_font->f, &p[start], &w, &h);
      p[i] = '\n'; // set back to newline
      if (w > MM_SCREEN_WIDTH) // if string is to long
      {
        // Special check for cases where > SW consecutive characters appear
        if (lastSpace<start) lastSpace = i;
        p[lastSpace] = 0;  // find previous space before newline.  
        s = TTF_RenderText_Shaded(_font->f, &p[start], _fgColor, _bgColor);
        start = lastSpace+1; // set new string start position
        // Set string index back 1 so newline will be processed again
        if (lastSpace != i) i--; 
      }
      else // string was not to long
      {
        p[i] = 0; // set null temrinator to where newline is and print string
        s = TTF_RenderText_Shaded(_font->f, &p[start], _fgColor, _bgColor);
        start = i+1; // set new start position
      }
    }
    else if (p[i] == 0)  // if end of string is reached...
    {
      TTF_SizeText(_font->f, &p[start], &w, &h);
      // if to long, print from start to previous space
      if (w > MM_SCREEN_WIDTH)  
      {
        // Special check for cases where > SW consecutive characters appear
        if (lastSpace<start) lastSpace = i;
        p[lastSpace] = 0;
        s = TTF_RenderText_Shaded(_font->f, &p[start], _fgColor, _bgColor);
        start = i = lastSpace+1;
      }
      else  // if not to long, just print what is left
        s = TTF_RenderText_Shaded(_font->f, &p[start], _fgColor, _bgColor);
    }
    
    // draw only when there is something to draw
    if (s && s->w > 0)
    {
      tmp = SDL_ConvertSurface(s, _scr->format, SDL_SWSURFACE);
      SDL_BlitSurface(tmp, 0, _scr, &dst);
      SDL_FreeSurface(tmp);
      SDL_FreeSurface(s);
      s = 0;
      dst.y += h; // move down to next line
    }
  }
  _buffer[0] = 0;
}

