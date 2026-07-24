#ifndef UTA_COREGAME2_H
#define UTA_COREGAME2_H

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

extern const uint8_t MapSizeX;
extern const uint8_t MapSizeY;

#endif // UTA_COREGAME2_H
