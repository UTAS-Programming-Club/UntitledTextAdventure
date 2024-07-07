#ifndef PCGAME_SAVE_H
#define PCGAME_SAVE_H

#include <stdbool.h>

#include "game.h"

// Do not free the result, it is part of the GameState arena
char *SaveState(struct GameState *);
bool LoadState(const struct GameInfo *, struct GameState *, const char *);
bool CreateNewState(const struct GameInfo *, struct GameState *);

#endif // PCGAME_SAVE_H
