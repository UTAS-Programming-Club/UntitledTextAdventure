#include <arena.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <types.h>

#include "../frontends/frontend.h"
#include "game.h"
#include "parser.h"
#include "screens.h"
#include "specialscreens.h"

static const struct RoomInfo DefaultRoom = {0};

bool SetupBackend(struct GameInfo *info) {
  if (!LoadGameData("GameData.json")) {
    PrintError("Failed to load GameData.json");
    return false;
  }

  if (info->initialised) {
    return true;
  }

  char *name = LoadGameName();
  if (!name) {
    return false;
  }

  uint_fast8_t floorSize = 0;
  struct RoomInfo *rooms = NULL;
  if (!LoadGameRooms(&floorSize, &rooms)) {
    PrintError("Failed to load rooms from GameData.json");
    return false;
  }

  struct GameInfo tempInfo = {name, true, floorSize, rooms};
  memcpy(info, &tempInfo, sizeof tempInfo);

  return true;
}

bool UpdateGameState(const struct GameInfo *info, struct GameState *state) {
  if (!info || !info->initialised || !state || state->screenID == InvalidScreen) {
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
    return CustomScreenCode[state->customScreenCodeID](info, state);
  } else {
    return true;
  }
}

static uint_fast8_t MapInputIndex(const struct GameState *state, uint_fast8_t inputIndex) {
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

enum InputOutcome HandleGameInput(const struct GameInfo *info, struct GameState *state, uint_fast8_t inputIndex) {
  uint_fast8_t inputID = MapInputIndex(state, inputIndex);
  if (!info || !info->initialised || !state || UINT_FAST8_MAX == inputID) {
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
      state->roomInfo = GetGameRoom(info, state->roomInfo->x, state->roomInfo->y + 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoEastOutcome:
      state->roomInfo = GetGameRoom(info, state->roomInfo->x + 1, state->roomInfo->y);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoSouthOutcome:
      state->roomInfo = GetGameRoom(info, state->roomInfo->x, state->roomInfo->y - 1);
      outcome = GetNextOutputOutcome;
      break;
    case GameGoWestOutcome:
      state->roomInfo = GetGameRoom(info, state->roomInfo->x - 1, state->roomInfo->y);
      outcome = GetNextOutputOutcome;
      break;
    case PlayerDartTrapOutcome:
      // chance to dodge the trap else take damage
      // TODO: Ensure this only trigger once, track room completion?
      // TODO: End game when health is 0
      // TODO: Prevent health from wrapping around
      // TODO: Move constants to types.in.h and GameData.in.json
      srand((unsigned int)time(NULL));
      if((rand() % 100) < 50) {
        state->playerInfo.health -= 10;
      }
      outcome = GetNextOutputOutcome;
      break;
    default:
      break;
  }

  return outcome;
}

// Room may not exist, always check result->exists
const struct RoomInfo *GetGameRoom(const struct GameInfo *info, RoomCoord x, RoomCoord y) {
  if (!info || !info->initialised
      || x >= info->floorSize || y >= info->floorSize
      || x == InvalidRoomCoord || y == InvalidRoomCoord) {
    return &DefaultRoom;
  }

  return &(info->rooms[y * info->floorSize + x]);
}

void CleanupGame(struct GameState *state) {
  if (!state) {
    return;
  }

  arena_free(&state->arena);
  free(state->stateData);
  state->stateData = NULL;
}

void CleanupBackend(struct GameInfo *info) {
  UnloadGameData();
  if (info && info->initialised) {
    info->initialised = false;
    free((void *)info->rooms);
  }
}
