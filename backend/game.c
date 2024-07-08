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
#include "save.h"
#include "screens.h"
#include "specialscreens.h"

static const struct RoomInfo DefaultRoom = {.type = InvalidRoomType};

bool SetupBackend(struct GameInfo *info) {
  if (!info) {
    PrintError("Required game info struct is inaccessable");
    return false;
  }

  if (info->initialised) {
    goto end;
  }

  if (info->rooms || info->equipment) {
    PrintError("Parts of the game info struct are already initialised");
    goto end;
  }

  char dataFile[] = "GameData.json";

  if (!LoadGameData(dataFile)) {
    PrintError("Failed to load %s", dataFile);
    goto end;
  }

  info->name = LoadGameName();
  if (!info->name) {
    PrintError("Failed to load game name from %s", dataFile);
    goto end;
  }

  if (!LoadDefaultPlayerStats(&info->defaultPlayerStats)) {
    PrintError("Failed to load default player stats from %s", dataFile);
    goto end;
  }

  if (!LoadGameRooms(&info->floorSize, &info->rooms)) {
    PrintError("Failed to load rooms from %s", dataFile);
    goto free_rooms;
  }

  if (!LoadGameEquipment(&info->equipmentCount, &info->equipment)) {
    PrintError("Failed to load equipment from %s", dataFile);
    goto free_rooms;
  }

  if (!LoadGameStateSaveStatus(&info->stateSaveStatus)) {
    PrintError("Failed to load state info from %s", dataFile);
    goto free_stateSaveStatus;
  }

  unsigned int currentTimestamp = time(NULL);
  srand(currentTimestamp);

  info->initialised = true;
  goto end;

free_stateSaveStatus:
  free(info->stateSaveStatus);
  info->stateSaveStatus = NULL;
free_rooms:
  free(info->rooms);
  info->rooms = NULL;

end:
  return info->initialised;
}

static bool UpdatePlayerStat(PlayerStat *base, PlayerStatDiff diff) {
  if (!base || MaximumPlayerStat < *base
      || MinimumPlayerStatDiff > diff || MaximumPlayerStatDiff < diff) {
    return false;
  }

  if (0 > diff && MinimumPlayerStat > *base + diff) {
    *base = MinimumPlayerStat;
  } else if (0 < diff && MaximumPlayerStat < *base + diff) {
    *base = MaximumPlayerStat;
  } else {
    *base += diff;
  }
  return true;
}

// TODO: Replace test with real impl with pickups
void UpdateStats(struct GameState *state) {
  state->playerInfo.physAtk = MinimumPlayerStat;
  state->playerInfo.magAtk = MinimumPlayerStat;
  state->playerInfo.physDef = MinimumPlayerStat;
  state->playerInfo.magDef = MinimumPlayerStat;

  for (EquipmentID i = 0; i < EquippedItemsSlots; ++i) {
    const struct EquipmentInfo *item = state->playerInfo.equippedItems[i];
    if (!item) {
      continue;
    }

    UpdatePlayerStat(&state->playerInfo.physAtk, item->physAtkMod);
    UpdatePlayerStat(&state->playerInfo.magAtk,  item->magAtkMod);
    UpdatePlayerStat(&state->playerInfo.physDef, item->physDefMod);
    UpdatePlayerStat(&state->playerInfo.magDef,  item->magDefMod);
  }
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

enum InputOutcome HandleGameInput(const struct GameInfo *info, struct GameState *state,
                                  uint_fast8_t buttonInputIndex, const char *textInput) {
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
      case GameHealthChangeOutcome: ;
        // chance to dodge the trap else take damage
        // TODO: Ensure this only trigger once, track room completion?
        // TODO: End game when health is 0
        // eventPercentageChance is (0, 100] so chance must be as well
        uint_fast8_t chance = rand() % MaximumPlayerStat + 1;
        if(state->roomInfo->eventPercentageChance > chance) {
          UpdatePlayerStat(&state->playerInfo.health, state->roomInfo->eventStatChange);
        }
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
    free(info->rooms);
    info->rooms = NULL;
    free(info->equipment);
    info->equipment = NULL;
    free(info->stateSaveStatus);
    info->stateSaveStatus = NULL;
  }
}
