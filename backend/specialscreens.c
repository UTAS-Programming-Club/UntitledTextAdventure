#include <arena.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "parser.h"
#include "save.h"

// Each room take 6x4 but the 6 required calls to WriteRoomRow per room only
// handle the top left most 5x3 unless it is the right and/or bottom most room
#define RoomGridSizeHor 6
#define RoomGridSizeVer 4

#define HorLine "─"
#define VerLine "│"
#define UpperHalfVerLine "╵"
#define LowerHalfVerLine "╷"

#ifdef _DEBUG
static const char *TopGridRowChars[] =    {"┌", "┬", "┐"};
static const char *MiddleGridRowChars[] = {"├", "┼", "┤"};
static const char *BottomGridRowChars[] = {"└", "┴", "┘"};

static void FPrintRep(char *sym, uint_fast8_t count, FILE *fp) {
  for (uint_fast8_t i = 0; i < count; ++i) {
    fputs(sym, fp);
  }
}

// TODO: Find a better way to mark rooms that don't exist
// TODO: Indicate room type
// TODO: Fix room openings for other sizes or remove resizing support
static void WriteRoomRow(FILE *fp, RoomCoord roomX, RoomCoord roomY, uint_fast8_t outputRow,
                         const struct GameInfo *info, const struct RoomInfo *playerRoom) {
  if (RoomGridSizeVer - 1 == outputRow && 0 != roomY) {
    return;
  }

  // Top, middle and bottom grid rows
  if (0 == outputRow || RoomGridSizeVer - 1 == outputRow) {
    const char **rowChars = NULL;
    if (info->floorSize - 1 == roomY) {
      rowChars = TopGridRowChars;
    } else if (0 < roomY || 0 == outputRow) {
      rowChars = MiddleGridRowChars;
    } else {
      rowChars = BottomGridRowChars;
    }

    if (0 == roomX) {
      fputs(rowChars[0], fp);
    } else {
      fputs(rowChars[1], fp);
    }

    if (0 == outputRow && GetGameRoom(info, roomX, roomY + 1)->type != InvalidRoomType) {
      fprintf(fp, HorLine "%*s" HorLine, RoomGridSizeHor - 4, "");
    } else {
      FPrintRep(HorLine, RoomGridSizeHor - 2, fp);
    }

    if (info->floorSize - 1 == roomX) {
      fputs(rowChars[2], fp);
      fputc('\n', fp);
    }
  }

  // Middle Room Rows
  else {
    enum RoomType roomExists = GetGameRoom(info, roomX, roomY)->type != InvalidRoomType;

    char *wallChar;
    if (!roomExists || GetGameRoom(info, roomX - 1, roomY)->type == InvalidRoomType) {
      wallChar = VerLine;
    } else if (1 == outputRow) {
      wallChar = UpperHalfVerLine;
    } else if (RoomGridSizeVer - 2 == outputRow) {
      wallChar = LowerHalfVerLine;
    } else {
      wallChar = " ";
    }

    // Player in room
    if (playerRoom->x == roomX && playerRoom->y == roomY && 1 == outputRow) {
      fprintf(fp, "%sP%*s", wallChar, RoomGridSizeHor - 3, "");
    // Room exists
    } else if (roomExists) {
      fprintf(fp, "%s%*s", wallChar, RoomGridSizeHor - 2, "");
    // Room does not exist
    } else {
      fprintf(fp, "%sNO%*s", wallChar, RoomGridSizeHor - 4, "");
    }

    if (info->floorSize - 1 == roomX) {
      fputs(VerLine "\n", fp);
    }
  }
}

static void WriteMap(const struct GameInfo *info, const struct RoomInfo *currentRoom) {
  FILE *fp = fopen("Map.txt", "w");
  if (!fp) {
    return;
  }

  for (RoomCoord roomY = info->floorSize - 1; roomY != InvalidRoomCoord; --roomY) {
    for (uint_fast8_t outputRow = 0; outputRow < RoomGridSizeVer; ++outputRow) {
      for (RoomCoord roomX = 0; roomX < info->floorSize; ++roomX) {
        WriteRoomRow(fp, roomX, roomY, outputRow, info, currentRoom);
      }
    }
  }

  fclose(fp);
}
#endif


// TODO: Use arena's arena_sprintf?
static char *CreateString(Arena *arena, const char *restrict format, ...) {
  va_list args1, args2;
  va_start(args1, format);
  va_copy(args2, args1);

  char *res = NULL;

  int allocatedCharCount = vsnprintf(NULL, 0, format, args1);
  va_end(args1);
  if (allocatedCharCount <= 0) {
    goto cleanup;
  }
  ++allocatedCharCount;

  res = arena_alloc(arena, allocatedCharCount);
  if (!res) {
    goto cleanup;
  }

  if (vsnprintf(res, allocatedCharCount, format, args2) <= 0) {
    free(res);
    res = NULL;
  }

cleanup:
  va_end(args2);
  return res;
}


static bool CreateMainMenuScreen(const struct GameInfo *info, struct GameState *state) {
  (void)info;

  size_t reloadCountVarOffset = GetGameStateOffset(state->screenID, 0);
  if (reloadCountVarOffset == SIZE_MAX) {
    return false;
  }

  state->startedGame = false;

  uint32_t *pReloadCount = (uint32_t *)(state->stateData + reloadCountVarOffset);

  if (*pReloadCount) {
    struct GameScreen screen = {0};
    if (!GetGameScreen(state->screenID, &screen)) {
      return false;
    }

    state->body = CreateString(&state->arena, "%s%s%" PRIu32, screen.body, screen.extraText, *pReloadCount);
    if (!state->body) {
      return false;
    }
  }

  ++(*pReloadCount);
  return true;
}

static bool CreateGameScreen(const struct GameInfo *info, struct GameState *state) {
  // TODO: Remove
  static char bodyBeginning[] = "This is the game, you are in room [";
  static char bodyMiddle[] = ", ";
  static char bodyEnding[] = "].";

  if (!state->startedGame && !CreateNewState(info, state)) {
    return false;
  }

#ifdef _DEBUG
  WriteMap(info, state->roomInfo);
#endif

  size_t openedChestVarOffset = GetGameStateOffset(state->screenID, 1);
  if (openedChestVarOffset == SIZE_MAX) {
    return InvalidInputOutcome;
  }
  uint8_t *pOpenedChest = (uint8_t *)(state->stateData + openedChestVarOffset);

  char *roomInfoStr = "";
  switch (state->roomInfo->type) {
    // TODO: Add other options w/ extra info such as failing etc
    case HealthChangeRoomType:
      roomInfoStr = CreateString(&state->arena, "\n\n%s.", state->roomInfo->eventDescription);
      if (!roomInfoStr) {
        return false;
      }
      break;
    // TODO: Use data from json
    case CustomChestRoomType:
      if(*pOpenedChest == 1) {
        roomInfoStr = CreateString(&state->arena, "\n\nYou open the chest and recieve a mythril vest.");
        if (!roomInfoStr) {
          return false;
        }
        UnlockItem(&state->playerInfo, 11);
        *pOpenedChest = 2;
      }
      break;
    default:
      break;
  }


  state->body = CreateString(&state->arena, "%s%" PRIRoomCoord "%s%" PRIRoomCoord "%s%s",
                             bodyBeginning, state->roomInfo->x + 1, bodyMiddle,
                             state->roomInfo->y + 1, bodyEnding, roomInfoStr);
  if (!state->body) {
    return false;
  }

  for (uint_fast8_t i = 0; i < state->inputCount; ++i) {
    switch (state->inputs[i].outcome) {
      case GotoScreenOutcome:
        struct GameScreenButton button = {0};
        if (!GetGameScreenButton(state->screenID, i, &button)) {
          return false;
        }
        
        switch (button.newScreenID) {
          case CombatScreen:
            state->inputs[i].visible = state->roomInfo->type == CombatRoomType;
            break;
        }
        break;
      case GameGoNorthOutcome:
        state->inputs[i].visible =
          GetGameRoom(info, state->roomInfo->x, state->roomInfo->y + 1)->type != InvalidRoomType;
        break;
      case GameGoEastOutcome:
        state->inputs[i].visible =
          GetGameRoom(info, state->roomInfo->x + 1, state->roomInfo->y)->type != InvalidRoomType;
        break;
      case GameGoSouthOutcome:
        state->inputs[i].visible =
          GetGameRoom(info, state->roomInfo->x, state->roomInfo->y - 1)->type != InvalidRoomType;
        break;
      case GameGoWestOutcome:
        state->inputs[i].visible =
          GetGameRoom(info, state->roomInfo->x - 1, state->roomInfo->y)->type != InvalidRoomType;
        break;
      case GameHealthChangeOutcome:
        state->inputs[i].visible = state->roomInfo->type == HealthChangeRoomType;
        break;
      case GameOpenChestOutcome:
        state->inputs[i].visible = *pOpenedChest == 0 && state->roomInfo->type == CustomChestRoomType;
        break;
      default:
        break;
    }
  }

  return true;
}

static bool CreatePlayerStatsScreen(const struct GameInfo *info, struct GameState *state) {
  (void)info;

  struct GameScreen screen = {0};
  if (!GetGameScreen(state->screenID, &screen)) {
    return false;
  }

  state->body = CreateString(&state->arena, "%s\n\n"
                                    "Health: %" PRIPlayerStat "\n"
                                    "Stamina: %" PRIPlayerStat "\n"
                                    "Physical Attack: %" PRIPlayerStat "\n"
                                    "Magic Attack: %" PRIPlayerStat "\n"
                                    "Physical Defence: %" PRIPlayerStat "\n"
                                    "Magic Defence: %" PRIPlayerStat,
                             screen.body,
                             state->playerInfo.health, state->playerInfo.stamina,
                             state->playerInfo.physAtk, state->playerInfo.magAtk,
                             state->playerInfo.physDef, state->playerInfo.magDef
  );
  if (!state->body) {
    return false;
  }

  return true;
}

static bool CreateSaveScreen(const struct GameInfo *info, struct GameState *state) {
  (void)info;

  struct GameScreen screen = {0};
  if (!GetGameScreen(state->screenID, &screen)) {
    return false;
  }

  const char *password = SaveState(state);
  if (!password) {
    return false;
  }

  state->body = CreateString(&state->arena, "%s%s", screen.body, password);
  if (!state->body) {
    return false;
  }

  return true;
}

static bool CreatePlayerEquipmentScreen(const struct GameInfo* info, struct GameState* state) {
  struct GameScreen screen = { 0 };
  if (!GetGameScreen(state->screenID, &screen)) {
    return false;
  }

  struct EquipmentInfo *slot0 = GetEquippedItem(info, &state->playerInfo, 0);
  struct EquipmentInfo *slot1 = GetEquippedItem(info, &state->playerInfo, 1);
  struct EquipmentInfo *slot2 = GetEquippedItem(info, &state->playerInfo, 2);
  struct EquipmentInfo *slot3 = GetEquippedItem(info, &state->playerInfo, 3);
  struct EquipmentInfo *slot4 = GetEquippedItem(info, &state->playerInfo, 4);
  struct EquipmentInfo *slot5 = GetEquippedItem(info, &state->playerInfo, 5);
  struct EquipmentInfo *slot6 = GetEquippedItem(info, &state->playerInfo, 6);
  if (!slot0 || !slot1 || !slot2 || !slot3 || !slot4 || !slot5 || !slot6) {
    return false;
  }

  state->body = CreateString(&state->arena, "%s\n\n"
    "Health: %" PRIPlayerStat "\n"
    "Stamina: %" PRIPlayerStat "\n"
    "Physical Attack: %" PRIPlayerStat "\n"
    "Magic Attack: %" PRIPlayerStat "\n"
    "Physical Defence: %" PRIPlayerStat "\n"
    "Magic Defence: %" PRIPlayerStat "\n\n"
    "Helmet: %s\n"
    "Chest: %s\n"
    "Gloves: %s\n"
    "Pants: %s\n"
    "Boots: %s\n"
    "Primary Weapon: %s\n"
    "Secondary Weapon: %s\n",
    screen.body,
    state->playerInfo.health, state->playerInfo.stamina,
    state->playerInfo.physAtk, state->playerInfo.magAtk,
    state->playerInfo.physDef, state->playerInfo.magDef,
    slot0->name, slot1->name, slot2->name, slot3->name,
    slot4->name, slot5->name, slot6->name
  );
  if (!state->body) {
    return false;
  }

  return true;
}

static bool CreateCombatScreen(const struct GameInfo *info, struct GameState *state) {
  (void)info;

  struct GameScreen screen = {0};
  if (!GetGameScreen(state->screenID, &screen)) {
    return false;
  }

  state->body = CreateString(&state->arena, "%s\n\n"
    "Health: %" PRIPlayerStat "\n"
    "Stamina: %" PRIPlayerStat "\n",
    screen.body,
    state->playerInfo.health, state->playerInfo.stamina
  );
  if (!state->body) {
    return false;
  }

  return true;
}


// TODO: Add GameScreen parameter
// TODO: Allow loading these dynamically to support adding new custom rooms? If so the api needs to be versioned
// Must match the order of the CustomScreenCode enum in types.h
bool (*CustomScreenCode[])(const struct GameInfo *, struct GameState *) = {
  CreateMainMenuScreen,
  CreateGameScreen,
  CreatePlayerStatsScreen,
  CreateSaveScreen,
  CreatePlayerEquipmentScreen,
  CreateCombatScreen
};
size_t CustomScreenCodeCount = sizeof CustomScreenCode / sizeof *CustomScreenCode;
