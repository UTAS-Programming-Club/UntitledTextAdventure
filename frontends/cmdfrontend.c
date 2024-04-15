#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#endif

#include "../backend/game.h"
#include "../shared/crossprint.h"
#include "frontend.h"

#define ESC "\x1B"
#define CSI ESC "["

static void SetupConsole(void) {
  printf(CSI "?1049h");
}

static void ResetConsole(void) {
  printf(CSI "?1049l");
}

static void PrintString(const char32_t *str) {
#if __WCHAR_MAX__ > 0x10000 // unix likes
  printf("%ls", (wchar_t *)str);
#else // windows
  wchar_t *wcStr = c32towc(str);
  printf("%ls", wcStr);
  free(wcStr);
#endif
}

// TODO: Enable ansi sequences on windows 10/11 for conhost, its on by default for terminal
static void PrintOutputBody(const char32_t *body) {
  printf(CSI "?25l"); // Hide cursor
  printf(CSI "0;0H"); // Move cursor to 0, 0
  printf(CSI "0J");   // Erase entire screen
  PrintString(body);
  printf(CSI "?25h"); // Show cursor
  putchar('\n');
}

// Returns 0 to 8 for inputs 1 to 9 and UINT8_MAX on failure
static uint8_t GetInput(void) {
#ifdef _WIN32
  while(true) {
    int input = _getch();
    if ('1' <= input && input <= '9') {
      return input - '1';
    }
  }
#else
  // TODO: Use tcgetattr to match allow any key to return a la https://stackoverflow.com/a/18806671
  getchar();
  return UINT8_MAX;
#endif
}

static void PrintInputs(uint8_t inputCount, const struct GameInput *inputs) {
  puts("\nUse the numbers below to make a selection.");
  for (uint8_t i = 0; i < inputCount; ++i) {
    printf("%" PRIu8 ". ", i + 1);
    PrintString(inputs[i].title);
    putchar('\n');
  }
}

static uint32_t HandleOutput(void) {
  struct GameOutput output;
  GetCurrentGameOutput(&output);
  PrintOutputBody(output.body);
  PrintInputs(output.inputCount, output.inputs);
  return output.stateID;
}

static bool HandleInput(uint32_t stateID) {
  uint8_t input = GetInput();
  if (input == UINT8_MAX) {
    return HandleInput(stateID);
  }

  enum GameInputOutcome outcome = HandleGameInput(stateID, input);
  switch(outcome) {
    case InvalidInput:
      return HandleInput(stateID);
    case GetNextOutput:
      return true;
    case QuitGame:
      return false;
  }

  return false;
}

int main(void) {
  SetupConsole();

  uint32_t stateID;
  do {
    stateID = HandleOutput();
  } while(HandleInput(stateID));

  CleanupGame();
  ResetConsole();
  return 0;
}