#ifndef PCGAME_SAVE_H
#define PCGAME_SAVE_H

#include <stdbool.h>

#include "game.h"

// Do not free the result, it is part of the GameState arena
char *GetSavePassword(struct GameState *);

#endif // PCGAME_SAVE_H
