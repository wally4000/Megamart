//-----------------------------------------------------------------------------
//  Class:
//  SCE Graphics
//
//  Description:
//  This class uses the GU to draw graphics to the screen.  It is mainly used 
//  to draw the Background to the screen, and gives quite a performance boost
//  when doing so.
//-----------------------------------------------------------------------------

#include <pspgu.h>
#include "sce_graphics.h"
#include "zip_manager.h"


// private data
static int _vramBase;
static int _vRamOffset;
static SDL_Surface *_scr;

//------------------------------------------------------------------------------
// Name:     SCE_Init
// Summary:  Called 1X, initialses SCE Graphics for use
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void SCE_Init()
{
  // Set global pointer to screen
  _scr = MM_GetScreenPtr();
  
  // get the address of video memory
  _vramBase   = (void *)sceGeEdramGetAddr();
  
  // set the initial offset to start after the 2 full screen buffers used
  // for drawing data to the screen
  _vRamOffset = (512 * 272 * 2) * 2;
}

//------------------------------------------------------------------------------
// Name:     SCE_BlitSurface
// Summary:  Draws a surface to screen using SCE functions
// Inputs:   None
// Outputs:  None
// Returns:  None
// Cautions: None
//------------------------------------------------------------------------------
void SCE_BlitSurface(SDL_Surface* img, SDL_Rect *src, 
                     SDL_Surface *scr, SDL_Rect *dst)
{
  sceGuStart(GU_DIRECT, _SCEBgList);
  sceGuCopyImage(GU_PSM_5551, src->x, src->y, src->w, src->h, 
                 img->w, img->pixels, dst->x, dst->y, 512, scr->pixels);
  sceGuFinish();
  sceGuSync(0,0);
}

// NOTES: This function loads the follwoing images in a single file:
// bg1.bmp = walk_ceiling & loop_image. It also loads the 7 individual
// files rf1.bmp - rf7.bmp. The width and height of the 
// input image file must be a power of 2 to ensure it has the proper
// dimesnsions for use with sceGuCopyImage.  Also, this image will
// be stored in VRAM.  Finally, the data for this image must begin at
// a 16 byte boundry.  Since the VRAM offset should allready be 
// placed immediatly after the screen buffers, this should not be a
// problem.
//------------------------------------------------------------------------------
// Name:     SCE_LoadBackground1
// Summary:  Loads the first set of background images
// Inputs:   Name of file to load data from
// Outputs:  None
// Returns:  SDL_Surface containing image data
// Cautions: Dimension of image data in file must be a power of 2.  Image 
//           size can be bigger than 512x512.
//------------------------------------------------------------------------------
SDL_Surface* SCE_LoadBackground1(const char *file)
{
  SDL_Surface *tmp      = 0;
  SDL_Surface *sdlImg   = 0;
  SDL_Surface *vImg     = 0;
  void        *vImgData = 0;
  int         imgSize   = 0;
  
  // Load image
  tmp = ZIP_LoadImage(file); 
  
  // verify image loaded
  if (tmp)
  {
    // Convert image to screen format
    sdlImg = SDL_ConvertSurface(tmp, _scr->format, SDL_SWSURFACE);  
    
    // get size in bytes of entire image 
    // image height * image width * 2 (for 2 bytes per pixel)
    imgSize = (sdlImg->w * sdlImg->h)*2;
    
    // Ensure we do not exceed our 2 meg of VRAM, as long as this function
    // is called before SCE_LoadVramImage, this should never be a problem
    if ((_vRamOffset + imgSize) < 0x200000)
    {
      // Set our image data pointer to the correct area of VRAM
      vImgData = (void *) (_vramBase + _vRamOffset);
     
      // Copy the image into VRAM
      memcpy(vImgData, sdlImg->pixels, imgSize);
      
      // increment the offset into VRAM to include the memory we just used up
      _vRamOffset += imgSize;
      
      // Images printed using SCE function must be 16 byte alligned in memory.
      // The image passed in should have had a w/h that was a power of 2.  
      // Thus, the new VRAM offset after accounting for above image should be
      // on a 16 byte boundry.  If it is not, error as a precaution
      int byteAllign16  = _vRamOffset%16;
      if (byteAllign16 > 0)
      {
        EH_Error(EH_SEVERE, 
        "SCE_LoadBackground1 - Image not 16 byte alligned: %s, w=%i, h=%i, _vRamOffset=%i\n", 
        file, sdlImg->w, sdlImg->h, _vRamOffset);
      }
    }
    else
    {
      EH_Error(EH_SEVERE, 
      "SCE_LoadBackground1 - Not enough free VRAM: Image %s, w=%i, h=%i, _vRamOffset=%i\n", 
      file, sdlImg->w, sdlImg->h, _vRamOffset);
    }
  
    // Create an SDL surface using the image data generated above
    vImg = SDL_CreateRGBSurfaceFrom(vImgData, sdlImg->w, sdlImg->h, 
           _scr->format->BitsPerPixel, _scr->pitch, 
           _scr->format->Rmask, _scr->format->Gmask, 
           _scr->format->Bmask, _scr->format->Amask);
    
    SDL_FreeSurface(tmp);  
    SDL_FreeSurface(sdlImg);
  }
  return(vImg);
}

// NOTES: This function loads the Follwoing images in a single file:
// bg2.bmp = run_ceiling & walk_floor.  The width and height of the 
// input image file must be a power of 2 to ensure it has the proper
// dimesnsions for use with sceGuCopyImage.  Also note, when allocating
// the memory for this image, it must be 16 byte alligned for 
// compatibility with sceGuCopyImage.
//------------------------------------------------------------------------------
// Name:     SCE_LoadBackground2
// Summary:  Loads the second set of background images
// Inputs:   Name of file to load data from
// Outputs:  None
// Returns:  SDL_Surface containing image data
// Cautions: Dimension of image data in file must be a power of 2.  Image 
//           size can be bigger than 512x512.
//------------------------------------------------------------------------------
SDL_Surface* SCE_LoadBackground2(const char *file)
{
  SDL_Surface *tmp     = 0;
  SDL_Surface *sdlImg  = 0;
  SDL_Surface *retImg  = 0;
  void        *imgData = 0;
  int         imgSize  = 0;

  // Load Image,  NOTE: Image width and height must be a power of 2
  tmp = ZIP_LoadImage(file); 

  // verify image loaded
  if (tmp)
  {
    // convert image to screen format
    sdlImg = SDL_ConvertSurface(tmp, _scr->format, SDL_SWSURFACE);
    
    // get size in bytes of entire image 
    // image height * image width * 2 (for 2 bytes per pixel)
    imgSize = sdlImg->w * sdlImg->h * 2;
    
    // Create a block of 16 byte alligned memory to hold image
    imgData = (void *) memalign(16, imgSize);
    
    // Copy image data into 16 byte alligned memory
    memcpy(imgData, sdlImg->pixels, imgSize);
    
    // Create an SDL surface using the image data generated above
    retImg = SDL_CreateRGBSurfaceFrom(imgData, sdlImg->w, sdlImg->h, 
             sdlImg->format->BitsPerPixel, sdlImg->pitch, 
             sdlImg->format->Rmask, sdlImg->format->Gmask, 
             sdlImg->format->Bmask, sdlImg->format->Amask);
    
    // free temporary SDL surfaces
    SDL_FreeSurface(tmp);  
    SDL_FreeSurface(sdlImg);
  }
  
  // return newly created image data
  return(retImg);
}

// This function should only be called AFTER SCE_LoadBackground1 is called.
// This function uses the remaining free VRAM to create SDL surfaces for the
// given images.  
//------------------------------------------------------------------------------
// Name:     SCE_LoadVramImage
// Summary:  Loads the specified image into VRAM assuming there is enough
//           VRAM left to hold image.  This function is not actually used in
//           the game but can be used if it is ever needed.  It seemed like a 
//           good one to have around.  It was kept in the source rather than 
//           abandoned in case it is one day needed.
// Inputs:   1. Name of file to load
//           2. Format to load data (SDL NATIVE or SCREEN format)
//           3. Any flag that should be used on SDL_surface once loaded
// Outputs:  None
// Returns:  SDL_Surface containing image data
// Cautions: Dimension of image data in file must be a power of 2.  Image 
//           size can be bigger than 512x512.
//------------------------------------------------------------------------------
SDL_Surface* SCE_LoadVramImage(const char *file, 
                               unsigned int format, unsigned int flags)
{
  SDL_Surface *sdlImg = 0;
  SDL_Surface *retImg = 0;
  SDL_Surface *tmp    = 0;
  void *vImgData      = 0;
  int bytesPerPixel   = 0;
  int imgSize         = 0;
  
  // Load image
  sdlImg = ZIP_LoadImage(file); 

  // Verify image loaded
  if (sdlImg)
  {
    // convert to screen format
    if (format & SCE_SCREEN_FORMAT)   
    {
      tmp = SDL_ConvertSurface(sdlImg, _scr->format, flags);  
      SDL_FreeSurface(sdlImg);
      sdlImg = tmp;
      tmp = 0;
    }
    
    // Determine number of bytes per pixel
    if (sdlImg->format->BitsPerPixel == 15)
      bytesPerPixel = 2;
    else
      bytesPerPixel = sdlImg->format->BitsPerPixel / 8;
    
    // Get the total size in bytes of the current image
    imgSize = (sdlImg->w * sdlImg->h) * bytesPerPixel;
    
    // verify enough free vram exists to hold image
    if ((_vRamOffset + imgSize) < 0x200000)
    {
      // Set our image data pointer to the correct area of VRAM
      vImgData = (void *) (_vramBase + _vRamOffset);
     
      // Copy the image into VRAM
      // For some reason the bottom row of pixels is not being copiied.
      // copying 1 extra row of pixels seems to work for some reason.
      memcpy(vImgData, sdlImg->pixels, (imgSize+(sdlImg->w*2)));
      //memcpy(vImgData, sdlImg->pixels, imgSize);
      //src = (unsigned char *) sdlImg->pixels;
      //dst = (unsigned char *) vImgData;
      //for (x = 0; x < imgSize+(sdlImg->w*2); x++)
      //{
      //  dst[x] = src[x];  
      //}
      
      // increment the offset into VRAM to include the memory we just used up
      _vRamOffset  += imgSize;
    }
    // Error if enough free VRAM was not present
    else
    {
      EH_Error(EH_SEVERE, 
      "SCE_LoadVramImage - Not enough free VRAM: Image %s, w=%i, h=%i, _vRamOffset=%i\n", 
      file, sdlImg->w, sdlImg->h, _vRamOffset);
    }
    
    // Create a new SDL surface using the pixel data created above.  
    // the key here is that the pixel data was created in VRAM
    retImg = SDL_CreateRGBSurfaceFrom(
             vImgData, sdlImg->w, sdlImg->h, 
             sdlImg->format->BitsPerPixel, sdlImg->pitch, 
             sdlImg->format->Rmask, sdlImg->format->Gmask, 
             sdlImg->format->Bmask, sdlImg->format->Amask);
    
    // activate transparent background on return image
    if (format & SCE_TRANSP_FORMAT) 
      SDL_SetColorKey(retImg, SDL_SRCCOLORKEY, 
                      SDL_MapRGB(sdlImg->format, 0xFF, 0x80, 0x80));
      
    // Free the version of the image now residing in system memory...
    // system Memory, HA!
    SDL_FreeSurface(sdlImg);  
  }
  
  // return image created in VRAM  
  return(retImg);
}

