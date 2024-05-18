#include <stdbool.h>
#include <stdlib.h>

#include "types.h"
#include "game.h"
#include "screens.h"
#include "specialscreens.h"
#include "../shared/parser.h"

bool SetupGame(void) {
  return LoadGameData("GameData.json");
}

// TODO: Add more screens
bool GetCurrentGameOutput(struct GameOutput *output) {
  if (!output || output->screenID == InvalidScreenID) {
    return false;
  }

  if (!output->stateData) {
    output->stateData = InitGameState();
  }
  if (!output->stateData) {
    return false;
  }

  arena_reset(&output->arena);

  if (!CreateScreen(output)) {
    return false;
  }

  if (InvalidCustomScreenID != output->customScreenID) {
    return CustomScreens[output->customScreenID](output);
  } else {
    return true;
  }
}

enum InputOutcome HandleGameInput(struct GameOutput *output, uint32_t inputID) {
  struct GameScreenButton button = {0};
  if (!GetGameScreenButton(output->screenID, inputID, &button)) {
    return InvalidInputOutcome;
  }

  enum InputOutcome outcome = button.outcome;
  switch (outcome) {
    case GotoScreen:
      output->screenID = button.newScreen;
      // fall through
    case GameGoNorth:
    case GameGoEast:
    case GameGoSouth:
    case GameGoWest:
      outcome = GetNextOutput;
      break;
    default:
      break;
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
