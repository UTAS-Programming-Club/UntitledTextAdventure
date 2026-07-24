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

extern const struct Screen CoreMainMenuScreen;

extern const uint8_t MapSizeX;
extern const uint8_t MapSizeY;

bool core_quit_game_action_handler(struct GameInfo *info, const struct Action *action);

#endif // UTA_COREGAME2_H
