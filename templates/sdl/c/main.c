#include <SDL2/SDL.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

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

  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x80, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  SDL_Rect box;
  box.x = WIDTH/4;
  box.y = HEIGHT/4;
  box.w = WIDTH/2;
  box.h = HEIGHT/2;

  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x00, SDL_ALPHA_OPAQUE);
  SDL_RenderFillRect(renderer, &box);

  SDL_RenderPresent(renderer);

  SDL_Delay(5000);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
