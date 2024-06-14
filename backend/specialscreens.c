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

// Each room take 4x4 but the 3 required calls to WriteRoomRow per room only
// handles the top left most 3x3 unless it is the right and/or bottom most room
#define RoomGridSize 4

#define TopLeftLine "┌"
#define TopRightLine "┐"
#define BottomLeftLine "└"
#define BottomRightLine "┘"

#define TopMiddleLine "┬"
#define SideLeftLine "├"
#define SideRightLine "┤"
#define BottomMiddleLine "┴"

#define MiddleLine "┼"

#define HorLine "─"
#define VerLine "│"

// TODO: Only show inner walls on rooms that exists
// TODO: Find a better way to mark rooms that don't exist
// TODO: Add openings between rooms that exist
// TODO: Indicate room type
// TODO: Indicate player position
#ifdef _DEBUG
static void WriteRoomRow(FILE *fp, uint_fast8_t roomRow,
                         uint_fast8_t roomColumn, uint_fast8_t outputRow) {
  // Top Grid Row
  if (FloorSize - 1 == roomRow && 0 == roomColumn && 0 == outputRow) {
    fputs(TopLeftLine HorLine HorLine, fp);
  } else if (FloorSize - 1 == roomRow && FloorSize - 1 > roomColumn && 0 == outputRow) {
    fputs(TopMiddleLine HorLine HorLine, fp);
  } else if (FloorSize - 1 == roomRow && FloorSize - 1 == roomColumn && 0 == outputRow) {
    fputs(TopMiddleLine HorLine HorLine TopRightLine "\n", fp);
  }

  // Middle Room Rows
  else if (FloorSize - 1 > roomColumn && 0 < outputRow && RoomGridSize - 1 > outputRow) {
    if (GetGameRoom(roomRow, roomColumn)->exists) {
      fputs(VerLine "  ", fp);
    } else {
      fputs(VerLine "NO", fp);
    }
  } else if (FloorSize - 1 == roomColumn && 0 < outputRow && RoomGridSize - 1 > outputRow) {
    if (GetGameRoom(roomRow, roomColumn)->exists) {
      fputs(VerLine "  " VerLine "\n", fp);
    } else {
      fputs(VerLine "NO" VerLine "\n", fp);
    }
  }

  // Middle Grid Rows
  else if (0 < roomRow && 0 == roomColumn && RoomGridSize - 1 == outputRow) {
    fputs(SideLeftLine HorLine HorLine, fp);
  } else if (0 < roomRow && FloorSize - 1 > roomColumn && RoomGridSize - 1 == outputRow) {
    fputs(MiddleLine HorLine HorLine, fp);
  } else if (0 < roomRow && FloorSize - 1 == roomColumn && RoomGridSize - 1 == outputRow) {
    fputs(MiddleLine HorLine HorLine SideRightLine "\n", fp);
  }

  // Bottom Grid Row
  else if (0 == roomRow && 0 == roomColumn && RoomGridSize - 1 == outputRow) {
    fputs(BottomLeftLine HorLine HorLine, fp);
  } else if (0 == roomRow && FloorSize - 1 > roomColumn && RoomGridSize - 1 == outputRow) {
    fputs(BottomMiddleLine HorLine HorLine, fp);
  } else if (0 == roomRow && FloorSize - 1 == roomColumn && RoomGridSize - 1 == outputRow) {
    fputs(BottomMiddleLine HorLine HorLine BottomRightLine "\n", fp);
  }
}

static void WriteMap(void) {
  FILE *fp = fopen("Map.txt", "w");
  if (!fp) {
    return;
  }

  for (uint_fast8_t roomRow = FloorSize - 1; roomRow != UINT_FAST8_MAX; --roomRow) {
    for (uint_fast8_t outputRow = 0; outputRow < RoomGridSize; ++outputRow) {
      for (uint_fast8_t roomColumn = 0; roomColumn < FloorSize; ++roomColumn) {
        WriteRoomRow(fp, roomRow, roomColumn, outputRow);
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
  WriteMap();
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
        // output->inputs[i].visible =
        //   GetGameRoom(output->roomInfo->x, output->roomInfo->y + 1)->exists;
        output->inputs[i].visible = false;
        break;
      case GameGoEastOutcome:
        // output->inputs[i].visible =
        //   GetGameRoom(output->roomInfo->x + 1, output->roomInfo->y)->exists;
        output->inputs[i].visible = false;
        break;
      case GameGoSouthOutcome:
        // output->inputs[i].visible =
        //   GetGameRoom(output->roomInfo->x, output->roomInfo->y - 1)->exists;
        output->inputs[i].visible = false;
        break;
      case GameGoWestOutcome:
        // output->inputs[i].visible =
        //   GetGameRoom(output->roomInfo->x - 1, output->roomInfo->y)->exists;
        output->inputs[i].visible = false;
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
