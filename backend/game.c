#include <arena.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <types.h>

#include "../frontends/frontend.h"
#include "entities.h"
#include "equipment.h"
#include "game.h"
#include "parser.h"
#include "save.h"
#include "screens.h"
#include "specialscreens.h"

static const struct RoomInfo DefaultRoom = {.type = InvalidRoomType};

static enum InputOutcome ChangeGameRoom(const struct GameInfo *restrict, struct GameState *restrict,
                                        RoomCoord, RoomCoord);

bool SetupBackend(struct GameInfo *info) {
  if (!info) {
    PrintError("Required game info struct is inaccessable");
    return false;
  }

  if (info->initialised) {
    goto end;
  }

  if (info->rooms || info->equipment || info->enemyAttacks) {
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
    goto unload_data;
  }

  if (!LoadDefaultPlayerInfo(&info->defaultPlayerInfo)) {
    PrintError("Failed to load default player info from %s", dataFile);
    goto unload_data;
  }

  struct RoomInfo *rooms = NULL;
  if (!LoadGameRooms(&info->floorSize, &rooms)) {
    PrintError("Failed to load rooms from %s", dataFile);
    goto free_rooms;
  }
  info->rooms = rooms;

  struct EquipmentInfo *equipment;
  if (!LoadGameEquipment(&equipment)) {
    PrintError("Failed to load equipment from %s", dataFile);
    goto free_rooms;
  }
  info->equipment = equipment;

  struct EnemyAttackInfo *enemyAttacks;
  if (!LoadGameEnemyAttacks(&info->enemyAttackCount, &enemyAttacks)) {
    PrintError("Failed to load enemy attacks from %s", dataFile);
    goto free_equipment;
  }
  info->enemyAttacks = enemyAttacks;

  unsigned int currentTimestamp = time(NULL);
  srand(currentTimestamp);

  info->initialised = true;
  goto end;

free_equipment:
  free(equipment);
  info->equipment = NULL;

free_rooms:
  free(rooms);
  info->floorSize = 0;
  info->rooms = NULL;

unload_data:
  UnloadGameData();

end:
  return info->initialised;
}

bool UpdateGameState(const struct GameInfo *info, struct GameState *state) {
  if (!info || !info->initialised || !state || state->screenID == InvalidScreen) {
    return false;
  }

  if (!state->stateData && !InitGameState(&state->stateDataSize, &state->stateData)) {
    return false;
  }

  if (!state->roomData) {
    state->roomDataSize = info->floorSize * info->floorSize;
    state->roomData = calloc(sizeof *state->roomData, state->roomDataSize);
  }
  if (!state->roomData) {
    return false;
  }

  arena_reset(&state->arena);

  // TODO: Move to input handling
  if (state->startedGame && 0 == state->playerInfo.health) {
    state->screenID = PlayerDeathScreen;
    state->startedGame = false;
  }

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

    const struct RoomInfo *currentRoom = GetCurrentGameRoom(info, state);
    if (currentRoom->type == InvalidRoomType) {
      return InvalidInputOutcome;
    }

    switch (button.outcome) {
      case GameCombatLeaveOutcome:
        state->combatInfo.inCombat = false;
        state->roomData[currentRoom->y * info->floorSize * currentRoom->x] = true;
        /* fallthrough */
      case GotoScreenOutcome:
        state->previousScreenID = state->screenID;
        state->screenID = button.newScreenID;
        return GetNextOutputOutcome;
      case GotoPreviousScreenOutcome:
        state->screenID = state->previousScreenID;
        state->previousScreenID = InvalidScreen; // GameScreen?
        return GetNextOutputOutcome;
      case GameGoNorthOutcome:
        state->previousRoomID = state->roomID;
        return ChangeGameRoom(info, state, currentRoom->x, currentRoom->y + 1);
      case GameGoEastOutcome:
        state->previousRoomID = state->roomID;
        return ChangeGameRoom(info, state, currentRoom->x + 1, currentRoom->y);
      case GameGoSouthOutcome:
        state->previousRoomID = state->roomID;
        return ChangeGameRoom(info, state, currentRoom->x, currentRoom->y - 1);
      case GameGoWestOutcome:
        state->previousRoomID = state->roomID;
        return ChangeGameRoom(info, state, currentRoom->x - 1, currentRoom->y);
      case GameHealthChangeOutcome: ;
        // chance to dodge the trap else take damage
        // TODO: Ensure this only trigger once, track room completion?
        // TODO: End game when health is 0
        // eventPercentageChance is (0, 100] so chance must be as well
        uint_fast8_t chance = rand() % MaximumEntityStat + 1;
        if (currentRoom->eventPercentageChance > chance) {
          ModifyEntityStat(&state->playerInfo.health, currentRoom->eventStatChange);
        }
        return GetNextOutputOutcome;
      case GameOpenChestOutcome:
        state->roomData[currentRoom->y * info->floorSize * currentRoom->x] = true;
        return GetNextOutputOutcome;
      case GameSwapEquipmentOutcome: ;
        EquipmentID origID = GetEquippedItemID(&state->playerInfo, button.equipmentType);
        EquipmentID curID = origID;
        if (InvalidEquipmentID == curID) {
          return InvalidInputOutcome;
        }
        ++curID; // Start at next item to avoid the loop always stopping at the current item

        EquipmentID minID = EquipmentPerTypeCount * (curID / EquipmentPerTypeCount);
        EquipmentID maxID = minID + EquipmentPerTypeCount;

        for (uint_fast8_t slotsChecked = 0; slotsChecked < EquipmentPerTypeCount; ++slotsChecked, ++curID) {
          if (curID == maxID) {
            curID = minID;
          }

          bool itemUnlocked;
          if (!CheckItemUnlocked(&state->playerInfo, curID, &itemUnlocked)) {
            return InvalidInputOutcome;
          }
          if (!itemUnlocked) {
             continue;
          }

          if (!SetEquippedItem(&state->playerInfo, button.equipmentType, curID)
              || !RefreshPlayerStats(info, state)) {
            return InvalidInputOutcome;
          }

          state->combatInfo.changedEquipment |= state->combatInfo.inCombat && origID != curID;
          break;
        }

        return GetNextOutputOutcome;
      case GameCombatFightOutcome:
        return HandleGameCombat(info, state, button.enemyID);
      case GameCombatFleeOutcome:
        state->screenID = GameScreen;
        state->combatInfo.inCombat = false;
        state->roomID = state->previousRoomID;
        state->previousRoomID = SIZE_MAX;
        return GetNextOutputOutcome;
      case QuitGameOutcome:
        return QuitGameOutcome;
      case GetNextOutputOutcome:
      case InvalidInputOutcome:
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
  } else if (NoneScreenInputType == state->inputType) {
    switch (state->screenID) {
      case CombatScreen:
        return HandleGameCombat(info, state, SIZE_MAX);
      case GameScreen:
      case InvalidScreen:
      case LoadScreen:
      case MainMenuScreen:
      case PlayerDeathScreen:
      case PlayerEquipmentScreen:
      case SaveScreen:
        return InvalidInputOutcome;
    }
  }

  return InvalidInputOutcome;
}

size_t GetGameRoomID(const struct GameInfo *restrict info, RoomCoord x, RoomCoord y) {
  if (!info || !info->initialised
      || x >= info->floorSize || y >= info->floorSize
      || x == InvalidRoomCoord || y == InvalidRoomCoord) {
    return SIZE_MAX;
  }

  size_t roomID = y * info->floorSize + x;
  if (info->rooms[roomID].type == InvalidRoomType) {
    return SIZE_MAX;
  }

  return roomID;
}

const struct RoomInfo *GetCurrentGameRoom(const struct GameInfo *restrict info, const struct GameState *restrict state) {
  if (!info || !info->initialised || !state) {
    return &DefaultRoom;
  }

  return &info->rooms[state->roomID];
}

static enum InputOutcome ChangeGameRoom(const struct GameInfo *restrict info, struct GameState *restrict state,
                                        RoomCoord x, RoomCoord y) {
  size_t roomID = GetGameRoomID(info, x, y);
  if (SIZE_MAX == roomID) {
    return InvalidInputOutcome;
  }
  state->roomID = roomID;

  if (CombatRoomType == info->rooms[roomID].type && !state->roomData[roomID]) {
    state->previousScreenID = state->screenID;
    state->screenID = CombatScreen;
  }

  return GetNextOutputOutcome;
}

void CleanupGame(struct GameState *state) {
  if (!state) {
    return;
  }

  arena_free(&state->arena);
  free(state->roomData);
  state->roomDataSize = 0;
  state->roomData = NULL;
  free(state->stateData);
  state->stateDataSize = 0;
  state->stateData = NULL;
}

void CleanupBackend(struct GameInfo *info) {
  UnloadGameData();
  if (!info || !info->initialised) {
    return;
  }

  info->initialised = false;
  free((void *)info->rooms);
  info->floorSize = 0;
  info->rooms = NULL;
  free((void *)info->equipment);
  info->equipment = NULL;
  free((void *)info->enemyAttacks);
  info->enemyAttackCount = 0;
  info->enemyAttacks = NULL;
}
