#include <stdbool.h>

#include "alloc.h"
#include "game.h"
#include "screens.h"
#include "../shared/parser.h"

static int32_t ScreenID = 0;

bool SetupGame(void) {
  return LoadGameData("GameData.json");
}

// TODO: Add more screens
// TODO: Seperate hardcoded IDs from json indexes. Add id field for both screens and buttons?
bool GetCurrentGameOutput(struct GameOutput *output) {
  switch (ScreenID) {
    case MAIN_MENU_SCREEN_ID:
      return CreateMainMenuScreen(output);
    case TEST_SCREEN_ID:
      return CreateTestScreen(output);
  }

  return false;
}

enum GameInputOutcome HandleGameInput(uint32_t screenID, uint32_t inputID) {
  enum GameInputOutcome outcome = InvalidInput;
  switch (screenID) {
    case MAIN_MENU_SCREEN_ID:
      outcome = HandleMainMenuScreenInput(inputID);
      break;
    case TEST_SCREEN_ID:
      outcome = HandleTestScreenInput(inputID);
      break;
  }

  // TODO: Fix this, return more data from HandleXInput?
  switch (outcome) {
    case GotoTestScreen:
      ScreenID = TEST_SCREEN_ID;
      outcome = GetNextOutput;
      break;
    case GotoMainMenuScreen:
      ScreenID = MAIN_MENU_SCREEN_ID;
      outcome = GetNextOutput;
      break;
    default:
      break;
  }

  return outcome;
}

void CleanupGame(void) {
  UnloadGameData();
  FreeAll();
}
