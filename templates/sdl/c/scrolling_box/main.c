#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "renderer.h"
#include "viewport.h"
#include "scrollmap.h"

#define WIDTH 1000
#define HEIGHT 800

#define MAX_SCALE 100000.0f
#define MIN_SCALE 0.00001f

// PIXELS PER UNIT
#define BASE_PPU 100.0f

int main(int argc, char **argv)
{
  if(SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    fprintf(stderr, "SDL_Init(SDL_INIT_VIDEO) FAILED: %s\n", SDL_GetError());
    return 1;
  }

  Renderer *renderer = createRenderer(WIDTH, HEIGHT);
  if(!renderer) SDL_Quit();

  ScrollMap *sm = createScrollMap(WIDTH, HEIGHT, BASE_PPU, "nodes.txt");
  if(!sm)
  {
    destroyRenderer(renderer);
    SDL_Quit();
  }

  SDL_Rect box;

  float box_x = (sm->vw->focus.x + sm->vw->view.x) / 2;
  float box_y = (sm->vw->focus.y + sm->vw->view.y) / 2;
  printf("box_x: %f\tbox_y: %f\n", box_x, box_y);
  float box_w = (sm->vw->focus.x - sm->vw->view.x);
  float box_h = (sm->vw->focus.y - sm->vw->view.y);

  bool quit  = false;
  SDL_Point mouse, motion;
  mouse.x = mouse.y = motion.x = motion.y = 0;
  float scroll_y  = 0;

////////////////////////////// LOOP /////////////////////////////////

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
      // TODO: not as fluid as it could be with this strategy ...
      handleMotion(motion, sm->vw);
      motion.x = motion.y = 0;
    }

    if(scroll_y)
    {
      printf("Scrolling (%d)\tmouse.x: %d\tmouse.y: %d\n", (int)scroll_y, mouse.x, mouse.y);
      handleScroll(scroll_y, mouse, MIN_SCALE, MAX_SCALE, sm->vw);
      scroll_y = 0.0f;
    }

    // Clear the screen
    setRenderDrawColor(0x20, 0x20, 0x20, renderer);
    clear(renderer);

    // Draw this box that scales w/ zoom
    box.x = (box_x - sm->vw->view.x) * sm->vw->pixels_per_unit;
    box.y = (box_y - sm->vw->view.y) * sm->vw->pixels_per_unit;
    box.w = box_w * sm->vw->pixels_per_unit;
    box.h = box_h * sm->vw->pixels_per_unit;
    setRenderDrawColor(0x40, 0x40, 0x40, renderer);
    SDL_RenderFillRect(renderer->renderer, &box);

    // Draw this static overlayed text
    SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };
    int text_size = renderer->window_height / 10;
    drawText("sample text", text_color, text_size, renderer);

    // Draw fixed size node markers on the map and connect with lines
    box.w = box.h = 10;
    for(int i = 0; i < sm->number_of_nodes; i++)
    {
      drawNode(&box, &(sm->nodes[i]), &(sm->vw->view), sm->vw->pixels_per_unit, renderer);
      if(i > 0 && i < sm->number_of_nodes) drawLine(&(sm->nodes[i-1]), &(sm->nodes[i]), &(sm->vw->view), sm->vw->pixels_per_unit, renderer);
    }

    display(renderer);
    if(quit) break;
  }

  destroyScrollMap(sm);
  destroyRenderer(renderer);
  SDL_Quit();

  return 0;
}
