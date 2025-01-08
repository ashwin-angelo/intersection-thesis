#pragma once

#include <SDL2/SDL.h>

typedef struct Renderer {
  uint32_t window_width, window_height;
  SDL_Window *window;
  SDL_Renderer *renderer;
  bool ttf;
} Renderer;

Renderer *createRenderer(uint32_t w, uint32_t h);
void destroyRenderer(Renderer *ren);

void setRenderDrawColor(uint8_t r, uint8_t g, uint8_t b, Renderer *ren);
void clear(Renderer *ren);

void drawNode(SDL_Rect *box, SDL_FPoint *node, SDL_FPoint *view, float pixels_per_unit, Renderer *ren);
void drawLine(SDL_FPoint *start, SDL_FPoint *end, SDL_FPoint *view, float pixels_per_unit, Renderer *ren);
void drawText(const char text[], SDL_Color text_color, int text_size, Renderer *ren);

void display(Renderer *ren);
