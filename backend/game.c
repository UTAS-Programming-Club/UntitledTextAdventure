#include <stdbool.h>
#include <stdlib.h>

#include "types.h"
#include "game.h"
#include "screens.h"
#include "../shared/parser.h"

// TODO: Move to GameOutput struct
static enum ScreenID ScreenID = InvaidScreenID;

bool SetupGame(void) {
  if (!LoadGameData("GameData.json")) {
    return false;
  }

  ScreenID = MainMenu;
  return true;
}

// TODO: Add more screens
bool GetCurrentGameOutput(struct GameOutput *output) {
  if (!output) {
    return false;
  }

  if (ScreenID == InvaidScreenID) {
    return false;
  }

  if (!output->stateData) {
    output->stateData = InitGameState();
  }
  if (!output->stateData) {
    return false;
  }

  arena_reset(&output->arena);

  if (ScreenID == MainMenu) {
    return CreateMainMenuScreen(ScreenID, output);
  } else {
    return CreateScreen(ScreenID, output);
  }
}

enum InputOutcome HandleGameInput(enum ScreenID screenID, uint32_t inputID) {
  struct GameScreenButton button = {0};
  if (!HandleScreenInput(screenID, inputID, &button)) {
    return InvalidInputOutcome;
  }

  enum InputOutcome outcome = button.outcome;
  if (outcome == GotoScreen) {
    ScreenID = button.newScreen;
    outcome = GetNextOutput;
  }

  FreeGameScreenButton(&button);
  return outcome;
}

void CleanupGame(struct GameOutput *output) {
  arena_free(&output->arena);
  UnloadGameData();
  free(output->stateData);
  output->stateData = NULL;
}
