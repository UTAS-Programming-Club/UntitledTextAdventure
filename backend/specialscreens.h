#ifndef PCGAME_SPECIAL_SCREENS_H
#define PCGAME_SPECIAL_SCREENS_H

#include <stdbool.h>
#include <stddef.h>

#include "game.h"

extern bool (*CustomScreenCode[])(const struct GameInfo *, struct GameState *);
extern size_t CustomScreenCodeCount;

#endif // PCGAME_SPECIAL_SCREENS_H
