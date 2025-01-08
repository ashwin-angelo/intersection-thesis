#pragma once

#include <SDL2/SDL.h>
#include "viewport.h"

#define MAX_MAP_NODES 64

typedef struct ScrollMap {
  Viewport *vw;
  SDL_FPoint nodes[MAX_MAP_NODES];
  size_t number_of_nodes;
} ScrollMap;

ScrollMap *createScrollMap(uint32_t w, uint32_t h, float base_ppu, const char nodes_filename[]);
void destroyScrollMap(ScrollMap *sm);

float rad(float deg);
void latLonToPt(float lat, float lon, SDL_FPoint *p, float aspect_ratio);
void loadNodesFromFile(FILE* nodes_file, ScrollMap* sm);
void centerViewport(Viewport *vw, ScrollMap *sm, int w, int h, float base_ppu);
