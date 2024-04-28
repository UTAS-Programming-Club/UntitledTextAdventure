#ifndef PCGAME_SCREENS_H
#define PCGAME_SCREENS_H

#include <stdbool.h>
#include <stdint.h>

#include "../shared/parser.h"
#include "game.h"

bool CreateScreen(uint32_t, struct GameOutput *);
void FreeScreen(struct GameOutput *);
bool CreateMainMenuScreen(uint32_t, struct GameOutput *);
bool HandleScreenInput(uint32_t, uint8_t, struct GameScreenButton *);

#endif // PCGAME_SCREENS_H
