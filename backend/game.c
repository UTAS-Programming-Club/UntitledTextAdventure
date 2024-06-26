#include <arena.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "../frontends/frontend.h"
#include "game.h"
#include "parser.h"
#include "save.h"
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
    bool result = CustomScreenCode[state->customScreenCodeID](info, state);
    if (!result) {
      PrintError("Custom screen code for screen %i failed", state->screenID);
    }
    return result;
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

enum InputOutcome HandleGameInput(const struct GameInfo *info, struct GameState *state, uint_fast8_t buttonInputIndex, const char *textInput) {
  if (!info || !info->initialised || !state) {
    return InvalidInputOutcome;
  }

  if (ButtonScreenInputType == state->inputType && UINT_FAST8_MAX != buttonInputIndex) {
    uint_fast8_t inputID = MapInputIndex(state, buttonInputIndex);
    if (UINT_FAST8_MAX == inputID) {
      return InvalidInputOutcome;
    }

    struct GameScreenButton button = {0};
    if (!GetGameScreenButton(state->screenID, inputID, &button)) {
      return InvalidInputOutcome;
    }

    switch (button.outcome) {
      case GotoScreenOutcome:
        state->screenID = button.newScreenID;
        return GetNextOutputOutcome;
      case GameGoNorthOutcome:
        state->roomInfo = GetGameRoom(info, state->roomInfo->x, state->roomInfo->y + 1);
        return GetNextOutputOutcome;
      case GameGoEastOutcome:
        state->roomInfo = GetGameRoom(info, state->roomInfo->x + 1, state->roomInfo->y);
        return GetNextOutputOutcome;
      case GameGoSouthOutcome:
        state->roomInfo = GetGameRoom(info, state->roomInfo->x, state->roomInfo->y - 1);
        return GetNextOutputOutcome;
      case GameGoWestOutcome:
        state->roomInfo = GetGameRoom(info, state->roomInfo->x - 1, state->roomInfo->y);
        return GetNextOutputOutcome;
      case QuitGameOutcome:
        return button.outcome;
      default:
        return InvalidInputOutcome;
    }
  } else if (TextScreenInputType == state->inputType && textInput) {
      if ('\x1B' /* ESC */ == textInput[0] && '\0' == textInput[1]) {
        state->screenID = state->previousScreenID;
        return GetNextOutputOutcome;
      } else if (LoadState(info, state, textInput)) {
        state->screenID = state->nextScreenID;
        return GetNextOutputOutcome;
      }
  }

  return InvalidInputOutcome;
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
