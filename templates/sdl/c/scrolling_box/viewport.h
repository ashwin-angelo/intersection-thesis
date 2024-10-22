#pragma once

#include <SDL2/SDL.h>

typedef struct Viewport {

  uint32_t width, height;
  float scale, pixels_per_unit, base_ppu;
  SDL_FPoint focus, view;

} Viewport;

struct Viewport *createViewport(uint32_t w, uint32_t h, float base_ppu);
void destroyViewport(Viewport *vw);

void handleMotion(SDL_Point motion, Viewport *vw);
void handleScroll(float scroll_y, SDL_Point mouse, float minScale, float maxScale, Viewport *vw);
