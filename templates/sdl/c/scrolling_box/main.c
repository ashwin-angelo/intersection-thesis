#include <SDL2/SDL.h>
#include "SDL_ttf.h"

#include <stdio.h>
#include <stdbool.h>

#define WIDTH 1000
#define HEIGHT 800

#define MAX_SCALE 10.0f
#define MIN_SCALE 0.1f

/* PIXELS PER "UNIT" */
#define BASE_PPU 100.0f

void clean_up(bool sdl, bool ttf, SDL_Window *window, SDL_Renderer *renderer)
{
  if(renderer)  SDL_DestroyRenderer(renderer);
  if(window)    SDL_DestroyWindow(window);
  if(ttf)       TTF_Quit();
  if(sdl)       SDL_Quit();
}

int main (int argc, char **argv)
{
  if(SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    fprintf(stderr, "SDL_Init(SDL_INIT_VIDEO) FAILED: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Test Window", 
    SDL_WINDOWPOS_CENTERED, 
    SDL_WINDOWPOS_CENTERED, 
    WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

  if(!window)
  {
    fprintf(stderr, "SDL_CreateWindow FAILED: %s\n", SDL_GetError());
    clean_up(true, false, NULL, NULL);
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
    SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

  if(!renderer)
  {
    fprintf(stderr, "SDL_CreateRenderer FAILED: %s\n", SDL_GetError());
    clean_up(true, false, NULL, NULL);
    return 1;
  }

  if(TTF_Init() < 0)
  {
    fprintf(stderr, "TTF_Init() FAILED: %s\n", SDL_GetError());
    clean_up(true, false, window, renderer);
    return 1;
  }

  const char font_file[] = "ttf/IBMPlexMono/IBMPlexMono-Regular.ttf";
  int ptsize = HEIGHT / 20;
  TTF_Font *font = TTF_OpenFont(font_file, ptsize);

  if(!font)
  {
    clean_up(true, true, window, renderer);
    fprintf(stderr, "TTF_OpenFont(%s, %d) FAILED\n", font_file, ptsize);
    return 1;
  }

  SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };
  SDL_Surface *text_surface = TTF_RenderText_Blended(font, "test text", text_color);
  TTF_CloseFont(font);

  if(!text_surface)
  {
    clean_up(true, true, window, renderer);
    fprintf(stderr, "TTF_RenderText_Shaded() FAILED\n");
    return 1;
  }

  SDL_Rect text_rect;
  text_rect.x = (WIDTH  - text_surface->w) / 2;
  text_rect.y = (HEIGHT - text_surface->h) / 2;
  text_rect.w = text_surface->w;
  text_rect.h = text_surface->h;

  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_FreeSurface(text_surface);

  if(!text_texture)
  {
    clean_up(true, true, window, renderer);
    fprintf(stderr, "SDL_CreateTextureFromSurface() FAILED: %s\n", SDL_GetError());
    return 1;
  }

  bool quit  = false;

  float scale = 1.0f;
  float pixels_per_unit = BASE_PPU * scale;

  SDL_FPoint focus, view;
  focus.x = focus.y = 0.0f;
  view.x = focus.x - ( WIDTH  / 2.0f ) / pixels_per_unit;
  view.y = focus.y - ( HEIGHT / 2.0f ) / pixels_per_unit;

  SDL_Point mouse, motion;
  mouse.x = mouse.y = motion.x = motion.y = 0;
  float scroll_y  = 0;

  SDL_Rect box;
  float box_x = -2.0f;
  float box_y = -2.0f;
  float box_w = 4.0f;
  float box_h = 4.0f;

  while(true)
  {
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
      switch(event.type)
      {
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym)
          {
            case SDLK_k:
              printf("K\n");
              break;

            default:
              printf("Quit\n");
              quit = true;
              break;
          }
          break;

        case SDL_MOUSEMOTION:
          if(event.motion.state)
          {
            motion.x = event.motion.xrel;
            motion.y = event.motion.yrel;
          }
          break;

        case SDL_MOUSEWHEEL:
          scroll_y = event.wheel.preciseY;
          mouse.x  = event.wheel.mouseX;
          mouse.y  = event.wheel.mouseY;
          break;

        case SDL_QUIT:
          quit = true;
          break;

        default:
          break;
      }
    }

    if(motion.x || motion.y)
    {
      printf("Dragging: motion.x: %d\tmotion.y: %d\n", motion.x, motion.y);
      focus.x -= motion.x / pixels_per_unit;
      focus.y -= motion.y / pixels_per_unit;
      view.x = focus.x - ( WIDTH  / 2.0f ) / pixels_per_unit;
      view.y = focus.y - ( HEIGHT / 2.0f ) / pixels_per_unit;
      motion.x = motion.y = 0;
    }

    if(scroll_y)
    {
      printf("Scrolling (%d)\tmouse.x: %d\tmouse.y: %d\n", (int)scroll_y, mouse.x, mouse.y);

      if(scroll_y < 0.0f && scale * 0.8f > MIN_SCALE) scale *= 0.8f;
      else if (scroll_y > 0.0f && scale * 1.25f < MAX_SCALE) scale *= 1.25f;

      scroll_y = 0.0f;

      SDL_FPoint cursor;
      cursor.x = mouse.x / pixels_per_unit + view.x;
      cursor.y = mouse.y / pixels_per_unit + view.y;

      pixels_per_unit = BASE_PPU * scale;

      view.x = cursor.x - mouse.x / pixels_per_unit;
      view.y = cursor.y - mouse.y / pixels_per_unit;
      focus.x = view.x + (WIDTH  / 2.0f) / pixels_per_unit;
      focus.y = view.y + (HEIGHT / 2.0f) / pixels_per_unit;
    }

    box.x = (box_x - view.x) * pixels_per_unit;
    box.y = (box_y - view.y) * pixels_per_unit;
    box.w = box_w * pixels_per_unit;
    box.h = box_h * pixels_per_unit;

    SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0x40, 0x40, 0x40, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &box);

    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

    SDL_RenderPresent(renderer);

    if(quit) break;
  }

  clean_up(true, true, window, renderer);
  return 0;
}
