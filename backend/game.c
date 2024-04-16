#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "alloc.h"
#include "game.h"
#include "../shared/strings.h"

// TODO: Parse game states from JSON
#define MAIN_MENU_SCREEN 0
  char32_t MAIN_MENU_BODY[] = U"Untitled text adventure game\n"\
                              U"----------------------------\n"\
                              U"By the UTAS Programming Club\n"\
                              U"\n"\
                              U"Currently unimplemented :(";
  char32_t MAIN_MENU_EXTRA_TEXT[] = U"\n"\
                                    U"Reload Count: ";
  #define MAIN_MENU_START_BUTTON 0
  char32_t MAIN_MENU_START_BUTTON_TITLE[] = U"Start Game";
  #define MAIN_MENU_QUIT_BUTTON 1
  char32_t MAIN_MENU_QUIT_BUTTON_TITLE[]  = U"Quit Game";

static size_t PrintNumberNoAlloc(char32_t *buffer, uint64_t number) {
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

static void PrintNumber(char32_t **buffer, uint64_t number) {
  size_t numberLength = PrintNumberNoAlloc(NULL, number);
  *buffer = Allocate((numberLength + 1) * sizeof **buffer);
  PrintNumberNoAlloc(*buffer, number);
  (*buffer)[numberLength] = U'\0';
}

bool GetCurrentGameOutput(struct GameOutput *output) {
  static long reloadCount = 0;
  size_t writtenCharCount = 0;
  size_t allocatedCharCount = 0;
  size_t bodyCharCount = codeunitcount32(MAIN_MENU_BODY);

  output->screenID = MAIN_MENU_SCREEN;

  if (reloadCount) {
    // Without null terminators
    bodyCharCount -= 1;
    size_t extraBodyCharCount = codeunitcount32(MAIN_MENU_EXTRA_TEXT) - 1;

    allocatedCharCount += bodyCharCount;
    allocatedCharCount += extraBodyCharCount;
    allocatedCharCount += PrintNumberNoAlloc(NULL, reloadCount);
    allocatedCharCount += 1; // Add null terminator

    output->body = Allocate(allocatedCharCount * sizeof *(output->body));
    if (!output->body) {
      return false;
    }

    memcpy(output->body, MAIN_MENU_BODY, bodyCharCount * sizeof *(output->body));
    writtenCharCount = bodyCharCount;

    memcpy(output->body + writtenCharCount, MAIN_MENU_EXTRA_TEXT, extraBodyCharCount * sizeof *(output->body));
    writtenCharCount += extraBodyCharCount;

    writtenCharCount += PrintNumberNoAlloc(output->body + writtenCharCount, reloadCount);

    output->body[writtenCharCount] = U'\0';
    writtenCharCount += 1;
  } else {
    output->body = MAIN_MENU_BODY;
  }

  output->inputCount = 2;
  output->inputs = Allocate(output->inputCount * sizeof *output->inputs);
  if (!output->inputs) {
    return false;
  }
  output->inputs[0].inputID = MAIN_MENU_START_BUTTON;
  output->inputs[0].title = MAIN_MENU_START_BUTTON_TITLE;
  output->inputs[1].inputID = MAIN_MENU_QUIT_BUTTON;
  output->inputs[1].title = MAIN_MENU_QUIT_BUTTON_TITLE;
  /*output->inputs[2].inputID = 2;
  PrintNumber(&output->inputs[2].title, writtenCharCount);
  output->inputs[3].inputID = 3;
  PrintNumber(&output->inputs[3].title, allocatedCharCount);*/

  ++reloadCount;
  return true;
}

enum GameInputOutcome HandleGameInput(uint32_t screenID, uint32_t inputID) {
  switch (screenID) {
    case MAIN_MENU_SCREEN:
      switch(inputID) {
        case MAIN_MENU_START_BUTTON:
          return GetNextOutput;
        case MAIN_MENU_QUIT_BUTTON:
          return QuitGame;
      }
    break;
  }

  return InvalidInput;
}

void CleanupGame(void) {
  FreeAll();
}
