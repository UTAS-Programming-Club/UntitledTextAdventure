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
#include "../backend/screens.h"
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

// Returns 0 to 8 for inputs 1 to 9
static uint8_t GetInput(void) {
  while(true) {
#ifdef _WIN32
    int input = _getch();
#else
    // getchar fails with cosmo on windows where it only returns after 2 keys have been pressed with ~ICANON if VMIN==1 so switched to read
    char buf[1] = {0};
    read(STDIN_FILENO, buf, sizeof(buf));
    int input = buf[0];
#endif
    if ('1' <= input && input <= '9') {
      return input - '1';
    }
  }
}

static void PrintInputs(uint8_t inputCount, const struct GameInput *inputs) {
  puts("\nUse the numbers below to make a selection.");
  // TODO: Find a better way to do this. Perhaps actually remove unused buttons fron inputs in game.c?
  for (uint8_t i = 0, visibleInputCount = 0; i < inputCount; ++i) {
    if (!inputs[i].visible) {
      continue;
    }
    printf("%" PRIu8 ". ", visibleInputCount + 1);
    PrintString(inputs[i].title);
    putchar('\n');
    ++visibleInputCount;
  }
}

static bool HandleOutput(struct GameOutput *output) {
  if (!GetCurrentGameOutput(output)) {
    return false;
  }
  PrintOutputBody(output->body);
  PrintInputs(output->inputCount, output->inputs);
  return true;
}

static bool HandleInput(struct GameOutput *output) {
  uint8_t input = GetInput();
  if (input == UINT8_MAX) {
    return HandleInput(output);
  }

  enum InputOutcome outcome = HandleGameInput(output, input);
  switch(outcome) {
    case InvalidInputOutcome:
      return HandleInput(output);
    case GetNextOutputOutcome:
      FreeScreen(output);
      return true;
    case QuitGameOutcome:
    default:
      FreeScreen(output);
      return false;
  }
}

int main(void) {
  if (!SetupGame()) {
    return 1;
  }
  SetupConsole();

  struct GameOutput output = {0};
  do {
    if (!HandleOutput(&output)) {
      break;
    }
  } while(HandleInput(&output));

  // TODO: Make sure this happens, even on crash. atexit + signal handler?
  ResetConsole();
  CleanupGame(&output);
  return 0;
}
