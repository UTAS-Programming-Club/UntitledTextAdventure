#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wchar.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#include "../backend/game.h"
#include "../shared/crossprint.h"
#include "frontend.h"

#define ESC "\x1B"
#define CSI ESC "["

static bool BackupsMade;
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
  putchar('\n');
}

// Returns 0 to 8 for inputs 1 to 9
static uint_fast8_t GetInput(void) {
  while(true) {
    // getchar fails with cosmo on windows where it only returns after 2 keys have been pressed with ~ICANON if VMIN==1 so switched to read
    char buf[1] = {0};
    read(STDIN_FILENO, buf, sizeof(buf));
    int input = buf[0];
    if ('1' <= input && input <= '9') {
      return input - '1';
    }
  }
}

static void PrintInputs(uint_fast8_t inputCount, const struct GameInput *inputs) {
  puts("\nUse the numbers below to make a selection.");
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

static bool HandleOutput(struct GameOutput *output) {
  if (!GetCurrentGameOutput(output)) {
    return false;
  }
  PrintOutputBody(output->body);
  PrintInputs(output->inputCount, output->inputs);
  return true;
}

static bool HandleInput(struct GameOutput *output) {
  uint_fast8_t input = GetInput();

  enum InputOutcome outcome = HandleGameInput(output, input);
  switch(outcome) {
    case InvalidInputOutcome:
      return HandleInput(output);
    case GetNextOutputOutcome:
      return true;
    case QuitGameOutcome:
    default:
      return false;
  }
}

void PrintError(const char *error, ...) {
  fputs("ERROR: ", stderr);

  va_list args;
  va_start(args, error);
  vfprintf(stderr, error, args);
  va_end(args);

  fputs(".\n", stderr);
}

int main(void) {
  int res = 1;

  if (!SetupConsole() || !SetupBackend()) {
    goto reset_console;
  }

  struct GameOutput output = {0};
  do {
    if (!HandleOutput(&output)) {
      break;
    }
  } while(HandleInput(&output));

  res = 0;

  // TODO: Make sure this happens, even on crash. atexit + signal handler?
  CleanupGame(&output);
  CleanupBackend();
reset_console:
  ResetConsole();
  return res;
}
