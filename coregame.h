#ifndef UTA_COREGAME_H
#ifndef UTA_IN_BACKEND_H
#define UTA_COREGAME_H

#include "smalllib.h"

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
#endif

#define MapSizeX (uint8_t)3
#define MapSizeY (uint8_t)2

#endif // UTA_COREGAME_H
