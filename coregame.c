#include "backend.h"  // for GameInfo, backend_default_action_visibility_checker, backend_get_map_room, USE_ACTION, NEW_EXT_ACTION, Screen, Room, ARR_COUNT, Action, NEW_ACTION, NEW_ROOM, NEW_SCREEN, NEW_VAR_SCREEN
#include "coregame.h"

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

static bool core_quit_game_action_handler(struct GameInfo *info, const struct Action *action) {
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


static const struct Screen CoreRoomScreen;

static const struct GotoScreenAction CoreStartAction = NEW_EXT_ACTION("Start Game", backend_default_action_visibility_checker, core_goto_screen_action_handler, &CoreRoomScreen);
static const struct Action CoreQuitAction = NEW_ACTION("Quit Game", backend_default_action_visibility_checker, core_quit_game_action_handler);
static const struct GoDirectionAction CoreGoNorthAction = NEW_EXT_ACTION("Go North", core_go_direction_visibility_checker, core_go_direction_action_handler, North);
static const struct GoDirectionAction CoreGoEastAction = NEW_EXT_ACTION("Go East", core_go_direction_visibility_checker, core_go_direction_action_handler, East);
static const struct GoDirectionAction CoreGoSouthAction = NEW_EXT_ACTION("Go South", core_go_direction_visibility_checker, core_go_direction_action_handler, South);
static const struct GoDirectionAction CoreGoWestAction = NEW_EXT_ACTION("Go West", core_go_direction_visibility_checker, core_go_direction_action_handler, West);
static const struct GotoScreenAction CoreGotoMainMenuAction = NEW_EXT_ACTION("Quit", backend_default_action_visibility_checker, core_goto_screen_action_handler, &CoreMainMenuScreen);


const struct Screen CoreMainMenuScreen = NEW_SCREEN("\
Untitled text adventure game\n\
----------------------------\n\
By the UTAS Programming Club\n\
\n\
Currently unimplemented :(",
  USE_ACTION(CoreStartAction), USE_ACTION(CoreQuitAction)
);

static const struct Screen CoreRoomScreen = NEW_VAR_SCREEN(core_room_screen_body_generator,
                                                           USE_ACTION(CoreGoNorthAction), USE_ACTION(CoreGoEastAction), USE_ACTION(CoreGoSouthAction), USE_ACTION(CoreGoWestAction),
                                                           USE_ACTION(CoreGotoMainMenuAction)
);

static const struct Room CoreFirstRoom = NEW_ROOM(0, 0, "First Room!");

const struct Room *const CoreRooms[] = { &CoreFirstRoom };
const size_t CoreRoomCount = ARR_COUNT(CoreRooms);
