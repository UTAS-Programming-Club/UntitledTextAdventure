#ifndef PCGAME_SCREENS_H
#define PCGAME_SCREENS_H

#include <stdbool.h>
#include <stdint.h>

#include "game.h"

#define MAIN_MENU_SCREEN_ID 0
#define TEST_SCREEN_ID 1

bool CreateMainMenuScreen(struct GameOutput *);
bool CreateTestScreen(struct GameOutput *);
enum GameInputOutcome HandleMainMenuScreenInput(uint8_t);
enum GameInputOutcome HandleTestScreenInput(uint8_t);

#endif // PCGAME_SCREENS_H
