#include <stdbool.h>  // for true, bool, false
#include <stdint.h>   // for uint8_t

#include "backend.h"  // for GameInfo, backend_get_map_screen, USE_ACTION, NEW_EXT_ACTION, backend_default_action_visibility_checker, ARR_COUNT, Action, NEW_ACTION, NEW_SCREEN, Screen

static bool core_go_direction_visibility_checker(const struct GameInfo *info, const struct Action *action) {
  const struct GoDirectionAction *this = (const struct GoDirectionAction *)action;
  switch (this->direction) {
    case North:
      if (NULL != backend_get_map_screen(info, info->x, info->y + 1)) {
        return true;
      }
      break;
    case East:
      if (NULL != backend_get_map_screen(info, info->x + 1, info->y)) {
        return true;
      }
      break;
    case South:
      if (NULL != backend_get_map_screen(info, info->x, info->y - 1)) {
        return true;
      }
      break;
    case West:
      if (NULL != backend_get_map_screen(info, info->x - 1, info->y)) {
        return true;
      }
      break;
  }

  return false;
}

static bool core_quit_action_handler(struct GameInfo *info, const struct Action *action) {
  info->quit = true;
  return true;
}

static bool core_go_direction_action_handler(struct GameInfo *info, const struct Action *action) {
  const struct GoDirectionAction *this = (const struct GoDirectionAction *)action;
  switch (this->direction) {
    case North:
      if (NULL != backend_get_map_screen(info, info->x, info->y + 1)) {
        ++info->y;
      }
      break;
    case East:
      if (NULL != backend_get_map_screen(info, info->x + 1, info->y)) {
        ++info->x;
      }
      break;
    case South:
      if (NULL != backend_get_map_screen(info, info->x, info->y - 1)) {
        --info->y;
      }
      break;
    case West:
      if (NULL != backend_get_map_screen(info, info->x - 1, info->y)) {
        --info->x;
      }
      break;
  }

  info->screen = backend_get_map_screen(info, info->x, info->y);
  return true;
}


const struct Action CoreQuitAction = NEW_ACTION("Quit Game", backend_default_action_visibility_checker, core_quit_action_handler);
const struct GoDirectionAction CoreGoNorthAction = NEW_EXT_ACTION("Go North", core_go_direction_visibility_checker, core_go_direction_action_handler, North);
const struct GoDirectionAction CoreGoEastAction = NEW_EXT_ACTION("Go East", core_go_direction_visibility_checker, core_go_direction_action_handler, East);
const struct GoDirectionAction CoreGoSouthAction = NEW_EXT_ACTION("Go South", core_go_direction_visibility_checker, core_go_direction_action_handler, South);
const struct GoDirectionAction CoreGoWestAction = NEW_EXT_ACTION("Go West", core_go_direction_visibility_checker, core_go_direction_action_handler, West);

const struct Screen CoreTestScreen = NEW_SCREEN(0, 0, "This is a test: 1", 
                                                 USE_ACTION(CoreQuitAction),
                                                 USE_ACTION(CoreGoNorthAction), USE_ACTION(CoreGoEastAction), USE_ACTION(CoreGoSouthAction), USE_ACTION(CoreGoWestAction)
);

const struct Screen *const CoreScreens[] = { &CoreTestScreen };
const size_t CoreScreenCount = ARR_COUNT(CoreScreens);

const uint8_t MapSizeX = 3;
const uint8_t MapSizeY = 2;
