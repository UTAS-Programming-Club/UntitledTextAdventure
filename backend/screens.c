#include <math.h>
#include <stdarg.h>
#include <string.h>

#include "alloc.h"
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

static char32_t *U64toS32(uint64_t number) {
  char32_t *str;
  size_t numberLength = U64toS32NoAlloc(NULL, number);
  str = Allocate((numberLength + 1) * sizeof *str);
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
    str[strSize] = U'\0';
  }
  ++strSize;

cleanup:
  va_end(args);
  return strSize;
}

bool CreateMainMenuScreen(struct GameOutput *output) {
  static uint32_t reloadCount = 0;

  if (!output) {
    return false;
  }
  output->screenID = MAIN_MENU_SCREEN_ID;

  struct GameScreen screen;
  if (!GetGameScreen(output->screenID, &screen)) {
    return false;
  }

  size_t writtenCharCount = 0;
  size_t allocatedCharCount = 0;
  if (reloadCount) {
    char32_t *reloadCountStr = U64toS32(reloadCount);
    if (!reloadCountStr) {
      return false;
    }

    allocatedCharCount = S32Merge(0, NULL, 3, screen.body, screen.extraText, reloadCountStr);
    if (!allocatedCharCount) {
      return false;
    }

    output->body = Allocate(allocatedCharCount * sizeof *(output->body));
    if (!S32Merge(allocatedCharCount, output->body, 3, screen.body, screen.extraText, reloadCountStr)) {
      return false;
    }
  } else {
    output->body = screen.body;
  }
  writtenCharCount = codeunitcount32(output->body);

  uint8_t debugButtonCount = 0;
  uint8_t buttonCount = GetGameScreenButtonCount(output->screenID) + debugButtonCount;
  if (buttonCount == UINT8_MAX) {
    return false;
  }
  output->inputCount = buttonCount;

  output->inputs = Allocate(output->inputCount * sizeof *output->inputs);
  if (!output->inputs) {
    return false;
  }

  uint8_t i = 0;
  for (; i < buttonCount - debugButtonCount; ++i) {
    struct GameScreenButton button;
    if (!GetGameScreenButton(output->screenID, i, &button)) {
      return false;
    }
    output->inputs[i].inputID = i;
    output->inputs[i].title = button.title;
  }

  if (debugButtonCount == 2) {
    output->inputs[i].inputID = i;
    output->inputs[i++].title = U64toS32(writtenCharCount);
    output->inputs[i].inputID = i;
    output->inputs[i].title = U64toS32(allocatedCharCount);
  }

  ++reloadCount;
  return true;
}

bool CreateTestScreen(struct GameOutput *output) {
  if (!output) {
    return false;
  }
  output->screenID = TEST_SCREEN_ID;

  struct GameScreen screen;
  if (!GetGameScreen(output->screenID, &screen)) {
    return false;
  }
  output->body = screen.body;

  uint8_t buttonCount = GetGameScreenButtonCount(output->screenID);
  if (buttonCount == UINT8_MAX) {
    return false;
  }
  output->inputCount = buttonCount;

  output->inputs = Allocate(output->inputCount * sizeof *output->inputs);
  if (!output->inputs) {
    return false;
  }

  for (uint8_t i = 0; i < buttonCount; ++i) {
    struct GameScreenButton button;
    if (!GetGameScreenButton(output->screenID, i, &button)) {
      return false;
    }
    output->inputs[i].inputID = i;
    output->inputs[i].title = button.title;
  }

  return true;
}


enum GameInputOutcome HandleMainMenuScreenInput(uint8_t inputID) {
  // TODO: Move to json
  switch(inputID) {
    case MAIN_MENU_START_BUTTON:
      return GetNextOutput;
    case MAIN_MENU_QUIT_BUTTON:
      return QuitGame;
  }

  return InvalidInput;
}

enum GameInputOutcome HandleTestScreenInput(uint8_t inputID) {
  // TODO: Move to json
  switch(inputID) {
    case MAIN_MENU_START_BUTTON:
      return GetNextOutput;
  }

  return InvalidInput;
}
