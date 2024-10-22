#include "renderer.h"

struct Renderer *createRenderer(SDL_Window *window)
{
  SDL_Renderer *sdl_renderer = SDL_CreateRenderer(window, -1, 
    SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

  if(!sdl_renderer)
  {
    fprintf(stderr, "SDL_CreateRenderer FAILED: %s\n", SDL_GetError());
    return NULL;
  }

  Renderer *ren = malloc(sizeof(Renderer));
  ren->renderer = sdl_renderer;
  return ren;
}

void destroyRenderer(Renderer *ren)
{
  if(ren->renderer) SDL_DestroyRenderer(ren->renderer);
  free(ren);
}

void set_render_draw_color(uint8_t r, uint8_t g, uint8_t b, Renderer *ren)
{
  SDL_SetRenderDrawColor(ren->renderer, r, g, b, SDL_ALPHA_OPAQUE);
}

void clear(Renderer *ren)
{
  SDL_RenderClear(ren->renderer);
}

void draw_node(SDL_Rect *box, SDL_FPoint *node, SDL_FPoint *view, float pixels_per_unit, Renderer *ren)
{
  box->x = (node->x - view->x) * pixels_per_unit - box->w / 2;
  box->y = (node->y - view->y) * pixels_per_unit - box->h / 2;
  SDL_SetRenderDrawColor(ren->renderer, 0xff, 0x00, 0x40, SDL_ALPHA_OPAQUE);
  SDL_RenderFillRect(ren->renderer, box);
}

void draw_line(SDL_FPoint *start, SDL_FPoint *end, SDL_FPoint *view, float pixels_per_unit, Renderer *ren)
{
  SDL_Point startPixel, endPixel;

  startPixel.x = (start->x - view->x) * pixels_per_unit;
  startPixel.y = (start->y - view->y) * pixels_per_unit;

  endPixel.x = (end->x - view->x) * pixels_per_unit;
  endPixel.y = (end->y - view->y) * pixels_per_unit;

  SDL_SetRenderDrawColor(ren->renderer, 0x00, 0x00,  0xff, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawLine(ren->renderer, startPixel.x, startPixel.y, endPixel.x, endPixel.y);
}

void display(Renderer* ren)
{
  SDL_RenderPresent(ren->renderer);
}
