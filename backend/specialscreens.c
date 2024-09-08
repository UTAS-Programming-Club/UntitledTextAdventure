#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <types.h>

#include "entities.h"  // CreateCombatString
#include "equipment.h" // struct EquipmentInfo, GetEquippedItem, UnlockItem
#include "game.h"
#include "parser.h"
#include "save.h"
#include "specialscreens.h"
#include "stringhelpers.h"

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

    if (0 == outputRow && GetGameRoomID(info, roomX, roomY + 1) != SIZE_MAX) {
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
    enum RoomType roomExists = GetGameRoomID(info, roomX, roomY) != SIZE_MAX;

    char *wallChar;
    if (!roomExists || GetGameRoomID(info, roomX - 1, roomY) == SIZE_MAX) {
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

  const struct RoomInfo *currentRoom = GetCurrentGameRoom(info, state);
  if (currentRoom->type == InvalidRoomType) {
    return InvalidInputOutcome;
  }

#ifdef _DEBUG
  WriteMap(info, currentRoom);
#endif

  size_t openedChestVarOffset = GetGameStateOffset(state->screenID, 1);
  if (openedChestVarOffset == SIZE_MAX) {
    return InvalidInputOutcome;
  }
  uint8_t *pOpenedChest = (uint8_t *)(state->stateData + openedChestVarOffset);

  const char *roomInfoStr = "";
  switch (currentRoom->type) {
    case CombatRoomType:
    case EmptyRoomType:
      break;
    // TODO: Add other options w/ extra info such as failing etc
    case HealthChangeRoomType:
      roomInfoStr = CreateString(&state->arena, "\n\n%s.", currentRoom->eventDescription);
      if (!roomInfoStr) {
        return false;
      }
      break;
    // TODO: Use data from json
    case CustomChestRoomType:
      if (*pOpenedChest == 1) {
        roomInfoStr = CreateString(&state->arena, "\n\nYou open the chest and recieve a mythril vest.");
        if (!roomInfoStr) {
          return false;
        }
        UnlockItem(&state->playerInfo, 11);
        *pOpenedChest = 2;
      }
      break;
    case InvalidRoomType:
      return false;
  }

  state->body = CreateString(&state->arena, "%s%" PRIRoomCoord "%s%" PRIRoomCoord "%s%s",
                             bodyBeginning, currentRoom->x + 1, bodyMiddle,
                             currentRoom->y + 1, bodyEnding, roomInfoStr);
  if (!state->body) {
    return false;
  }

  for (uint_fast8_t i = 0; i < state->inputCount; ++i) {
    switch (state->inputs[i].outcome) {
      case GameCombatFightOutcome:
      case GameCombatFleeOutcome:
      case GameCombatLeaveOutcome:
      case GameSwapEquipmentOutcome:
      case GetNextOutputOutcome:
      case GotoPreviousScreenOutcome:
      case QuitGameOutcome:
        break;
      case GotoScreenOutcome: ;
        struct GameScreenButton button = {0};
        if (!GetGameScreenButton(state->screenID, i, &button)) {
          return false;
        }

        switch (button.newScreenID) {
          case CombatScreen:
            state->inputs[i].visible = currentRoom->type == CombatRoomType;
            break;
          default:
            break;
        }
        break;
      case GameGoNorthOutcome:
        state->inputs[i].visible =
          GetGameRoomID(info, currentRoom->x, currentRoom->y + 1) != SIZE_MAX;
        break;
      case GameGoEastOutcome:
        state->inputs[i].visible =
          GetGameRoomID(info, currentRoom->x + 1, currentRoom   ->y) != SIZE_MAX;
        break;
      case GameGoSouthOutcome:
        state->inputs[i].visible =
          GetGameRoomID(info, currentRoom->x, currentRoom->y - 1) != SIZE_MAX;
        break;
      case GameGoWestOutcome:
        state->inputs[i].visible =
          GetGameRoomID(info, currentRoom->x - 1, currentRoom->y) != SIZE_MAX;
        break;
      case GameHealthChangeOutcome:
        state->inputs[i].visible = currentRoom->type == HealthChangeRoomType;
        break;
      case GameOpenChestOutcome:
        state->inputs[i].visible = *pOpenedChest == 0 && currentRoom->type == CustomChestRoomType;
        break;
      case InvalidInputOutcome:
        return false;
    }
  }

  return true;
}

static bool CreateSaveScreen(const struct GameInfo *info, struct GameState *state) {
  (void)info;

  struct GameScreen screen = {0};
  if (!GetGameScreen(state->screenID, &screen)) {
    return false;
  }

  const char *password = SaveState(info, state);
  if (!password) {
    return false;
  }

  state->body = CreateString(&state->arena, "%s%s", screen.body, password);
  if (!state->body) {
    return false;
  }

  return true;
}

// TODO: Use bars for PlayerStats and PlayerStatDiffs
// TODO: Show agility
static bool CreatePlayerEquipmentScreen(const struct GameInfo* info, struct GameState* state) {
  struct GameScreen screen = { 0 };
  if (!GetGameScreen(state->screenID, &screen)) {
    return false;
  }

  const struct EquipmentInfo *slot0 = GetEquippedItem(info, &state->playerInfo, HelmetEquipmentType);
  const struct EquipmentInfo *slot1 = GetEquippedItem(info, &state->playerInfo, ChestPieceEquipmentType);
  const struct EquipmentInfo *slot2 = GetEquippedItem(info, &state->playerInfo, GlovesEquipmentType);
  const struct EquipmentInfo *slot3 = GetEquippedItem(info, &state->playerInfo, PantsEquipmentType);
  const struct EquipmentInfo *slot4 = GetEquippedItem(info, &state->playerInfo, BootsEquipmentType);
  const struct EquipmentInfo *slot5 = GetEquippedItem(info, &state->playerInfo, PriWeapEquipmentType);
  const struct EquipmentInfo *slot6 = GetEquippedItem(info, &state->playerInfo, SecWeapEquipmentType);
  if (!slot0 || !slot1 || !slot2 || !slot3 || !slot4 || !slot5 || !slot6) {
    return false;
  }

  state->body = CreateString(&state->arena, "%s\n\n"
    "Health: %" PRIEntityStat "\n"
    "Stamina: %" PRIEntityStat "\n"
    "Primary Physical Attack: %" PRIEntityStatDiff "\n"
    "Primary Magic Attack: %" PRIEntityStatDiff "\n"
    "Secondary Physical Attack: %" PRIEntityStatDiff "\n"
    "Secondary Magic Attack: %" PRIEntityStatDiff "\n"
    "Physical Defence: %" PRIEntityStat "\n"
    "Magic Defence: %" PRIEntityStat "\n\n"
    "Helmet: %s\n"
    "Chest: %s\n"
    "Gloves: %s\n"
    "Pants: %s\n"
    "Boots: %s\n"
    "Primary Weapon: %s\n"
    "Secondary Weapon: %s\n",
    screen.body,
    state->playerInfo.health, state->playerInfo.stamina,
    -state->playerInfo.priPhysAtk, -state->playerInfo.priMagAtk,
    -state->playerInfo.secPhysAtk, -state->playerInfo.secMagAtk,
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

  if (!state->combatInfo.inCombat && !StartCombat(info, state)) {
    return false;
  }

  if (!UpdateCombat(state)) {
    return false;
  }

  if (!state->combatInfo.performingEnemyAttacks && state->combatInfo.playerWon) {
    // TODO: Move to json?
    state->body = "You have won!";
  } else {
    state->body = CreateCombatString(info, state);
  }
  if (!state->body) {
    return false;
  }

  // TODO: Allow changing weapons during combat
  // TODO: Add health, stamina potions

  for (uint_fast8_t i = 0; i < state->inputCount; ++i) {
    struct GameScreenButton button = {0};
    if (!GetGameScreenButton(state->screenID, i, &button)) {
      return false;
    }

    if (GameCombatFleeOutcome == button.outcome && GameScreen == button.newScreenID) {
      state->inputs[i].visible = !state->combatInfo.playerWon;
      continue;
    }

    if (GameCombatLeaveOutcome == button.outcome && GameScreen == button.newScreenID) {
      state->inputs[i].visible = state->combatInfo.playerWon;
      continue;
    }

    if (state->inputs[i].outcome != GameCombatFightOutcome) {
      continue;
    }

    if (button.enemyID >= state->combatInfo.enemyCount) {
      state->inputs[i].visible = false;
      continue;
    }

    // TODO: Disable rather than hide to allow preplaning moves?
    // Currently if you press multiple buttons at once, the cmd frontend runs them all in order but
    // if an enemy dies it screws up the button ordering from then on and so causes undesired moves
    state->inputs[i].visible = 0 != state->combatInfo.enemies[button.enemyID].health;
    state->inputs[i].title = CreateString(&state->arena, "%s%zu", button.title, button.enemyID + 1);
    if (!state->inputs[i].title) {
      return false;
    }
  }

  if (state->combatInfo.performingEnemyAttacks) {
    state->inputType = NoneScreenInputType;
  }

  return true;
}


// TODO: Add GameScreen parameter
// TODO: Allow loading these dynamically to support adding new custom rooms? If so the api needs to be versioned
// Must match the order of the CustomScreenCode enum in types.h
bool (*CustomScreenCode[])(const struct GameInfo *, struct GameState *) = {
  CreateMainMenuScreen,
  CreateGameScreen,
  CreateSaveScreen,
  CreatePlayerEquipmentScreen,
  CreateCombatScreen
};
size_t CustomScreenCodeCount = sizeof CustomScreenCode / sizeof *CustomScreenCode;
