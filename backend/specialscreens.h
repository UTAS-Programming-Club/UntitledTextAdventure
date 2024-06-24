#ifndef PCGAME_SPECIAL_SCREENS_H
#define PCGAME_SPECIAL_SCREENS_H

#include <stdbool.h>

#include "game.h"

extern bool (*CustomScreenCode[])(const struct GameInfo *, struct GameState *);

#endif // PCGAME_SPECIAL_SCREENS_H
