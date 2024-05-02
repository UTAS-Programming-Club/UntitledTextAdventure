#ifndef PCGAME_SCREENS_H
#define PCGAME_SCREENS_H

#include <stdbool.h>
#include <stdint.h>

#include "../shared/parser.h"
#include "game.h"

bool CreateScreen(enum ScreenID, struct GameOutput *);
void FreeScreen(struct GameOutput *);
bool CreateMainMenuScreen(enum ScreenID, struct GameOutput *);
bool HandleScreenInput(enum ScreenID, uint8_t, struct GameScreenButton *);

#endif // PCGAME_SCREENS_H
