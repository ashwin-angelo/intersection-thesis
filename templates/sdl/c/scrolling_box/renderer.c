#include "SDL_ttf.h"
#include <stdbool.h>
#include "renderer.h"

Renderer *createRenderer(uint32_t w, uint32_t h)
{

  // TODO: take param to name window?
  SDL_Window *sdl_window = SDL_CreateWindow("Test Window", 
    SDL_WINDOWPOS_CENTERED, 
    SDL_WINDOWPOS_CENTERED, 
    w, h, SDL_WINDOW_SHOWN);

  if(!sdl_window)
  {
    fprintf(stderr, "SDL_CreateWindow FAILED: %s\n", SDL_GetError());
    return NULL;
  }

  SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 
    SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

  if(!sdl_renderer)
  {
    fprintf(stderr, "SDL_CreateRenderer FAILED: %s\n", SDL_GetError());
    SDL_DestroyWindow(sdl_window);
    return NULL;
  }

  Renderer *ren = malloc(sizeof(Renderer));
  ren->renderer = sdl_renderer;
  ren->window = sdl_window;
  ren->window_width = w;
  ren->window_height = h;
  ren->ttf = false;
  return ren;
}

void destroyRenderer(Renderer *ren)
{
  if(ren->window)   SDL_DestroyWindow(ren->window);
  if(ren->renderer) SDL_DestroyRenderer(ren->renderer);
  if(ren->ttf)      TTF_Quit();

  free(ren);
}

void setRenderDrawColor(uint8_t r, uint8_t g, uint8_t b, Renderer *ren)
{
  SDL_SetRenderDrawColor(ren->renderer, r, g, b, SDL_ALPHA_OPAQUE);
}

void clear(Renderer *ren)
{
  SDL_RenderClear(ren->renderer);
}

void drawNode(SDL_Rect *box, SDL_FPoint *node, SDL_FPoint *view, float pixels_per_unit, Renderer *ren)
{
  box->x = (node->x - view->x) * pixels_per_unit - box->w / 2;
  box->y = (node->y - view->y) * pixels_per_unit - box->h / 2;
  SDL_SetRenderDrawColor(ren->renderer, 0xff, 0x00, 0x40, SDL_ALPHA_OPAQUE);
  SDL_RenderFillRect(ren->renderer, box);
}

void drawLine(SDL_FPoint *start, SDL_FPoint *end, SDL_FPoint *view, float pixels_per_unit, Renderer *ren)
{
  SDL_Point startPixel, endPixel;

  startPixel.x = (start->x - view->x) * pixels_per_unit;
  startPixel.y = (start->y - view->y) * pixels_per_unit;

  endPixel.x = (end->x - view->x) * pixels_per_unit;
  endPixel.y = (end->y - view->y) * pixels_per_unit;

  SDL_SetRenderDrawColor(ren->renderer, 0x00, 0x00,  0xff, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawLine(ren->renderer, startPixel.x, startPixel.y, endPixel.x, endPixel.y);
}

// TODO: take param for custom fonts
void drawText(const char text[], SDL_Color text_color, int text_size, Renderer *ren)
{
  if(!ren->ttf)
  {
    if(TTF_Init() < 0)
    {
      fprintf(stderr, "TTF_Init() FAILED: %s\n", SDL_GetError());
      return;
    }
    else ren->ttf = true;
  }

  const char font_file[] = "ttf/IBMPlexMono/IBMPlexMono-Regular.ttf";
  TTF_Font *font = TTF_OpenFont(font_file, text_size);

  if(!font)
  {
    fprintf(stderr, "TTF_OpenFont(%s, %d) FAILED\n", font_file, text_size);
    return;
  }

  SDL_Surface *text_surface = TTF_RenderText_Blended(font, text, text_color);
  TTF_CloseFont(font);

  if(!text_surface)
  {
    fprintf(stderr, "TTF_RenderText_Shaded() FAILED\n");
    return;
  }

  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(ren->renderer, text_surface);
  SDL_FreeSurface(text_surface);

  if(!text_texture)
  {
    fprintf(stderr, "SDL_CreateTextureFromSurface() FAILED: %s\n", SDL_GetError());
    return;
  }

  SDL_Rect text_rect;
  text_rect.x = (ren->window_width  - text_surface->w) / 2;
  text_rect.y = (ren->window_height - text_surface->h) / 2;
  text_rect.w = text_surface->w;
  text_rect.h = text_surface->h;

  SDL_RenderCopy(ren->renderer, text_texture, NULL, &text_rect);
  SDL_DestroyTexture(text_texture);
}

void display(Renderer* ren)
{
  SDL_RenderPresent(ren->renderer);
}
