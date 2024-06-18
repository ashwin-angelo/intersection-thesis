#include <SDL2/SDL.h>
#include "SDL_ttf.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define WIDTH 1000
#define HEIGHT 800

#define MAX_SCALE 100000.0f
#define MIN_SCALE 0.00001f

// PIXELS PER UNIT
#define BASE_PPU 100.0f

float rad(float deg)
{
  return (deg * M_PI) / 180.0f;
}

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


//////////////////////////////////////////////////////////////////////
/*

  SIMPLE EQUIRECTANGULAR PROJECTION

  x = radius_earth * longitude * cos( average latitude )
  y = radius_earth * latitude

  latitude and longitude must be converted to radians

*/
//////////////////////////////////////////////////////////////////////

  /* NORTHEAST CORNER */
  float ne_lat = 40.693262476072995;
  float ne_lon = -73.97384719510481;

  /* SOUTHEAST CORNER */
  float se_lat = 40.68961102229747;
  float se_lon = -73.97313177869177;

  /* SOUTHWEST CORNER */
  float sw_lat = 40.689845032051686;
  float sw_lon = -73.97863436726685;

  /* NORTHWEST CORNER */
  float nw_lat = 40.69349946108703; 
  float nw_lon = -73.97915310771947;

  float avg_lat = (nw_lat + se_lat) / 2;
  float r = 3958.8; // radius of earth in miles

  float a = cos( rad(avg_lat) );
  
  printf("a: %f\n", a);

  SDL_FPoint NE, NW, SE, SW;

  NE.x = r * rad(ne_lon) * a; 
  NE.y = r * rad(ne_lat) * -1.0f;
  printf("NE.x: %f NE.y: %f\n", NE.x, NE.y);

  NW.x = r * rad(nw_lon) * a; 
  NW.y = r * rad(nw_lat) * -1.0f;
  printf("NW.x: %f NW.y: %f\n", NW.x, NW.y);

  SE.x = r * rad(se_lon) * a; 
  SE.y = r * rad(se_lat) * -1.0f;
  printf("SE.x: %f SE.y: %f\n", SE.x, SE.y);

  SW.x = r * rad(sw_lon) * a; 
  SW.y = r * rad(sw_lat) * -1.0f;
  printf("SW.x: %f SW.y: %f\n", SW.x, SW.y);

  SDL_FPoint nodes[4];
  nodes[0] = NE;
  nodes[1] = NW;
  nodes[2] = SE;
  nodes[3] = SW;

  SDL_FRect start_box;
  SDL_EncloseFPoints(nodes, 4, NULL, &start_box);

  /* THE w/h ARE OFF BY ONE DUE TO A BUG IN EncloseFPoints */
  start_box.w -= 1;
  start_box.h -= 1;

  printf("start_x = %f\tstart_y = %f\tstart_w = %f\tstart_h = %f\n",
    start_box.x, start_box.y, start_box.w, start_box.h);
  
  focus.x = start_box.x + start_box.w / 2;
  focus.y = start_box.y + start_box.h / 2;

  float desired_x_ppu = WIDTH  / (start_box.w * 1.1);
  float desired_y_ppu = HEIGHT / (start_box.h * 1.1);

  float desired_ppu = desired_x_ppu;
  if(desired_y_ppu > desired_ppu) desired_ppu = desired_y_ppu;

  pixels_per_unit = desired_ppu;
  scale = pixels_per_unit / BASE_PPU;
  view.x = focus.x - ( WIDTH  / 2.0f ) / pixels_per_unit;
  view.y = focus.y - ( HEIGHT / 2.0f ) / pixels_per_unit;

  printf("PPU: %f\n", pixels_per_unit);
  printf("scale: %f\n", scale);
  printf("focus.x: %f\tfocus.y: %f\n", focus.x, focus.y);
  printf("view.x: %f\tview.y: %f\n", view.x, view.y);

  SDL_Rect box;
  float box_x = (focus.x + view.x) / 2;
  float box_y = (focus.y + view.y) / 2;
  printf("box_x: %f\tbox_y: %f\n", box_x, box_y);
  float box_w = (focus.x - view.x);
  float box_h = (focus.y - view.y);

//////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////

    box.w = 10;
    box.h = 10;

    box.x = (NE.x - view.x) * pixels_per_unit;
    box.y = (NE.y - view.y) * pixels_per_unit;
    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x40, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &box);

    box.x = (NW.x - view.x) * pixels_per_unit;
    box.y = (NW.y - view.y) * pixels_per_unit;
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x40, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &box);

    box.x = (SE.x - view.x) * pixels_per_unit;
    box.y = (SE.y - view.y) * pixels_per_unit;
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &box);

    box.x = (SW.x - view.x) * pixels_per_unit;
    box.y = (SW.y - view.y) * pixels_per_unit;
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &box);

/////////////////////////////////////////////////////////


    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

    SDL_RenderPresent(renderer);

    if(quit) break;
  }

  clean_up(true, true, window, renderer);
  return 0;
}
