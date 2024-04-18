#include <stdbool.h>

#include "alloc.h"
#include "game.h"
#include "screens.h"
#include "../shared/parser.h"

bool SetupGame(void) {
  return LoadGameData("GameData.json");
}

// TODO: Add more screens
bool GetCurrentGameOutput(struct GameOutput *output) {
  return CreateMainMenuScreen(output);
}

enum GameInputOutcome HandleGameInput(uint32_t screenID, uint32_t inputID) {
  switch (screenID) {
    case MAIN_MENU_SCREEN_ID:
      return HandleMainMenuScreenInput(inputID);
    break;
  }

  return InvalidInput;
}

void CleanupGame(void) {
  UnloadGameData();
  FreeAll();
}
