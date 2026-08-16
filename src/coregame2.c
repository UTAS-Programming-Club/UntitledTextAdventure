#include <stdbool.h>       // for true, bool, false
#include <stddef.h>        // for NULL
#include <stdint.h>        // for uint8_t

#include "backend.h"       // GameInfo, backend_get_map_room, Action (ptr only), Room
#include "coregame2.h"
#include "gen/coregame.h"  // for Core_ScreenDirection, Core_GoDirectionAction

bool core_go_direction_visibility_checker(const struct GameInfo *const info, const struct Action *const action) {
  const struct Core_GoDirectionAction *const this = (const struct Core_GoDirectionAction *const)action;
  switch (this->direction) {
    case Core_ScreenDirection_North:
      if (NULL != backend_get_map_room(info, info->x, info->y + 1)) {
        return true;
      }
      break;
    case Core_ScreenDirection_East:
      if (NULL != backend_get_map_room(info, info->x + 1, info->y)) {
        return true;
      }
      break;
    case Core_ScreenDirection_South:
      if (NULL != backend_get_map_room(info, info->x, info->y - 1)) {
        return true;
      }
      break;
    case Core_ScreenDirection_West:
      if (NULL != backend_get_map_room(info, info->x - 1, info->y)) {
        return true;
      }
      break;
  }

  return false;
}


/*static bool core_goto_screen_action_handler(struct GameInfo *const info, const struct Action *const action) {
  const struct Core_GotoScreenAction *const this = (const struct Core_GotoScreenAction *const)action;
  info->screen = this->screen;
  return true;
}*/

const char *core_room_screen_body_generator(const struct GameInfo *const info) {
  const struct Room *const room = backend_get_map_room(info, info->x, info->y);
  if (NULL == room) {
    return NULL;
  }

  return room->body;
}

const uint8_t MapSizeX = 3;
const uint8_t MapSizeY = 2;
