#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "../backend/game.h"
#include "../shared/crossprint.h"
#include "frontend.h"

#define ESC "\x1B"
#define CSI ESC "["

#ifndef _WIN32
static int BackupVEOF = '\4';
static int BackupVEOL = '\0';
#endif

static void SetupConsole(void) {
  printf(CSI "?1049h"); // Switch to alternative buffer
#ifndef _WIN32
  struct termios info;
  tcgetattr(STDIN_FILENO, &info);

  // On some systems VEOF and VEOL are reused as VMIN and VTIME so backup their values first
  BackupVEOF = info.c_cc[VEOF];
  BackupVEOL = info.c_cc[VEOL];

  info.c_lflag &= ~ICANON; // Allow reading functions to return before enter is pressed
  info.c_lflag &= ~ECHO;   // Do not write characters to screen
  info.c_cc[VMIN] = 1;     // Prevent reading functions returning if no keys are pressed
  info.c_cc[VTIME] = 0;    // Prevent reading functions returning after some time
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &info);
#endif
}

// TODO: Check if ICANON and ECHO were previously on?
static void ResetConsole(void) {
#ifndef _WIN32
  struct termios info;
  tcgetattr(STDIN_FILENO, &info);
  info.c_lflag |= ICANON; // Require enter to be pressed before reading functions return
  info.c_lflag |= ECHO;   // Print characters to screen
  info.c_cc[VEOF] = BackupVEOF;
  info.c_cc[VEOL] = BackupVEOL;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &info);
#endif
  printf(CSI "?1049l"); // Restore original buffer
}

static void PrintString(const char32_t *str) {
#if WCHAR_MAX == INT_LEAST32_MAX || WCHAR_MAX == UINT_LEAST32_MAX // unix likes
  static_assert(sizeof(wchar_t) == sizeof(char32_t), "Need them to be the same size to print utf-32 chars");
  printf("%ls", (wchar_t *)str);
#elif defined(_WIN32) // windows
  wchar_t *wcStr = c32towc(str);
  if (!wcStr) {
    return;
  }
  printf("%ls", wcStr);
  free(wcStr);
#else
#error Need utf-32 printing support
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
  while(true) {
#ifdef _WIN32
    int input = _getch();
#else
    int input = getchar();
#endif
    if ('1' <= input && input <= '9') {
      return input - '1';
    }
  }
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
  bool succeeded = GetCurrentGameOutput(&output);
  if (!succeeded) {
    return UINT32_MAX;
  }
  PrintOutputBody(output.body);
  PrintInputs(output.inputCount, output.inputs);
  return output.screenID;
}

static bool HandleInput(uint32_t screenID) {
  uint8_t input = GetInput();
  if (input == UINT8_MAX) {
    return HandleInput(screenID);
  }

  enum GameInputOutcome outcome = HandleGameInput(screenID, input);
  switch(outcome) {
    case InvalidInput:
      return HandleInput(screenID);
    case GetNextOutput:
      return true;
    case QuitGame:
      return false;
  }

  return false;
}

int main(void) {
  if (!SetupGame()) {
    return 1;
  }
  SetupConsole();

  uint32_t stateID;
  do {
    stateID = HandleOutput();
    if (stateID == UINT32_MAX) {
      break;
    }
  } while(HandleInput(stateID));

  ResetConsole();
  CleanupGame();
  return 0;
}
