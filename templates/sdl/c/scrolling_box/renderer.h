#pragma once

#include <SDL2/SDL.h>

typedef struct Renderer {
  SDL_Renderer *renderer;
} Renderer;

struct Renderer *createRenderer(SDL_Window *window);
void destroyRenderer(Renderer *ren);

void set_render_draw_color(uint8_t r, uint8_t g, uint8_t b, Renderer *ren);
void clear(Renderer *ren);

void draw_node(SDL_Rect *box, SDL_FPoint *node, SDL_FPoint *view, float pixels_per_unit, Renderer *ren);
void draw_line(SDL_FPoint *start, SDL_FPoint *end, SDL_FPoint *view, float pixels_per_unit, Renderer *ren);

void display(Renderer *ren);
