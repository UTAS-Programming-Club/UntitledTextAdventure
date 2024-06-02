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

static bool CreateMainMenuScreen(struct GameOutput *output) {
  size_t reloadCountVarOffset = GetGameStateOffset(output->screenID, 0);
  if (reloadCountVarOffset == SIZE_MAX) {
    return false;
  }

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
  static char bodyBeginning[] = "This is the game, you are in room ";
  static char bodyEnding[] = ".";

  if (!GetGameRoom(&output->roomInfo)) {
    return false;
  }

  int allocatedCharCount = snprintf(NULL, 0, "%s%" PRIRoomID "%s", bodyBeginning, output->roomInfo.roomID, bodyEnding);
  if (allocatedCharCount <= 0) {
    return false;
  }
  ++allocatedCharCount;

  char *str = arena_alloc(&output->arena, allocatedCharCount * sizeof *str);
  if (!str) {
    return false;
  }

  if (snprintf(str, allocatedCharCount, "%s%" PRIRoomID "%s", bodyBeginning, output->roomInfo.roomID, bodyEnding) <= 0) {
    return false;
  }

  output->body = str;

  for (uint8_t i = 0; i < output->inputCount; ++i) {
    switch (output->inputs[i].outcome) {
      case GameGoNorthOutcome:
        output->inputs[i].visible = InvalidRoomID != output->roomInfo.northRoomID;
        break;
      case GameGoEastOutcome:
        output->inputs[i].visible = InvalidRoomID != output->roomInfo.eastRoomID;
        break;
      case GameGoSouthOutcome:
        output->inputs[i].visible = InvalidRoomID != output->roomInfo.southRoomID;
        break;
      case GameGoWestOutcome:
        output->inputs[i].visible = InvalidRoomID != output->roomInfo.westRoomID;
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
