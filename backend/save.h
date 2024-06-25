#ifndef PCGAME_SAVE_H
#define PCGAME_SAVE_H

#include <stdbool.h>

#include "game.h"

// Do not free the result, it is part of the GameState arena
char *SaveState(struct GameState *);
bool LoadState(const struct GameInfo *, struct GameState *, const char *);

#endif // PCGAME_SAVE_H
