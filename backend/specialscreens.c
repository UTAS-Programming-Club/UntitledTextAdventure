#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "screens.h"
#include "specialscreens.h"
#include "../shared/parser.h"

// Each room take 4x4 but the 4 required calls to WriteRoomRow per room only
// handle the top left most 3x3 unless it is the right and/or bottom most room
#define RoomGridSize 4

#define HorLine "─"
#define VerLine "│"

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
#ifdef _DEBUG
static void WriteRoomRow(FILE *fp, RoomCoord roomRow, RoomCoord roomColumn,
                         uint_fast8_t outputRow, const struct RoomInfo *currentRoom) {
  if (RoomGridSize - 1 == outputRow && 0 != roomRow) {
    return;
  }

  // Top, middle and bottom grid rows
  if (0 == outputRow || RoomGridSize - 1 == outputRow) {
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

    FPrintRep(HorLine, RoomGridSize - 2, fp);

    if (FloorSize - 1 == roomColumn) {
      fputs(rowChars[2], fp);
      fputc('\n', fp);
    }
  }

  // Middle Room Rows
  else {
    // Player in room
    if (currentRoom->x == roomColumn && currentRoom->y == roomRow && 1 == outputRow) {
      fprintf(fp, VerLine "P%*s", RoomGridSize - 3, "");
    // Room exists
    } else if (GetGameRoom(roomRow, roomColumn)->exists) {
      fprintf(fp, VerLine "%*s", RoomGridSize - 2, "");
    // Room does not exist
    } else {
      fprintf(fp, VerLine "NO%*s", RoomGridSize - 4, "");
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
    for (uint_fast8_t outputRow = 0; outputRow < RoomGridSize; ++outputRow) {
      for (RoomCoord roomColumn = 0; roomColumn < FloorSize; ++roomColumn) {
        WriteRoomRow(fp, roomRow, roomColumn, outputRow, currentRoom);
      }
    }
  }

  fclose(fp);
}
#endif


static void StartGame(struct GameOutput *output) {
  output->roomInfo = GetGameRoom(DefaultRoomCoordX, DefaultRoomCoordY);
  output->startedGame = true;
}


static bool CreateMainMenuScreen(struct GameOutput *output) {
  size_t reloadCountVarOffset = GetGameStateOffset(output->screenID, 0);
  if (reloadCountVarOffset == SIZE_MAX) {
    return false;
  }

  output->startedGame = false;

  uint32_t *pReloadCount = (uint32_t *)(output->stateData + reloadCountVarOffset);

  if (*pReloadCount) {
    struct GameScreen screen = {0};
    if (!GetGameScreen(output->screenID, &screen)) {
      return false;
    }

    int allocatedCharCount = snprintf(NULL, 0, "%s%s%" PRIu32, screen.body, screen.extraText, *pReloadCount);
    if (allocatedCharCount <= 0) {
      return false;
    }
    ++allocatedCharCount;

    char *str = arena_alloc(&output->arena, allocatedCharCount * sizeof *str);
    if (!str) {
      return false;
    }

    if (snprintf(str, allocatedCharCount, "%s%s%" PRIu32, screen.body, screen.extraText, *pReloadCount) <= 0) {
      return false;
    }

    output->body = str;
  }

  ++(*pReloadCount);
  return true;
}

static bool CreateGameScreen(struct GameOutput *output) {
  // TODO: Remove
  static char bodyBeginning[] = "This is the game, you are in room [";
  static char bodyMiddle[] = ", ";
  static char bodyEnding[] = "].";

  if (!output->startedGame) {
    StartGame(output);
  }

#ifdef _DEBUG
  WriteMap(output->roomInfo);
#endif

  int allocatedCharCount = snprintf(NULL, 0, "%s%" PRIRoomCoord "%s%" PRIRoomCoord "%s",
                                    bodyBeginning, output->roomInfo->x + 1, bodyMiddle,
                                    output->roomInfo->y + 1, bodyEnding);
  if (allocatedCharCount <= 0) {
    return false;
  }
  ++allocatedCharCount;

  char *str = arena_alloc(&output->arena, allocatedCharCount * sizeof *str);
  if (!str) {
    return false;
  }

  if (snprintf(str, allocatedCharCount, "%s%" PRIRoomCoord "%s%" PRIRoomCoord "%s",
               bodyBeginning, output->roomInfo->x + 1, bodyMiddle,
               output->roomInfo->y + 1, bodyEnding)
      <= 0) {
    return false;
  }

  output->body = str;

  for (uint8_t i = 0; i < output->inputCount; ++i) {
    switch (output->inputs[i].outcome) {
      case GameGoNorthOutcome:
        output->inputs[i].visible =
          GetGameRoom(output->roomInfo->x, output->roomInfo->y + 1)->exists;
        break;
      case GameGoEastOutcome:
        output->inputs[i].visible =
          GetGameRoom(output->roomInfo->x + 1, output->roomInfo->y)->exists;
        break;
      case GameGoSouthOutcome:
        output->inputs[i].visible =
          GetGameRoom(output->roomInfo->x, output->roomInfo->y - 1)->exists;
        break;
      case GameGoWestOutcome:
        output->inputs[i].visible =
          GetGameRoom(output->roomInfo->x - 1, output->roomInfo->y)->exists;
        break;
      default:
        break;
    }
  }

  return true;
}


// Must match the order of the CustomScreenCode enum in types.h
bool (*CustomScreenCode[])(struct GameOutput *) = {
  CreateMainMenuScreen,
  CreateGameScreen,
};
