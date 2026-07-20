#include <stdlib.h>    // for calloc, free

#include "backend.h"
#include "coregame.h"  // for CoreScreenCount, CoreScreens, CoreTestScreen, MapSizeX, MapSizeY

bool backend_default_action_visibility_checker(const struct GameInfo *info, const struct Action *action) {
  return true;
}


bool backend_default_action_trigger_handler(struct GameInfo *info, const struct Action *action) {
  return true;
}


bool backend_register_extension(struct GameInfo *info, size_t screenCount, const struct Screen *const screens[static screenCount]) {
  for (size_t i = 0; i < screenCount; ++i) {
    const struct Screen *screen = screens[i];
    if (NULL == screen || screen->x >= info->mapSizeX || screen->y >= info->mapSizeY || NULL != backend_get_map_screen(info, screen->x, screen->y)) {
      return false;
    }

    info->map[screen->y * info->mapSizeX + screen->x] = screen;
  }

  return true;
}

const struct Screen *backend_get_map_screen(const struct GameInfo *info, uint8_t x, uint8_t y) {
  if (x >= info->mapSizeX || y >= info->mapSizeY) {
    return NULL;
  }

  return info->map[y * info->mapSizeX + x];
}


bool backend_setup(struct GameInfo *info) {
  if (NULL == info) {
    return false;
  }

  info->mapSizeX = MapSizeX;
  info->mapSizeY = MapSizeY;
  info->map = calloc(info->mapSizeX * info->mapSizeY, sizeof *info->map);

  info->screen = &CoreTestScreen;
  info->quit = false;

  return backend_register_extension(info, CoreScreenCount, CoreScreens);
}

bool backend_input(struct GameInfo *info, uint8_t actionId) {
  uint8_t currentActionId = 0;
  const struct Action *action;
  for (size_t actionIdx = 0; currentActionId <= actionId && actionIdx < info->screen->actionCount; ++actionIdx) {
    action = info->screen->actions[actionIdx];
    if (action->visibility_checker(info, action)) {
      ++currentActionId;
    }
  }

  if (currentActionId - 1 > actionId) {
    return false;
  }
  return action->trigger_handler(info, action);
}

void backend_cleanup(struct GameInfo *info) {
  if (NULL == info) {
    return;
  }

  info->mapSizeX = info->mapSizeY = 0;
  free((void *)info->map);
  info->map = NULL;

  info->screen = NULL;
  info->quit = false;
}
