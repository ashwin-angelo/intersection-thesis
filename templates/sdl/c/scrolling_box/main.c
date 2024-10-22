#include <SDL2/SDL.h>
#include "SDL_ttf.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "renderer.h"
#include "viewport.h"

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

// void clean_up(bool sdl, bool ttf, SDL_Window *window, SDL_Renderer *renderer)
void clean_up(bool sdl, bool ttf, SDL_Window *window, Renderer *renderer)
{
  if(renderer)  destroyRenderer(renderer);
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

  Renderer *renderer = createRenderer(window);
  if(!renderer)
  {
    clean_up(true, false, NULL, NULL);
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

  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer->renderer, text_surface);
  SDL_FreeSurface(text_surface);

  if(!text_texture)
  {
    clean_up(true, true, window, renderer);
    fprintf(stderr, "SDL_CreateTextureFromSurface() FAILED: %s\n", SDL_GetError());
    return 1;
  }

  bool quit  = false;

  Viewport *vw = createViewport(WIDTH, HEIGHT, BASE_PPU);

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
    // NOTE: no I do not think I'll be able to remember how strtok works

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
  
  vw->focus.x = start_box.x + start_box.w / 2;
  vw->focus.y = start_box.y + start_box.h / 2;

  // im not even sure this logic works for all scenarios
  // like if theres one extreme point off the map somewhere...
  float desired_x_ppu = vw->width  / (start_box.w * 1.5);
  float desired_y_ppu = vw->height / (start_box.h * 1.5);

  float desired_ppu = desired_x_ppu;
  if(desired_y_ppu > desired_ppu) desired_ppu = desired_y_ppu;

  vw->pixels_per_unit = desired_ppu;
  vw->scale = vw->pixels_per_unit / BASE_PPU;
  vw->view.x = vw->focus.x - ( WIDTH  / 2.0f ) / vw->pixels_per_unit;
  vw->view.y = vw->focus.y - ( HEIGHT / 2.0f ) / vw->pixels_per_unit;

  SDL_Rect box;

  float box_x = (vw->focus.x + vw->view.x) / 2;
  float box_y = (vw->focus.y + vw->view.y) / 2;
  printf("box_x: %f\tbox_y: %f\n", box_x, box_y);
  float box_w = (vw->focus.x - vw->view.x);
  float box_h = (vw->focus.y - vw->view.y);

  SDL_Rect text_rect;
  text_rect.x = (WIDTH  - text_surface->w) / 2;
  text_rect.y = (HEIGHT - text_surface->h) / 2;
  text_rect.w = text_surface->w;
  text_rect.h = text_surface->h;

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
      handleMotion(motion, vw);
      motion.x = motion.y = 0;
    }

    if(scroll_y)
    {
      printf("Scrolling (%d)\tmouse.x: %d\tmouse.y: %d\n", (int)scroll_y, mouse.x, mouse.y);
      handleScroll(scroll_y, mouse, MIN_SCALE, MAX_SCALE, vw);
      scroll_y = 0.0f;
    }

    box.x = (box_x - vw->view.x) * vw->pixels_per_unit;
    box.y = (box_y - vw->view.y) * vw->pixels_per_unit;
    box.w = box_w * vw->pixels_per_unit;
    box.h = box_h * vw->pixels_per_unit;

    set_render_draw_color(0x20, 0x20, 0x20, renderer);
    clear(renderer);

    set_render_draw_color(0x40, 0x40, 0x40, renderer);
    SDL_RenderFillRect(renderer->renderer, &box);

    box.w = 10;
    box.h = 10;

    SDL_RenderCopy(renderer->renderer, text_texture, NULL, &text_rect);

    for(int i = 0; i < number_of_nodes; i++)
    {
      draw_node(&box, &nodes[i], &(vw->view), vw->pixels_per_unit, renderer);
      if(i > 0 && i < number_of_nodes) draw_line(&nodes[i-1], &nodes[i], &(vw->view), vw->pixels_per_unit, renderer);
    }

    display(renderer);

    if(quit) break;
  }

  clean_up(true, true, window, renderer);

  destroyViewport(vw);

  return 0;
}
