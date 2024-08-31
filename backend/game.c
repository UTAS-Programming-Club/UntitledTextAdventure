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
    goto unload_data;
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

// TODO: Move to entities.c?
static enum InputOutcome HandleGameCombat(const struct GameInfo *restrict info, struct GameState *restrict state, size_t attackedEnemyID) {
  if (!state->combatInfo.performingEnemyAttacks) {
    if (!PlayerPerformAttack(info, state, attackedEnemyID)) {
      return InvalidInputOutcome;
    }

    state->combatInfo.performingEnemyAttacks = true;
    state->combatInfo.currentEnemyID = 0;
    return GetNextOutputOutcome;
  }

  if (state->combatInfo.performingEnemyAttacks) {
    size_t *curEnemyID = &state->combatInfo.currentEnemyID;
    while (0 == state->combatInfo.enemies[*curEnemyID].health && *curEnemyID < state->combatInfo.enemyCount) {
      ++*curEnemyID;
    }
    if (*curEnemyID < state->combatInfo.enemyCount && !EnemyPerformAttack(info, state)) {
      return InvalidInputOutcome;
    }
    ++*curEnemyID;

    if (*curEnemyID >= state->combatInfo.enemyCount) {
      state->combatInfo.performingEnemyAttacks = false;
    }

    return GetNextOutputOutcome;
  }

  return InvalidInputOutcome;
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
        uint_fast8_t chance = rand() % MaximumEntityStat + 1;
        if(state->roomInfo->eventPercentageChance > chance) {
          ModifyEntityStat(&state->playerInfo.health, state->roomInfo->eventStatChange);
        }
        return GetNextOutputOutcome;
      case GameOpenChestOutcome: ;
        size_t openedChestVarOffset = GetGameStateOffset(state->screenID, 1);
        if (openedChestVarOffset == SIZE_MAX) {
          return InvalidInputOutcome;
        }
        uint8_t *pOpenedChest = (uint8_t *)(state->stateData + openedChestVarOffset);
        *pOpenedChest = 1;
        return GetNextOutputOutcome;
      case GameSwapEquipmentOutcome: ;
        EquipmentID curID = GetEquippedItemID(&state->playerInfo, button.equipmentType);
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
          break;
        }
        return GetNextOutputOutcome;
      case GameFightEnemiesOutcome:
        return HandleGameCombat(info, state, button.enemyID);
      case QuitGameOutcome:
        return QuitGameOutcome;
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
  } else if (NoneScreenInputType == state->inputType) {
    switch (state->screenID) {
      case CombatScreen:
        return HandleGameCombat(info, state, SIZE_MAX);
      default:
        return InvalidInputOutcome;
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
    info->floorSize = 0;
    info->rooms = NULL;
    free((void *)info->equipment);
    info->equipment = NULL;
    free((void *)info->enemyAttacks);
    info->enemyAttackCount = 0;
    info->enemyAttacks = NULL;
  }
}
