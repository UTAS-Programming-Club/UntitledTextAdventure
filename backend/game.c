#include <arena.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <types.h>

#include "../frontends/frontend.h"
#include "game.h"
#include "parser.h"
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

bool UpdateGameState(struct GameState *state) {
  if (!state || state->screenID == InvalidScreen) {
    return false;
  }

  if (!state->stateData) {
    state->stateData = InitGameState();
  }
  if (!state->stateData) {
    return false;
  }

  arena_reset(&state->arena);

  if (!CreateScreen(state)) {
    return false;
  }

  if (InvalidCustomScreenCode != state->customScreenCodeID) {
    return CustomScreenCode[state->customScreenCodeID](state);
  } else {
    return true;
  }
}

static uint_fast8_t MapInputIndex(struct GameState *state, uint_fast8_t inputIndex) {
  for (uint_fast8_t i = 0, visibleInputCount = 0; i < state->inputCount; ++i) {
    if (!state->inputs[i].visible) {
      continue;
    }

    if (inputIndex == visibleInputCount) {
      return i;
    }

    ++visibleInputCount;
  }
  return UINT_FAST8_MAX;
}

enum InputOutcome HandleGameInput(struct GameState *state, uint_fast8_t inputIndex) {
  uint_fast8_t inputID = MapInputIndex(state, inputIndex);
  if (UINT_FAST8_MAX == inputID) {
    return InvalidInputOutcome;
  }

  struct GameScreenButton button = {0};
  if (!GetGameScreenButton(state->screenID, inputID, &button)) {
    return InvalidInputOutcome;
  }

  enum InputOutcome outcome = button.outcome;
  switch (outcome) {
    case GotoScreenOutcome:
      state->screenID = button.newScreenID;
      outcome = GetNextOutputOutcome;
      break;
    case GameGoNorthOutcome:
      state->roomInfo = GetGameRoom(state->roomInfo->x, state->roomInfo->y + 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoEastOutcome:
      state->roomInfo = GetGameRoom(state->roomInfo->x + 1, state->roomInfo->y);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoSouthOutcome:
      state->roomInfo = GetGameRoom(state->roomInfo->x, state->roomInfo->y - 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoWestOutcome:
      state->roomInfo = GetGameRoom(state->roomInfo->x - 1, state->roomInfo->y);
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

void CleanupGame(struct GameState *state) {
  arena_free(&state->arena);
  free(state->stateData);
  state->stateData = NULL;
}

void CleanupBackend(void) {
  UnloadGameData();
  FloorSize = 0;
  free((void *)Rooms);
  Rooms = NULL;
}
