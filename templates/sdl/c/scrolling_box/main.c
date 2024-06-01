#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#define WIDTH 800
#define HEIGHT 800

#define MAX_SCALE 10.0f
#define MIN_SCALE 0.1f

/* PIXELS PER "UNIT" */
#define BASE_PPU 100.0f

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

  if(window == NULL)
  {
    fprintf(stderr, "SDL_CreateWindow FAILED: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
    SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

  if(renderer == NULL)
  {
    fprintf(stderr, "SDL_CreateRenderer FAILED: %s\n", SDL_GetError());
    return 1;
  }

  bool  quit  = false;

  float scale = 1.0f;
  float pixels_per_unit = BASE_PPU * scale;

  float focus_x = 0.0f;
  float focus_y = 0.0f;
  float view_x  = focus_x - ( WIDTH  / 2.0f ) / pixels_per_unit;
  float view_y  = focus_y - ( HEIGHT / 2.0f ) / pixels_per_unit;

  int   x_motion  = 0;
  int   y_motion  = 0;
  int   mouse_x   = 0;
  int   mouse_y   = 0;
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
      if(event.type == SDL_KEYDOWN)
      {
        if(event.key.keysym.sym == SDLK_k)
          printf("K\n");

        printf("Quit\n");
        quit = true;
      }
      else if(event.type == SDL_MOUSEMOTION)
      {
        if(event.motion.state)
        {
          x_motion = event.motion.xrel;
          y_motion = event.motion.yrel;
        }
      }
      else if(event.type == SDL_MOUSEWHEEL)
      {
        scroll_y = event.wheel.preciseY;
        mouse_x  = event.wheel.mouseX;
        mouse_y  = event.wheel.mouseY;
      }
      else if(event.type == SDL_QUIT)
      {
        printf("Quit\n");
        quit = true;
      }
    }

    if(x_motion || y_motion)
    {
      printf("Dragging: x_motion: %d\ty_motion: %d\n", x_motion, y_motion);

      focus_x -= x_motion / pixels_per_unit;
      focus_y -= y_motion / pixels_per_unit;

      view_x = focus_x - ( WIDTH  / 2.0f ) / pixels_per_unit;
      view_y = focus_y - ( HEIGHT / 2.0f ) / pixels_per_unit;

      x_motion = y_motion = 0;
    }

    if(scroll_y)
    {
      printf("scrolling (%d)\tmouse_x: %d\tmouse_y: %d\n", (int)scroll_y, mouse_x, mouse_y);

      if(scroll_y < 0.0f && scale * 0.8f > MIN_SCALE) scale *= 0.8f;
      else if (scroll_y > 0.0f && scale * 1.25f < MAX_SCALE) scale *= 1.25f;

      scroll_y = 0.0f;

      float cursor_x = mouse_x / pixels_per_unit + view_x;
      float cursor_y = mouse_y / pixels_per_unit + view_y;

      pixels_per_unit = BASE_PPU * scale;

      view_x = cursor_x - mouse_x / pixels_per_unit;
      view_y = cursor_y - mouse_y / pixels_per_unit;

      focus_x = view_x + (WIDTH  / 2.0f) / pixels_per_unit;
      focus_y = view_y + (HEIGHT / 2.0f) / pixels_per_unit;
    }

    box.x = (box_x - view_x) * pixels_per_unit;
    box.y = (box_y - view_y) * pixels_per_unit;
    box.w = box_w * pixels_per_unit;
    box.h = box_h * pixels_per_unit;

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x80, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &box);

    SDL_RenderPresent(renderer);

    if(quit) break;
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
