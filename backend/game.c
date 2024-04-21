#include <stdbool.h>

#include "alloc.h"
#include "game.h"
#include "screens.h"
#include "../shared/parser.h"

static uint32_t MainMenuScreenID = INVALID_SCREEN_ID;
static uint32_t ScreenID = INVALID_SCREEN_ID;

bool SetupGame(void) {
  if (!LoadGameData("GameData.json")) {
    return false;
  }

  MainMenuScreenID = GetMainMenuScreenID();
  ScreenID = MainMenuScreenID;
  if (MainMenuScreenID == INVALID_SCREEN_ID) {
    return false;
  }

  return true;
}

// TODO: Add more screens
bool GetCurrentGameOutput(struct GameOutput *output) {
  if (ScreenID == MainMenuScreenID) {
    return CreateMainMenuScreen(ScreenID, output);
  } else {
    return CreateScreen(ScreenID, output);
  }
}

enum GameInputOutcome HandleGameInput(uint32_t screenID, uint32_t inputID) {
  struct GameScreenButton *button = HandleScreenInput(screenID, inputID);
  if (!button) {
    return InvalidInput;
  }

  enum GameInputOutcome outcome = button->outcome;
  if (outcome == GotoScreen) {
    ScreenID = button->newScreen;
    outcome = GetNextOutput;
  }

  FreeGameScreenButton(button);
  return outcome;
}

void CleanupGame(void) {
  UnloadGameData();
  FreeAll();
}
