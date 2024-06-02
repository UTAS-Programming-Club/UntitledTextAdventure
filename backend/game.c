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

bool GetCurrentGameOutput(struct GameOutput *output) {
  if (!output || output->screenID == InvalidScreen) {
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

  if (InvalidCustomScreenCode != output->customScreenCodeID) {
    return CustomScreenCode[output->customScreenCodeID](output);
  } else {
    return true;
  }
}

static uint8_t MapInputIndex(struct GameOutput *output, uint8_t inputIndex) {
  for (uint8_t i = 0, visibleInputCount = 0; i < output->inputCount; ++i) {
    if (!output->inputs[i].visible) {
      continue;
    }

    if (inputIndex == visibleInputCount) {
      return i;
    }

    ++visibleInputCount;
  }
  return UINT8_MAX;
}

enum InputOutcome HandleGameInput(struct GameOutput *output, uint8_t inputIndex) {
  uint8_t inputID = MapInputIndex(output, inputIndex);

  struct GameScreenButton button = {0};
  if (!GetGameScreenButton(output->screenID, inputID, &button)) {
    return InvalidInputOutcome;
  }

  enum InputOutcome outcome = button.outcome;
  switch (outcome) {
    case GotoScreenOutcome:
      output->screenID = button.newScreenID;
      outcome = GetNextOutputOutcome;
      break;
    case GameGoNorthOutcome:
      output->roomInfo.roomID = output->roomInfo.northRoomID;
      outcome = GetNextOutputOutcome;
      break;
    case GameGoEastOutcome:
      output->roomInfo.roomID = output->roomInfo.eastRoomID;
      outcome = GetNextOutputOutcome;
      break;
    case GameGoSouthOutcome:
      output->roomInfo.roomID = output->roomInfo.southRoomID;
      outcome = GetNextOutputOutcome;
      break;
    case GameGoWestOutcome:
      output->roomInfo.roomID = output->roomInfo.westRoomID;
      outcome = GetNextOutputOutcome;
      break;
    default:
      break;
  }

  return outcome;
}

void CleanupGame(struct GameOutput *output) {
  arena_free(&output->arena);
  UnloadGameData();
  free(output->stateData);
  output->stateData = NULL;
}
