#include <arena.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <types.h>

#include "../frontends/frontend.h"
#include "../shared/parser.h"
#include "game.h"
#include "screens.h"
#include "specialscreens.h"

uint_fast8_t FloorSize = 0;
static const struct RoomInfo *Rooms = NULL;
static const struct RoomInfo DefaultRoom = {0};

bool SetupBackend(void) {
  if (!LoadGameData("GameData.json")) {
    PrintError("Failed to load GameData.json");
    return false;
  }

  struct RoomInfo *newRooms = NULL;
  if (!Rooms && !LoadGameRooms(&FloorSize, &newRooms)) {
    PrintError("Failed to load rooms from GameData.json");
    return false;
  }
  Rooms = (const struct RoomInfo *)newRooms;

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

static uint_fast8_t MapInputIndex(struct GameOutput *output, uint_fast8_t inputIndex) {
  for (uint_fast8_t i = 0, visibleInputCount = 0; i < output->inputCount; ++i) {
    if (!output->inputs[i].visible) {
      continue;
    }

    if (inputIndex == visibleInputCount) {
      return i;
    }

    ++visibleInputCount;
  }
  return UINT_FAST8_MAX;
}

enum InputOutcome HandleGameInput(struct GameOutput *output, uint_fast8_t inputIndex) {
  uint_fast8_t inputID = MapInputIndex(output, inputIndex);
  if (UINT_FAST8_MAX == inputID) {
    return InvalidInputOutcome;
  }

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
      output->roomInfo = GetGameRoom(output->roomInfo->x, output->roomInfo->y + 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoEastOutcome:
      output->roomInfo = GetGameRoom(output->roomInfo->x + 1, output->roomInfo->y);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoSouthOutcome:
      output->roomInfo = GetGameRoom(output->roomInfo->x, output->roomInfo->y - 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoWestOutcome:
      output->roomInfo = GetGameRoom(output->roomInfo->x - 1, output->roomInfo->y);
      outcome = GetNextOutputOutcome;
      break;
    default:
      break;
  }

  return outcome;
}

// Room may not exist, always check result->exists
const struct RoomInfo *GetGameRoom(RoomCoord x, RoomCoord y) {
  if (x >= FloorSize || y >= FloorSize
      || x == InvalidRoomCoord || y == InvalidRoomCoord) {
    return &DefaultRoom;
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
  free((void *)Rooms);
  Rooms = NULL;
}
