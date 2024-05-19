#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "screens.h"
#include "specialscreens.h"
#include "../shared/parser.h"
#include "../shared/strings.h"

static size_t U64toS32NoAlloc(char32_t *buffer, uint64_t number) {
  size_t numberLength = 1;
  if (number) {
    numberLength = floor(log10(number)) + 1;
  }

  if (!buffer) {
    return numberLength;
  }

  if (!number) {
    buffer[0] = U'0';
  }

  for (size_t i = 0; number; ++i) {
    buffer[numberLength - i - 1] = (number % 10) + U'0';
    number /= 10;
  }

  return numberLength;
}

static char32_t *U64toS32(uint64_t number, Arena *arena) {
  char32_t *str;
  size_t numberLength = U64toS32NoAlloc(NULL, number);

  str = arena_alloc(arena, (numberLength + 1) * sizeof *str);
  if (!str) {
    return NULL;
  }

  U64toS32NoAlloc(str, number);
  str[numberLength] = U'\0';
  return str;
}

static size_t S32Merge(size_t strSize, char32_t *str, uint8_t strCount, ...) {
  va_list args;
  va_start(args, strCount);
  if (!strCount) {
    return 0;
  }

  for (uint8_t i = 0; i < strCount; ++i) {
    char32_t *nextStr = va_arg(args, char32_t *);
    if (!nextStr) {
      strSize = 0; // Need to report errors somehow :)
      goto cleanup;
    }

    size_t nextStrLen = codeunitcount32(nextStr) - 1; // Without null terminator

    if (str) {
      memcpy(str, nextStr, nextStrLen * sizeof *str);
      str += nextStrLen;
    }
    strSize += nextStrLen;
  }

  if (str) {
    *str = U'\0';
  }
  ++strSize;

cleanup:
  va_end(args);
  return strSize;
}


static bool CreateMainMenuScreen(struct GameOutput *output) {
  size_t reloadCountVarOffset = GetGameStateOffset(output->screenID, 0);
  if (reloadCountVarOffset == SIZE_MAX) {
    return false;
  }

  uint32_t *pReloadCount = (uint32_t *)(output->stateData + reloadCountVarOffset);

  if (*pReloadCount) {
    char32_t *reloadCountStr = U64toS32(*pReloadCount, &output->arena);
    if (!reloadCountStr) {
      return false;
    }

    struct GameScreen screen = {0};
    if (!GetGameScreen(output->screenID, &screen)) {
      return false;
    }

    size_t allocatedCharCount = S32Merge(0, NULL, 3, screen.body, screen.extraText, reloadCountStr);
    if (!allocatedCharCount) {
      return false;
    }

    char32_t *str = arena_alloc(&output->arena, allocatedCharCount * sizeof *str);
    if (!str) {
      return false;
    }

    if (!S32Merge(allocatedCharCount, str, 3, screen.body, screen.extraText, reloadCountStr)) {
      free(screen.body);
      free(screen.extraText);
      FreeScreen(output);
      return false;
    }
    free(screen.body);
    free(screen.extraText);
    if (!output->bodyArena) {
      free(output->body);
    }

    output->body = str;
    output->bodyArena = true;
  }

  ++(*pReloadCount);
  return true;
}

static bool CreateGameScreen(struct GameOutput *output) {
  static char32_t bodyBeginning[] = U"This is the game, you are in room ";
  static char32_t bodyEnding[] = U".";

  char32_t *roomIDStr = U64toS32(output->roomInfo.roomID, &output->arena);
  if (!roomIDStr) {
    return false;
  }

  size_t allocatedCharCount = S32Merge(0, NULL, 3, bodyBeginning, roomIDStr, bodyEnding);
  if (!allocatedCharCount) {
    return false;
  }

  char32_t *str = arena_alloc(&output->arena, allocatedCharCount * sizeof *str);
  if (!str) {
    return false;
  }

  if (!S32Merge(allocatedCharCount, str, 3, bodyBeginning, roomIDStr, bodyEnding)) {
    FreeScreen(output);
    return false;
  }
  if (!output->bodyArena) {
    free(output->body);
  }

  output->body = str;
  output->bodyArena = true;

  return true;
}


// Must match the order of the CustomScreenCode enum in types.h
bool (*CustomScreenCode[])(struct GameOutput *) = {
  CreateMainMenuScreen,
  CreateGameScreen,
};
