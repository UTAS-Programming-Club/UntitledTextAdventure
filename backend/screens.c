#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define ARENA_IMPLEMENTATION
#include <arena.h>

#include "game.h"
#include "screens.h"
#include "../shared/parser.h"
#include "../shared/strings.h"

static size_t U64toS32NoAlloc(char32_t *buffer, uint64_t number) {
  size_t numberLength = 1;
  if (number) {
    numberLength = floor(log10(number)) + 1;
  }
  if (!buffer) {
    return numberLength;
  };
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

// TODO: Fix naming, this gives a GameOutput, not a GameScreen
bool CreateMainMenuScreen(uint32_t screenID, struct GameOutput *output) {
  static uint32_t reloadCount = 0;
  size_t allocatedCharCount = 0;
  size_t writtenCharCount = 0;

  if (!CreateScreen(screenID, output)) {
    return false;
  }

  if (reloadCount) {
    char32_t *reloadCountStr = U64toS32(reloadCount, &output->arena);
    if (!reloadCountStr) {
      return false;
    }

    struct GameScreen screen = {0};
    if (!GetGameScreen(output->screenID, &screen)) {
      return false;
    }

    allocatedCharCount = S32Merge(0, NULL, 3, screen.body, screen.extraText, reloadCountStr);
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
  writtenCharCount = codeunitcount32(output->body);

  uint8_t debugButtonCount = 0;
  uint8_t newButtonCount = output->inputCount + debugButtonCount;

  struct GameInput *buttons;
  if (output->inputsArrayArena) {
    buttons = arena_realloc(&output->arena, output->inputs, output->inputCount * sizeof *buttons, newButtonCount * sizeof *buttons);
  } else {
    buttons = realloc(output->inputs, output->inputCount * sizeof *buttons);
  }

  if (!buttons) {
    // TODO: Free anything manually allocated
    return false;
  }
  output->inputs = buttons;
  output->inputCount = newButtonCount;

  uint8_t buttonIndex = output->inputCount - debugButtonCount;
  if (debugButtonCount == 2) {
    output->inputs[buttonIndex].title = U64toS32(allocatedCharCount, &output->arena);
    if (!output->inputs[buttonIndex].title) {
      // TODO: Free anything manually allocated
      return false;
    }
    output->inputs[buttonIndex++].titleArena = true;

    output->inputs[buttonIndex].title = U64toS32(writtenCharCount, &output->arena);
    if (!output->inputs[buttonIndex].title) {
      // TODO: Free anything manually allocated
      return false;
    }
    output->inputs[buttonIndex].titleArena = true;
  }

  ++reloadCount;
  return true;
}

bool CreateScreen(uint32_t screenID, struct GameOutput *output) {
  if (!output) {
    return false;
  }
  output->screenID = screenID;

  struct GameScreen screen = {0};
  if (!GetGameScreen(output->screenID, &screen)) {
    return false;
  }
  output->body = screen.body;
  output->bodyArena = false;
  free(screen.extraText);

  uint8_t buttonCount = GetGameScreenButtonCount(output->screenID);
  if (buttonCount == UINT8_MAX) {
    return false;
  }
  output->inputCount = buttonCount;

  output->inputs = arena_alloc(&output->arena, output->inputCount * sizeof *output->inputs);
  output->inputsArrayArena = true;
  if (!output->inputs) {
    return false;
  }

  for (uint8_t i = 0; i < buttonCount; ++i) {
    struct GameScreenButton button;
    if (!GetGameScreenButton(output->screenID, i, &button)) {
      return false;
    }
    output->inputs[i].title = button.title;
    output->inputs[i].titleArena = false;
  }

  return true;
}

// TODO: Remove once everything is using the arena
void FreeScreen(struct GameOutput *output) {
  if (!output->bodyArena) {
    free(output->body);
    output->body = NULL;
  }
  for (uint8_t i = 0; i < output->inputCount; ++i) {
    // This only works because FreeGameScreenButton only touches the string at the beginning of
    // GameScreenButton which matches the layout of GameInput
    // TODO: Use GameScreenButton everywhere?
    if (!output->inputs[i].titleArena) {
      FreeGameScreenButton((struct GameScreenButton *)(output->inputs + i));
    }
  }
  if (!output->inputsArrayArena) {
    free(output->inputs);
    output->inputs = NULL;
  }
}

struct GameScreenButton *HandleScreenInput(uint32_t screenID, uint8_t inputID) {
  static struct GameScreenButton button;
  if (!GetGameScreenButton(screenID, inputID, &button)) {
    return NULL;
  }
  return &button;
}
