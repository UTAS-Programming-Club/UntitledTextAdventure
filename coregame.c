#include <stdint.h>

#include "backend.h"
#include "coregame.h"

static bool CoreGameQuitActionHandler(struct GameInfo *info, const struct Action *action) {
  info->quit = true;
  return true;
}

static bool CoreGoDirectionActionHandler(struct GameInfo *info, const struct Action *action) {
  const struct GoDirectionAction *this = (const struct GoDirectionAction *)action;
  switch (this->direction) {
    case North:
      if (nullptr != backend_get_map_screen(info, info->x, info->y + 1)) {
        ++info->y;
      }
      break;
    case East:
      if (nullptr != backend_get_map_screen(info, info->x + 1, info->y)) {
        ++info->x;
      }
      break;
    case South:
      if (nullptr != backend_get_map_screen(info, info->x, info->y - 1)) {
        --info->y;
      }
      break;
    case West:
      if (nullptr != backend_get_map_screen(info, info->x - 1, info->y)) {
        --info->x;
      }
      break;
  }

  info->screen = backend_get_map_screen(info, info->x, info->y);
  return true;
}


const struct Action CoreQuitAction = NEW_ACTION("Quit Game", CoreGameQuitActionHandler);
const struct GoDirectionAction CoreGoNorthAction = NEW_EXT_ACTION("Go North", CoreGoDirectionActionHandler, North);
const struct GoDirectionAction CoreGoEastAction = NEW_EXT_ACTION("Go East", CoreGoDirectionActionHandler, East);
const struct GoDirectionAction CoreGoSouthAction = NEW_EXT_ACTION("Go South", CoreGoDirectionActionHandler, South);
const struct GoDirectionAction CoreGoWestAction = NEW_EXT_ACTION("Go West", CoreGoDirectionActionHandler, West);

const struct Screen CoreTestScreen = NEW_SCREEN(0, 0, "This is a test: 1", 
                                                 USE_ACTION(CoreQuitAction),
                                                 USE_ACTION(CoreGoNorthAction), USE_ACTION(CoreGoEastAction), USE_ACTION(CoreGoSouthAction), USE_ACTION(CoreGoWestAction)
);

const struct Screen *const CoreScreens[] = { &CoreTestScreen };
const size_t CoreScreenCount = ARR_COUNT(CoreScreens);

const uint8_t MapSizeX = 3;
const uint8_t MapSizeY = 2;
