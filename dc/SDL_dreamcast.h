#ifndef GPSPDC_SDL_DREAMCAST_COMPAT_H
#define GPSPDC_SDL_DREAMCAST_COMPAT_H

#include <SDL.h>

#define SDL_DC_TEXTURED_VIDEO 0
#define SDL_DC_DMA_VIDEO 0

#define SDL_DC_LEFT 0
#define SDL_DC_RIGHT 1
#define SDL_DC_UP 2
#define SDL_DC_DOWN 3
#define SDL_DC_START 4
#define SDL_DC_A 5
#define SDL_DC_B 6
#define SDL_DC_X 7
#define SDL_DC_Y 8

#define SDL_FALSE 0
#define SDL_TRUE 1

static inline void SDL_DC_VerticalWait(int enabled)
{
  (void)enabled;
}

static inline void SDL_DC_MapKey(int port, int button, SDLKey key)
{
  (void)port;
  (void)button;
  (void)key;
}

static inline void SDL_DC_SetVideoDriver(int driver)
{
  (void)driver;
}

static inline void SDL_DC_SetWindow(int width, int height)
{
  (void)width;
  (void)height;
}

#endif
