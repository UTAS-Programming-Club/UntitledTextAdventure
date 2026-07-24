#include <stdlib.h>        // for calloc, free

#include "backend.h"
#include "coregame2.h"     // for CoreMainMenuScreen, MapSizeX, MapSizeY
#include "gen/coregame.h"  // for Core_RoomCount, Core_Rooms

const char *backend_default_screen_body_generator(const struct GameInfo *info) {
  if (NULL == info || NULL == info->screen) {
    return NULL;
  }

  return info->screen->body;
}

bool backend_default_action_visibility_checker(const struct GameInfo *info, const struct Action *action) {
  return true;
}

bool backend_default_action_trigger_handler(struct GameInfo *info, const struct Action *action) {
  return true;
}


bool backend_register_extension(struct GameInfo *info, size_t roomCount, const struct Room *const rooms[static roomCount]) {
  for (size_t i = 0; i < roomCount; ++i) {
    const struct Room *room = rooms[i];
    if (NULL == room || room->x >= info->mapSizeX || room->y >= info->mapSizeY || NULL != backend_get_map_room(info, room->x, room->y)) {
      return false;
    }

    info->map[room->y * info->mapSizeX + room->x] = room;
  }

  return true;
}

const struct Room *backend_get_map_room(const struct GameInfo *info, uint8_t x, uint8_t y) {
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

  info->screen = &CoreMainMenuScreen;
  info->x = info->y = 0;
  info->quit = false;

  return backend_register_extension(info, Core_RoomCount, Core_Rooms);
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
  info->x = info->y = 0;
  info->quit = false;
}
