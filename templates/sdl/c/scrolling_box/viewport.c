#include "viewport.h"

Viewport *createViewport(uint32_t w, uint32_t h, float base_ppu)
{
  Viewport *vw = malloc(sizeof(Viewport));

  vw->width = w;
  vw->height = h;

  vw->scale = 1.0f;
  vw->base_ppu = base_ppu;
  vw->pixels_per_unit = vw->base_ppu * vw->scale;

  vw->focus.x = vw->focus.y = 0.0f;

  vw->view.x = vw->focus.x - ( vw->width  / 2.0f ) / vw->pixels_per_unit;
  vw->view.y = vw->focus.y - ( vw->height / 2.0f ) / vw->pixels_per_unit;
}

void destroyViewport(Viewport *vw)
{
  free(vw);
}

void handleMotion(SDL_Point motion, Viewport *vw)
{
  vw->focus.x -= motion.x / vw->pixels_per_unit;
  vw->focus.y -= motion.y / vw->pixels_per_unit;
  vw->view.x = vw->focus.x - ( vw->width  / 2.0f ) / vw->pixels_per_unit;
  vw->view.y = vw->focus.y - ( vw->height / 2.0f ) / vw->pixels_per_unit;
}

void handleScroll(float scroll_y, SDL_Point mouse, float minScale, float maxScale, Viewport *vw)
{
  // TODO: make the scroll factors constants
  if(scroll_y < 0.0f && vw->scale * 0.8f > minScale) vw->scale *= 0.8f;
  else if (scroll_y > 0.0f && vw->scale * 1.25f < maxScale) vw->scale *= 1.25f;
  else return;

  SDL_FPoint cursor;
  cursor.x = mouse.x / vw->pixels_per_unit + vw->view.x;
  cursor.y = mouse.y / vw->pixels_per_unit + vw->view.y;

  vw->pixels_per_unit = vw->base_ppu * vw->scale;

  vw->view.x = cursor.x - mouse.x / vw->pixels_per_unit;
  vw->view.y = cursor.y - mouse.y / vw->pixels_per_unit;
  vw->focus.x = vw->view.x + (vw->width  / 2.0f) / vw->pixels_per_unit;
  vw->focus.y = vw->view.y + (vw->height / 2.0f) / vw->pixels_per_unit;
}
