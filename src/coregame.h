#ifndef UTA_COREGAME_H
#define UTA_COREGAME_H

#include <stddef.h>   // for size_t
#include <stdint.h>   // for uint8_t

#include "backend.h"  // for Action

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

extern const size_t CoreRoomCount;
extern const struct Room *const CoreRooms[];

extern const uint8_t MapSizeX;
extern const uint8_t MapSizeY;

#endif // UTA_COREGAME_H
