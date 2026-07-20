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

struct GoDirectionAction {
  struct Action base;

  enum ScreenDirection direction;
};

extern const struct Action CoreQuitAction;
extern const struct GoDirectionAction CoreGoNorthAction;
extern const struct GoDirectionAction CoreGoEastAction;
extern const struct GoDirectionAction CoreGoSouthAction;
extern const struct GoDirectionAction CoreGoWestAction;

extern const struct Screen CoreTestScreen;

extern const size_t CoreScreenCount;
extern const struct Screen *const CoreScreens[];

extern const uint8_t MapSizeX;
extern const uint8_t MapSizeY;

#endif // UTA_COREGAME_H
