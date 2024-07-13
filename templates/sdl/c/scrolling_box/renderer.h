#include <SDL2/SDL.h>

typedef struct Renderer {
  SDL_Renderer *renderer;
} Renderer;

void draw_node(SDL_Rect *box, SDL_FPoint *node, SDL_FPoint *view, float pixels_per_unit, Renderer *renderer);

void display(Renderer* renderer);
