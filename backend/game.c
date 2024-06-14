#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "game.h"
#include "screens.h"
#include "specialscreens.h"
#include "../shared/parser.h"

uint8_t FloorSize = 0;
struct RoomInfo *Rooms = NULL;

static inline uint64_t Min(uint64_t a, uint64_t b) {
  return a > b ? a : b;
}

bool SetupBackend(void) {
  if (!LoadGameData("GameData.json")) {
    fprintf(stderr, "ERROR: Failed to load GameData.json\n");
    return false;
  }

  if (!Rooms && !LoadGameRooms(&FloorSize, &Rooms)) {
    fprintf(stderr, "ERROR: Failed to load rooms from GameData.json\n");
    return false;
  }

  return true;
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
      output->roomInfo->y = Min(output->roomInfo->y + 1, FloorSize - 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoEastOutcome:
      output->roomInfo->x = Min(output->roomInfo->x + 1, FloorSize - 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoSouthOutcome:
      output->roomInfo->y = Min(output->roomInfo->y - 1, 0);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoWestOutcome:
      output->roomInfo->x = Min(output->roomInfo->x - 1, 0);
      outcome = GetNextOutputOutcome;
      break;
    default:
      break;
  }

  return outcome;
}

// Room may not exist, always check result->exists
struct RoomInfo *GetGameRoom(RoomCoord x, RoomCoord y) {
if (x >= FloorSize || y >= FloorSize
    || x == InvalidRoomCoord || y == InvalidRoomCoord) {
    return NULL;
  }

  return &(Rooms[y * FloorSize + x]);
}

void CleanupGame(struct GameOutput *output) {
  arena_free(&output->arena);
  free(output->stateData);
  output->stateData = NULL;
}

void CleanupBackend(void) {
  UnloadGameData();
  FloorSize = 0;
  free(Rooms);
  Rooms = NULL;
}
