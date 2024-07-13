#include "renderer.h"

void display(Renderer* renderer)
{
  SDL_RenderPresent(renderer->renderer);
}

void draw_node(SDL_Rect *box, SDL_FPoint *node, SDL_FPoint *view, float pixels_per_unit, Renderer *renderer)
{
    box->x = (node->x - view->x) * pixels_per_unit;
    box->y = (node->y - view->y) * pixels_per_unit;
    SDL_SetRenderDrawColor(renderer->renderer, 0xff, 0x00, 0x40, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer->renderer, box);
}
