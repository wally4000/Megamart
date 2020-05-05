TARGET = MegaMart
OBJS =  main.o hero_manager.o sprite_manager.o map_manager.o bg_manager.o power_manager.o menu_manager.o
OBJS += zip_manager.o unzip.o ioapi.o resource_manager.o dl_manager.o sce_graphics.o eh_manager.o cc_manager.o


INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
SDL_CONFIG = $(PSPBIN)/sdl-config

LIBDIR =
LDFLAGS =

BUILD_PRX=1s
EXTRA_TARGETS = EBOOT.PBP
EXTRA_CLEAN = my-clean

PSP_EBOOT_TITLE = Mega Mart
PSP_DIR_NAME = mm

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
CFLAGS += `$(PSPBIN)/sdl-config --cflags`
CFLAGS += -I. -Wall -g3 -O3 -G0 -DNOUNCRYPT -DPSP_FW_VERSION=661

LIBS = `$(PSPBIN)/sdl-config --libs` -lm -lSDL_ttf -lfreetype -lSDL_gfx -lSDL_image -lSDL_mixer -lmikmod -lpng -lz -lm -ljpeg -lpspwlan -lpspgu -lpsppower
LIBS += $(shell $(SDL_CONFIG) --libs)
include $(PSPSDK)/lib/build.mak

# Use "make pc" to build a copy of sdltest that runs on your PC.
pc:
	gcc -o sdltest-pc sdltest.c `sdl-config --cflags` `sdl-config --libs`

my-clean:
	rm -rf $(TARGET) $(TARGET)% sdltest-pc
