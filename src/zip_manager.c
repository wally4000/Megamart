//-----------------------------------------------------------------------------
//  Class:
//  Zip Manager
//
//  Description:
//  This class manages the zip archive used to hold all program resources.
//  All classes wishing to access data in the resource file must use this
//  class as an interface.  It should be easy to modify this class to load
//  files straight from a disk for systems where zip files support does not
//  exists (or for which the zip source code is not readily available)
//-----------------------------------------------------------------------------


#include <unzip.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "zip_manager.h"
#include "eh_manager.h"


#define MAX_PATH                255
#define RESOURCE_FILE    "data.lbg"

static unzFile _zipFile;
static int _fileOpen;
static void *LoadZipData(const char *filename, unzFile *zip, int *size);

//------------------------------------------------------------------------------
// Name:     ZIP_OpenZipFile
// Summary:  Opens the specified ZIP file for use
// Inputs:   ID of zip file you wish to open (only 1 exists, but the framework
//           for multiple zip files is present)
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int ZIP_OpenZipFile(unsigned int file)
{
  int status = 0;

  if (_fileOpen)
  {
    status = 1;
  }
  else
  {
    switch(file)
    {
       case ZIP_MAIN:
       _zipFile = unzOpen(RESOURCE_FILE);
       break;
       
       default:
       status = 2;
       break;
    } 
  }
  
  if (!(status))
  {
    _fileOpen = 1;
  }
    
  return(status);
}

//------------------------------------------------------------------------------
// Name:     ZIP_CloseZipFile
// Summary:  Closes the specified zip file
// Inputs:   None
// Outputs:  None
// Returns:  Status value of 0 on success, non-zero on error
// Cautions: None
//------------------------------------------------------------------------------
int ZIP_CloseZipFile()
{
  int status = 0;
  if (_fileOpen)
  {
    unzClose(_zipFile);
    _fileOpen = 0;
  }
  else
  {
    status = 1;    
  }
  return(status);
}

//------------------------------------------------------------------------------
// Name:     ZIP_LoadImage
// Summary:  Loads an image from the ZIP file and stores it into an SDL_Surface
// Inputs:   Name of file to extract from zip
// Outputs:  None
// Returns:  SDL_Surface pointer to image data loaded from zip
// Cautions: None
//------------------------------------------------------------------------------
SDL_Surface *ZIP_LoadImage(const char *img)
{
  int size;
  SDL_Surface *image;
  void *data = LoadZipData(img, &_zipFile, &size);
  if (data)
  {
    SDL_RWops *zipRw = SDL_RWFromConstMem(data, size);
    if (zipRw)
    {
      // 1 means free data after the surface is created 
      image = IMG_Load_RW(zipRw, 1);  
      if (image == 0)
        EH_Error(EH_SEVERE,  
                 "LBG_LoadImage(image): Could not load file %s.", img); 
        
    }
    else
    {
      EH_Error(EH_SEVERE, 
               "LBG_LoadImage(zipRw): Could not load file %s.", img); 
    }
  }
  else
  {
    EH_Error(EH_SEVERE, "LBG_LoadImage(data): Could not load file %s.", img); 
  }
  free(data);
  return(image);
}

//------------------------------------------------------------------------------
// Name:     ZIP_LoadMusic
// Summary:  Loads the specified SFX and stores it into Mix_Chunk
// Inputs:   Name of file to extract from zip
// Outputs:  None
// Returns:  Mix_Chunk pointer to sfx data loaded from zip
// Cautions: None
//------------------------------------------------------------------------------
Mix_Chunk *ZIP_LoadMusic(const char *name)
{
  int size         = 0;
  void *data       = LoadZipData(name, &_zipFile, &size);
	SDL_RWops *zipRw = SDL_RWFromConstMem(data, size);
	Mix_Chunk *sound = Mix_LoadWAV_RW(zipRw, 1);
  
	free(data);
  return(sound);
}

//------------------------------------------------------------------------------
// Name:     ZIP_LoadFont
// Summary:  Loads the specified TTF Font and stores it into a ZIP_Font 
//           structure
// Inputs:   1. Name of file to extract from zip
//           2. point size of font
// Outputs:  None
// Returns:  ZIP_Font pointer to font data loaded from zip
// Cautions: We use a special ZIP_Font structure to wrap up the normal SDL 
//           TTF_Font structure.  Attempring to free TTF loaded into the
//           SDL TTF structure does not work correctly, it seems the PSP SDL
//           port is lacking in this area.  We must use our own font structure
//           wrapper to ensure the memroy allocated for the font gets freed
//           when it is time to free this font.
//------------------------------------------------------------------------------
ZIP_Font *ZIP_LoadFont(const char *name, int ptSize)
{
  ZIP_Font *z = (ZIP_Font*) malloc(sizeof(ZIP_Font));
  int size;
  void *data       = LoadZipData(name, &_zipFile, &size);
	SDL_RWops *zipRw = SDL_RWFromConstMem(data, size);
	TTF_Font *font   = TTF_OpenFontRW(zipRw, 1, ptSize);
  // use me if loading font from a zip file acts up
  //TTF_Font *font = TTF_OpenFont(name, ptSize);
  
  // NOTE: the data value cannot be freed here as it can with other calls to 
  //ZIP_Load.  For TTF files, it seems the TTF_Font structure created needs 
  //access to the data held by the data pointer.  If we free it here, the
  // font structure only works 1/2 the time.  So, we return a structure with 
  //both the font and the data pointer.  Then, when finished instead of 
  //calling TTF_CloseFont, we call ZIP_CloseFOnt and it frees the data, 
  //font, and Font Struct for us.  This should be a permanant solution,
  // but if it starts acting up, simply replace this code with the 
  // TTF_OpenFont call and remove the ttf file from the zip file.
  
  z->f = font;  // font to be freed when font is no longer in use
  z->d = data;  // data to be freed when font is finished
  return(z);
}

//------------------------------------------------------------------------------
// Name:     ZIP_CloseFont
// Summary:  Special wrapper funstion used to close a font and free its memory
// Inputs:   Pointer to ZIP_Font structure to free
// Outputs:  None
// Returns:  NONE
// Cautions: None
//------------------------------------------------------------------------------
void ZIP_CloseFont(ZIP_Font *z)
{
  if (z)  
  {
    // close font
    TTF_CloseFont(z->f);
    if (z->d)  // free font data if it exists (it allways should)
    {
      free(z->d);
    }
    free(z);  // free wrapper font structure
  }
}

//------------------------------------------------------------------------------
// Name:     LoadZipData
// Summary:  Loads specified file from zip into memory
// Inputs:   1. Name of file to extract from zip
//           2. Pointer to zip file structure to extract from
// Outputs:  size - the size of the data extracted from the ZIP archive
// Returns:  Pointer to data extracted from zip archive
// Cautions: None
//------------------------------------------------------------------------------
void *LoadZipData(const char *filename, unzFile *zip, int *size)
{
  unz_file_info zinfo;
  char zipfilename[MAX_PATH];
  unsigned char *data = NULL;
  int found           = 0;
  
  if (unzGoToFirstFile(*zip) != UNZ_OK)
  {
    EH_Error(EH_SEVERE, 
       "LoadLbgData: Could not go to first file when loading file %s.", 
       filename);
    return(NULL);
  }

  do
  {
    zipfilename[0] = 0;
    if (unzGetCurrentFileInfo(*zip, &zinfo, zipfilename, MAX_PATH, 
                              NULL, 0, NULL, 0) != UNZ_OK)
        continue;
    if (!strcasecmp(filename, zipfilename))
        found = 1;
  }
  while (!found && (unzGoToNextFile(*zip) == UNZ_OK));
  if (!found || (unzOpenCurrentFile(*zip) != UNZ_OK))
  {
    EH_Error(EH_SEVERE, 
             "LoadLbgData: Could not load file %s.", 
             filename);
     return(NULL);
  }
  
  data = (unsigned char *) malloc(zinfo.uncompressed_size);
  if (unzReadCurrentFile(*zip, data, zinfo.uncompressed_size) != 
      zinfo.uncompressed_size)
      goto end_load;

  end_load:
  unzCloseCurrentFile(*zip);
  *size = zinfo.uncompressed_size;
  return(data);
}

