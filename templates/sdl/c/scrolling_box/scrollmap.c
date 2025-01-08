#include "scrollmap.h"
#include <stdio.h>
#include <math.h>

// radius of earth in miles
#define RADIUS_EARTH 3958.8

float rad(float deg) { return (deg * M_PI) / 180.0f; }

/* SIMPLE EQUIRECTANGULAR PROJECTION
  x = radius_earth * longitude * cos( average latitude )
  y = radius_earth * latitude
  ( latitude and longitude must be converted to radians ) */

void latLonToPt(float lat, float lon, SDL_FPoint *p, float aspect_ratio)
{
  p->x = RADIUS_EARTH * rad(lon) * aspect_ratio;
  p->y = RADIUS_EARTH * rad(lat) * -1.0f;
}

// TODO: error check ... and eventually make more flexible
void loadNodesFromFile(FILE* nodes_file, ScrollMap* sm)
{
  SDL_FPoint lat_lon_pairs[MAX_MAP_NODES];
  float sum_of_lats = 0.0f;
  char* line = NULL;
  size_t len = 0;

  while( getline(&line, &len, nodes_file) != -1)
  {
    printf("%s", line);
    char * token = strtok(line," ,\n");
    float lat = atof(token);
    token = strtok(NULL, " ,\n");
    float lon = atof(token);
    lat_lon_pairs[sm->number_of_nodes].x = lat;
    lat_lon_pairs[sm->number_of_nodes].y = lon;
    sum_of_lats += lat;
    sm->number_of_nodes++;
  }

  free(line);

  float avg_lat = sum_of_lats / sm->number_of_nodes;
  float aspect_ratio = cos( rad(avg_lat) );

  // convert lat lon pairs to 2D points
  for(int i = 0; i < sm->number_of_nodes; i++)
    latLonToPt(lat_lon_pairs[i].x, lat_lon_pairs[i].y, &sm->nodes[i], aspect_ratio);
}

void centerViewport(Viewport *vw, ScrollMap *sm, int w, int h, float base_ppu)
{
  SDL_FRect start_box;
  SDL_EncloseFPoints(sm->nodes, 4, NULL, &start_box);
 
  // THE W/H ARE OFF BY ONE DUE TO A BUG IN EncloseFPoints
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
  vw->scale = vw->pixels_per_unit / base_ppu;
  vw->view.x = vw->focus.x - ( w / 2.0f ) / vw->pixels_per_unit;
  vw->view.y = vw->focus.y - ( h / 2.0f ) / vw->pixels_per_unit;
}

ScrollMap *createScrollMap(uint32_t w, uint32_t h, float base_ppu, const char nodes_filename[])
{

  Viewport *vw = createViewport(w, h, base_ppu);
  if(!vw) return NULL; // TODO: error message?

  ScrollMap *sm = malloc(sizeof(ScrollMap));
  if(!sm)
  {
    // TODO: error message?
    destroyViewport(vw);
    return NULL;
  }

  FILE* nodes_file = fopen(nodes_filename, "r");
  if (!nodes_file)
  {
    destroyViewport(vw);
    destroyScrollMap(sm);
    fprintf(stderr, "Failed to open file: %s\n", nodes_filename);
    return NULL;
  }

  loadNodesFromFile(nodes_file, sm);
  fclose(nodes_file);

  centerViewport(vw, sm, w, h, base_ppu);
  sm->vw = vw;
  return sm;
}

void destroyScrollMap(ScrollMap *sm)
{
  if(sm->vw) destroyViewport(sm->vw);
  free(sm);
}
