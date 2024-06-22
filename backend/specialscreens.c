#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game.h"
#include "parser.h"

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

// TODO: Only show inner walls on rooms that exists
// TODO: Find a better way to mark rooms that don't exist
// TODO: Add openings between rooms that exist
// TODO: Indicate room type
static void WriteRoomRow(FILE *fp, RoomCoord roomRow, RoomCoord roomColumn,
                         uint_fast8_t outputRow, const struct RoomInfo *currentRoom) {
  if (RoomGridSizeVer - 1 == outputRow && 0 != roomRow) {
    return;
  }

  // Top, middle and bottom grid rows
  if (0 == outputRow || RoomGridSizeVer - 1 == outputRow) {
    const char **rowChars = NULL;
    if (FloorSize - 1 == roomRow) {
      rowChars = TopGridRowChars;
    } else if (0 < roomRow || 0 == outputRow) {
      rowChars = MiddleGridRowChars;
    } else {
      rowChars = BottomGridRowChars;
    }

    if (0 == roomColumn) {
      fputs(rowChars[0], fp);
    } else {
      fputs(rowChars[1], fp);
    }

    if (0 == outputRow && GetGameRoom(roomRow + 1, roomColumn)->exists) {
      fprintf(fp, HorLine "%*s" HorLine, RoomGridSizeHor - 4, "");
    } else {
      FPrintRep(HorLine, RoomGridSizeHor - 2, fp);
    }

    if (FloorSize - 1 == roomColumn) {
      fputs(rowChars[2], fp);
      fputc('\n', fp);
    }
  }

  // Middle Room Rows
  else {
    char *wallChar;
    if (!GetGameRoom(roomRow, roomColumn)->exists
        || !GetGameRoom(roomRow, roomColumn - 1)->exists) {
      wallChar = VerLine;
    } else if (1 == outputRow) {
      wallChar = UpperHalfVerLine;
    } else if (RoomGridSizeVer - 2 == outputRow) {
      wallChar = LowerHalfVerLine;
    } else {
      wallChar = " ";
    }

    // Player in room
    if (currentRoom->x == roomColumn && currentRoom->y == roomRow && 1 == outputRow) {
      fprintf(fp, "%sP%*s", wallChar, RoomGridSizeHor - 3, "");
    // Room exists
    } else if (GetGameRoom(roomRow, roomColumn)->exists) {
      fprintf(fp, "%s%*s", wallChar, RoomGridSizeHor - 2, "");
    // Room does not exist
    } else {
      fprintf(fp, "%sNO%*s", wallChar, RoomGridSizeHor - 4, "");
    }

    if (FloorSize - 1 == roomColumn) {
      fputs(VerLine "\n", fp);
    }
  }
}

static void WriteMap(const struct RoomInfo *currentRoom) {
  FILE *fp = fopen("Map.txt", "w");
  if (!fp) {
    return;
  }

  for (RoomCoord roomRow = FloorSize - 1; roomRow != InvalidRoomCoord; --roomRow) {
    for (uint_fast8_t outputRow = 0; outputRow < RoomGridSizeVer; ++outputRow) {
      for (RoomCoord roomColumn = 0; roomColumn < FloorSize; ++roomColumn) {
        WriteRoomRow(fp, roomRow, roomColumn, outputRow, currentRoom);
      }
    }
  }

  fclose(fp);
}
#endif


static void StartGame(struct GameState *state) {
  state->roomInfo = GetGameRoom(DefaultRoomCoordX, DefaultRoomCoordY);
  state->startedGame = true;
}


static bool CreateMainMenuScreen(struct GameState *state) {
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

    int allocatedCharCount = snprintf(NULL, 0, "%s%s%" PRIu32, screen.body, screen.extraText, *pReloadCount);
    if (allocatedCharCount <= 0) {
      return false;
    }
    ++allocatedCharCount;

    char *str = arena_alloc(&state->arena, allocatedCharCount * sizeof *str);
    if (!str) {
      return false;
    }

    if (snprintf(str, allocatedCharCount, "%s%s%" PRIu32, screen.body, screen.extraText, *pReloadCount) <= 0) {
      return false;
    }

    state->body = str;
  }

  ++(*pReloadCount);
  return true;
}

static bool CreateGameScreen(struct GameState *state) {
  // TODO: Remove
  static char bodyBeginning[] = "This is the game, you are in room [";
  static char bodyMiddle[] = ", ";
  static char bodyEnding[] = "].";

  if (!state->startedGame) {
    StartGame(state);
  }

#ifdef _DEBUG
  WriteMap(state->roomInfo);
#endif

  int allocatedCharCount = snprintf(NULL, 0, "%s%" PRIRoomCoord "%s%" PRIRoomCoord "%s",
                                    bodyBeginning, state->roomInfo->x + 1, bodyMiddle,
                                    state->roomInfo->y + 1, bodyEnding);
  if (allocatedCharCount <= 0) {
    return false;
  }
  ++allocatedCharCount;

  char *str = arena_alloc(&state->arena, allocatedCharCount * sizeof *str);
  if (!str) {
    return false;
  }

  if (snprintf(str, allocatedCharCount, "%s%" PRIRoomCoord "%s%" PRIRoomCoord "%s",
               bodyBeginning, state->roomInfo->x + 1, bodyMiddle,
               state->roomInfo->y + 1, bodyEnding)
      <= 0) {
    return false;
  }

  state->body = str;

  for (uint_fast8_t i = 0; i < state->inputCount; ++i) {
    switch (state->inputs[i].outcome) {
      case GameGoNorthOutcome:
        state->inputs[i].visible =
          GetGameRoom(state->roomInfo->x, state->roomInfo->y + 1)->exists;
        break;
      case GameGoEastOutcome:
        state->inputs[i].visible =
          GetGameRoom(state->roomInfo->x + 1, state->roomInfo->y)->exists;
        break;
      case GameGoSouthOutcome:
        state->inputs[i].visible =
          GetGameRoom(state->roomInfo->x, state->roomInfo->y - 1)->exists;
        break;
      case GameGoWestOutcome:
        state->inputs[i].visible =
          GetGameRoom(state->roomInfo->x - 1, state->roomInfo->y)->exists;
        break;
      default:
        break;
    }
  }

  return true;
}


// Must match the order of the CustomScreenCode enum in types.h
bool (*CustomScreenCode[])(struct GameState *) = {
  CreateMainMenuScreen,
  CreateGameScreen,
};
