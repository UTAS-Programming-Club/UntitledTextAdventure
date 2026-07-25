#include <stdbool.h>  // for true, bool, false
#include <stddef.h>   // for NULL
#include <stdint.h>   // for uint8_t

#include "backend.h"  // for GameInfo, backend_get_map_room, backend_default_action_visibility_checker, USE_ACTION, NEW_EXT_ACTION, Action (ptr only), Screen, NEW_SCREEN, NEW_VAR_SCREEN, Room
#include "coregame2.h"
#include "gen/coregame.h" // for Core_QuitAction

static bool core_go_direction_visibility_checker(const struct GameInfo *info, const struct Action *action) {
  const struct GoDirectionAction *this = (const struct GoDirectionAction *)action;
  switch (this->direction) {
    case North:
      if (NULL != backend_get_map_room(info, info->x, info->y + 1)) {
        return true;
      }
      break;
    case East:
      if (NULL != backend_get_map_room(info, info->x + 1, info->y)) {
        return true;
      }
      break;
    case South:
      if (NULL != backend_get_map_room(info, info->x, info->y - 1)) {
        return true;
      }
      break;
    case West:
      if (NULL != backend_get_map_room(info, info->x - 1, info->y)) {
        return true;
      }
      break;
  }

  return false;
}


static bool core_goto_screen_action_handler(struct GameInfo *info, const struct Action *action) {
  const struct GotoScreenAction *this = (const struct GotoScreenAction *)action;
  info->screen = this->screen;
  return true;
}

bool core_quit_game_action_handler(struct GameInfo *info, const struct Action *action) {
  info->quit = true;
  return true;
}

static bool core_go_direction_action_handler(struct GameInfo *info, const struct Action *action) {
  const struct GoDirectionAction *this = (const struct GoDirectionAction *)action;
  switch (this->direction) {
    case North:
      if (NULL != backend_get_map_room(info, info->x, info->y + 1)) {
        ++info->y;
      }
      break;
    case East:
      if (NULL != backend_get_map_room(info, info->x + 1, info->y)) {
        ++info->x;
      }
      break;
    case South:
      if (NULL != backend_get_map_room(info, info->x, info->y - 1)) {
        --info->y;
      }
      break;
    case West:
      if (NULL != backend_get_map_room(info, info->x - 1, info->y)) {
        --info->x;
      }
      break;
  }

  // info->screen = backend_get_map_room(info, info->x, info->y);
  return true;
}


static const char *core_room_screen_body_generator(const struct GameInfo *info) {
  const struct Room *room = backend_get_map_room(info, info->x, info->y);
  if (NULL == room) {
    return NULL;
  }

  return room->body;
}

const struct GotoScreenAction CoreStartAction = NEW_EXT_ACTION("Start Game", backend_default_action_visibility_checker, core_goto_screen_action_handler, &CoreRoomScreen);
static const struct GoDirectionAction CoreGoNorthAction = NEW_EXT_ACTION("Go North", core_go_direction_visibility_checker, core_go_direction_action_handler, North);
static const struct GoDirectionAction CoreGoEastAction = NEW_EXT_ACTION("Go East", core_go_direction_visibility_checker, core_go_direction_action_handler, East);
static const struct GoDirectionAction CoreGoSouthAction = NEW_EXT_ACTION("Go South", core_go_direction_visibility_checker, core_go_direction_action_handler, South);
static const struct GoDirectionAction CoreGoWestAction = NEW_EXT_ACTION("Go West", core_go_direction_visibility_checker, core_go_direction_action_handler, West);
static const struct GotoScreenAction CoreGotoMainMenuAction = NEW_EXT_ACTION("Quit", backend_default_action_visibility_checker, core_goto_screen_action_handler, &Core_MainMenuScreen);

const struct Screen CoreRoomScreen = NEW_VAR_SCREEN(core_room_screen_body_generator,
                                                    USE_ACTION(CoreGoNorthAction), USE_ACTION(CoreGoEastAction), USE_ACTION(CoreGoSouthAction), USE_ACTION(CoreGoWestAction),
                                                    USE_ACTION(CoreGotoMainMenuAction)
);

const uint8_t MapSizeX = 3;
const uint8_t MapSizeY = 2;
