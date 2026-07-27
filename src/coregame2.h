#ifndef UTA_COREGAME2_H
#define UTA_COREGAME2_H

#include <stdbool.h>  // for bool
#include <stdint.h>   // for uint8_t

#include "backend.h"  // for Action, GameInfo (ptr only)

enum ScreenDirection {
  North,
  East,
  South,
  West
};

struct GotoScreenAction {
  struct Action base;

  const struct Screen *screen;
};

struct GoDirectionAction {
  struct Action base;

  enum ScreenDirection direction;
};

extern const struct GotoScreenAction CoreStartAction;
extern const struct GoDirectionAction CoreGoNorthAction;
extern const struct GoDirectionAction CoreGoEastAction;
extern const struct GoDirectionAction CoreGoSouthAction;
extern const struct GoDirectionAction CoreGoWestAction;
extern const struct GotoScreenAction CoreGotoMainMenuAction;

extern const uint8_t MapSizeX;
extern const uint8_t MapSizeY;

bool core_test_action_handler(struct GameInfo *info, const struct Action *action);
bool core_quit_game_action_handler(struct GameInfo *info, const struct Action *action);

const char *core_room_screen_body_generator(const struct GameInfo *info);

#endif // UTA_COREGAME2_H
