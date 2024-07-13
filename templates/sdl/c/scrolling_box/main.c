#include <SDL2/SDL.h>
#include "SDL_ttf.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "renderer.h"

#define WIDTH 1000
#define HEIGHT 800

#define MAX_SCALE 100000.0f
#define MIN_SCALE 0.00001f

// PIXELS PER UNIT
#define BASE_PPU 100.0f

// radius of earth in miles
#define R 3958.8

#define MAX_NODES 64

float rad(float deg)
{
  return (deg * M_PI) / 180.0f;
}

/* SIMPLE EQUIRECTANGULAR PROJECTION
  x = radius_earth * longitude * cos( average latitude )
  y = radius_earth * latitude
  ( latitude and longitude must be converted to radians ) */

void lat_lon_to_pt(float lat, float lon, SDL_FPoint *p, float aspect_ratio)
{
  p->x = R * rad(lon) * aspect_ratio; 
  p->y = R * rad(lat) * -1.0f;
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

  // extract nodes
  SDL_FPoint lat_lon_pairs[MAX_NODES];
  SDL_FPoint nodes[MAX_NODES];
  size_t number_of_nodes = 0;
  float sum_of_lats = 0.0f;

  const char nodes_filename[] = "nodes.txt";
  FILE* nodes_file = fopen(nodes_filename, "r");
  if (!nodes_file)
  {
    clean_up(true, true, window, renderer);
    fprintf(stderr, "Failed to open file: %s\n", nodes_filename);
    return 1;
  }
  char* line = NULL;
  size_t len = 0;

  while( getline(&line, &len, nodes_file) != -1)
  {
    // TODO: error check ... and eventually make more flexible

    printf("%s", line);
    char * token;
    token = strtok(line," ,\n");
    float lat = atof(token);
    token = strtok(NULL, " ,\n");
    float lon = atof(token);

    lat_lon_pairs[number_of_nodes].x = lat;
    lat_lon_pairs[number_of_nodes].y = lon;
    sum_of_lats += lat;
    number_of_nodes++;
  }
  
  fclose(nodes_file);
  free(line);

  float avg_lat = sum_of_lats / number_of_nodes;
  float aspect_ratio = cos( rad(avg_lat) );

  for(int i = 0; i < number_of_nodes; i++)
    lat_lon_to_pt(lat_lon_pairs[i].x, lat_lon_pairs[i].y, &nodes[i], aspect_ratio);

  SDL_FRect start_box;
  SDL_EncloseFPoints(nodes, 4, NULL, &start_box);
  
  /* THE W/H ARE OFF BY ONE DUE TO A BUG IN EncloseFPoints */
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

    box.w = 10;
    box.h = 10;

    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

    Renderer ren;
    ren.renderer = renderer;

    for(int i = 0; i < number_of_nodes; i++)
    {
      draw_node(&box, &nodes[i], &view, pixels_per_unit, &ren);
    }

    display(&ren);

    if(quit) break;
  }

  clean_up(true, true, window, renderer);
  return 0;
}
