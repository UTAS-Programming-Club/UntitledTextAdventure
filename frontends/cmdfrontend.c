#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>
#include <wchar.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#include "../backend/game.h"
#include "../backend/stringhelpers.h"
#include "frontend.h"

#define ESC "\x1B"
#define CSI ESC "["

static bool BackupsMade = false;
static bool ConsoleRestored = false;
#ifdef _WIN32
static DWORD BackupMode;
#else
static tcflag_t BackupLFlag;
static int BackupVEOF;
static int BackupVEOL;
#endif

static bool SetupConsole(void) {
  printf(CSI "?1049h"); // Switch to alternative buffer

#ifdef _WIN32
  // Disable line buffering
  // Required for older mintty(I think) e.g. git bash(does non bmp unicode work there?) and mobaxterm
  if (setvbuf(stdout, NULL, _IONBF, 0)) {
    return false;
  }

  // From https://stackoverflow.com/a/9218551
  // Required for older mintty(I think) e.g. git bash and mobaxterm
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  if (INVALID_HANDLE_VALUE == hStdin) {
    return false;
  }

  DWORD mode;
  if (!GetConsoleMode(hStdin, &mode)) {
    printf("WTF, %ld\n", GetLastError());
    // For some reason on mobaxterm this fails with ERROR_INVALID_HANDLE, hStdin looks normal

#ifdef __MINGW64_VERSION_MAJOR
    return true;
#endif
    return false;
  }

  BackupMode = mode;
  BackupsMade = true;

  mode &= ~ENABLE_LINE_INPUT; // Allow reading functions to return before enter is pressed
  mode &= ~ENABLE_ECHO_INPUT; // Do not write characters to screen
  if (!SetConsoleMode(hStdin, mode)) {
    return false;
  }
#else
  struct termios info;
  if (tcgetattr(STDIN_FILENO, &info)) {
    return false;
  }

  BackupLFlag = info.c_lflag;
  // On some systems VEOF and VEOL are reused as VMIN and VTIME so backup their values first
  BackupVEOF = info.c_cc[VEOF];
  BackupVEOL = info.c_cc[VEOL];
  BackupsMade = true;

  info.c_lflag &= ~ICANON; // Allow reading functions to return before enter is pressed
  info.c_lflag &= ~ECHO;   // Do not write characters to screen
  info.c_cc[VMIN] = 1;     // Prevent reading functions returning if no keys are pressed
  info.c_cc[VTIME] = 0;    // Prevent reading functions returning after some time
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &info)) {
    return false;
  }
#endif

  return true;
}

static void ResetConsole(void) {
  if (ConsoleRestored) {
    return;
  }

  if (!BackupsMade) {
    goto end;
  }

#ifdef _WIN32
  // From https://stackoverflow.com/a/9218551
  // Required for older mintty(I think) e.g. git bash and mobaxterm
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  if (INVALID_HANDLE_VALUE == hStdin) {
    goto end;
  }

  SetConsoleMode(hStdin, BackupMode);
#else
  struct termios info;
  if (tcgetattr(STDIN_FILENO, &info)) {
    goto end;
  }

  info.c_lflag = BackupLFlag;
  info.c_cc[VEOF] = BackupVEOF;
  info.c_cc[VEOL] = BackupVEOL;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &info);
#endif

end:
  printf(CSI "?1049l"); // Restore original buffer
  fflush(stdout);
  ConsoleRestored = true;
}


static void PrintString(const char *str) {
#ifndef _WIN32
  printf("%s", str);
#else
  wchar_t *wcStr = s8tows(str);
  if (!wcStr) {
    return;
  }
  printf("%ls", wcStr);
  free(wcStr);
#endif
}

// TODO: Enable ansi sequences on windows 10/11 for conhost, its on by default for terminal
static void PrintOutputBody(const char *body) {
  printf(CSI "?25l"); // Hide cursor
  printf(CSI "0;0H"); // Move cursor to 0, 0
  printf(CSI "0J");   // Erase entire screen
  PrintString(body);
  printf(CSI "?25h"); // Show cursor
}


// Returns 0 to 8 for inputs 1 to 9
static uint_fast8_t GetButtonInput(void) {
  while(true) {
    // getchar fails with cosmo on windows where it only returns after 2 keys have been pressed with ~ICANON if VMIN==1 so switched to read
    char buf[1] = {0};
    int ret = read(STDIN_FILENO, buf, sizeof buf);
    char input = buf[0];
    if (ret && '1' <= input && input <= '9') {
      return input - '1';
    }
  }
}

static void PrintButtonInputs(uint_fast8_t inputCount, const struct GameInput *inputs) {
  puts("\n\nUse the numbers below to make a selection.");
  // TODO: Find a better way to do this. Perhaps actually remove unused buttons from inputs in game.c?
  for (uint_fast8_t i = 0, visibleInputCount = 0; i < inputCount; ++i) {
    if (!inputs[i].visible) {
      continue;
    }
    printf("%" PRIuFAST8 ". ", visibleInputCount + 1);
    PrintString(inputs[i].title);
    putchar('\n');
    ++visibleInputCount;
  }
}


// Returns input text as a utf-8 string if enter is pressed, "\x1B" on esc
// being pressed or NULL if buffer allocation failed
// Resulting pointer must be freed if not "\x1B" or NULL
static char *GetTextInput(void) {
  // Starting at 16 chars and will go to the next power of two when full
  // Do not need to allocate room for '\0' as it replaces the final char
  size_t bufSize = 16;
  size_t bufOffset = 0;
  char *buf = malloc(bufSize);
  if (!buf) {
    return NULL;
  }

  while(true) {
    if (bufOffset == bufSize) {
      size_t newBufSize = bufSize * bufSize;
      char *newBuf = realloc(buf, newBufSize);
      if (!newBuf) {
        free(buf);
        return NULL;
      }
      bufSize = newBufSize + 1;
      buf = newBuf;
    }

    int ret = read(STDIN_FILENO, buf + bufOffset, 1);
    if (!ret) {
      continue;
    }

    char input = buf[bufOffset];
    // This also detects any key that sends an escape sequence e.g. arrow keys
    if (ESC[0] == input) {
      free(buf);
      return ESC;
    }

    // delete, stand-in for backspace
    if (0x7f == input && 1 <= bufOffset) {
      printf("\b \b");
      --bufOffset;
    } else if (isblank(input) || isgraph(input)) {
      putchar(buf[bufOffset]);
      ++bufOffset;
    } else if ('\n' == input) {
      buf[bufOffset] = '\0';
      return buf;
    }

    fflush(stdout);
  }
}

static void PrintTextInput(void) {
  puts(ESC "7"); // Backup cursor position
  puts("\nPress Enter to confirm password entry.\nPress Esc to return to the previous screen.");
  printf(ESC "8"); // Restore cursor position
  fflush(stdout);
}


static bool HandleOutput(const struct GameInfo *info, struct GameState *state) {
  if (!UpdateGameState(info, state)) {
    return false;
  }
  PrintOutputBody(state->body);

  switch(state->inputType) {
    case ButtonScreenInputType:
      PrintButtonInputs(state->inputCount, state->inputs);
      return true;
    case TextScreenInputType:
      PrintTextInput();
      return true;
    case NoneScreenInputType:
      thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
      return true;
    default:
      PrintError("Unexpected input type with id %i received while handling output", state->inputType);
      return false;
  }
}

// Returned bool indicates whether or not to continue, true meaning continue
// res indicates whether or not there was a failure, 1 meaning failure
static bool HandleInput(const struct GameInfo *info, struct GameState *state, int *res) {
  uint_fast8_t buttonInput = UINT_FAST8_MAX;
  char *textInput = NULL;

  *res = 1;

  switch(state->inputType) {
    case ButtonScreenInputType:
      buttonInput = GetButtonInput();
      break;
    case TextScreenInputType:
      textInput = GetTextInput();
      if (!textInput) {
        return false;
      }
      break;
    case NoneScreenInputType:
      break;
    default:
      PrintError("Unexpected input type with id %i received while handling input", state->inputType);
      return false;
  }

  enum InputOutcome outcome = HandleGameInput(info, state, buttonInput, textInput);
  if (textInput && ESC[0] != textInput[0]) {
    free(textInput);
  }

  switch(outcome) {
    case InvalidInputOutcome:
      if (ButtonScreenInputType == state->inputType) {
        return HandleInput(info, state, res);
      // Force a redraw if the password was invalid
      } else if (TextScreenInputType == state->inputType) {
        return true;
      } else {
        PrintError("Invalid input outcome received");
        return false;
      }
    case GetNextOutputOutcome:
      return true;
    case QuitGameOutcome:
      *res = 0;
      return false;
    default:
      PrintError("Unexpected input outcome with id %i received", outcome);
      return false;
  }
}


void PrintError(const char *error, ...) {
  // Required in order to make stderr persist after the programs exists
  ResetConsole();

  fputs("ERROR: ", stderr);

  va_list args;
  va_start(args, error);
  vfprintf(stderr, error, args);
  va_end(args);

  fputs(".\n", stderr);
}

int main(void) {
  int res = 1;

  struct GameInfo info = {0};
  if (!SetupConsole() || !SetupBackend(&info)) {
    goto reset_console;
  }

  struct GameState state = {0};
  do {
    if (!HandleOutput(&info, &state)) {
      break;
    }
  } while(HandleInput(&info, &state, &res));

  // TODO: Make sure this happens, even on crash. atexit + signal handler?
  CleanupGame(&state);
  CleanupBackend(&info);
reset_console:
  ResetConsole();
  return res;
}
