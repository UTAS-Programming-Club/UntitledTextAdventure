#include <stdlib.h>
#include <string.h>

#include "backend.h"
#include "coregame.h"

bool backend_default_action_handler(struct GameInfo *info, const struct Action *action) {
  return true;
}

bool backend_register_extension(struct GameInfo *info, size_t screenCount, const struct Screen *const screens[static screenCount]) {
  for (size_t i = 0; i < screenCount; ++i) {
    const struct Screen *screen = screens[i];
    if (nullptr == screen || screen->x >= info->mapSizeX || screen->y >= info->mapSizeY || nullptr != backend_get_map_screen(info, screen->x, screen->y)) {
      return false;
    }
    
    info->map[screen->y * info->mapSizeX + screen->x] = screen;
  }

  return true;
}

const struct Screen *backend_get_map_screen(const struct GameInfo *info, uint8_t x, uint8_t y) {
  if (x >= info->mapSizeX || y >= info->mapSizeY) {
    return nullptr;
  }

  return info->map[y * info->mapSizeX + x];
}


bool backend_setup(struct GameInfo *info) {
  if (nullptr == info) {
    return false;
  }

  info->mapSizeX = MapSizeX;
  info->mapSizeY = MapSizeY;
  info->map = calloc(info->mapSizeX * info->mapSizeY, sizeof *info->map);

  info->screen = &CoreTestScreen;
  info->quit = false;

  return backend_register_extension(info, CoreScreenCount, CoreScreens);
}

bool backend_input(struct GameInfo *info, uint8_t actionIdx) {
  const struct Action *action = info->screen->actions[actionIdx];
  return action->handler(info, action);
}

void backend_cleanup(struct GameInfo *info) {
  if (nullptr == info) {
    return;
  }

  info->mapSizeX = info->mapSizeY = 0;
  free((void *)info->map);
  info->map = nullptr;

  info->screen = nullptr;
  info->quit = false;
}
