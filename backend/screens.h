#ifndef PCGAME_SCREENS_H
#define PCGAME_SCREENS_H

#include <stdbool.h>
#include <stdint.h>

#include "game.h"

#define MAIN_MENU_SCREEN_ID 0

// TODO: Remove these
#define MAIN_MENU_START_BUTTON 0
#define MAIN_MENU_QUIT_BUTTON 1

bool CreateMainMenuScreen(struct GameOutput *);
enum GameInputOutcome HandleMainMenuScreenInput(uint8_t);

#endif // PCGAME_SCREENS_H
