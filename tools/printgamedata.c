#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

#include "../frontends/frontend.h"
#include "../shared/crossprint.h"
#include "../shared/parser.h"

#define INDENT "  "

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

void PrintError(const char *error, ...) {
  fputs("ERROR: ", stderr);

  va_list args;
  va_start(args, error);
  vfprintf(stderr, error, args);
  va_end(args);

  fputs(".\n", stderr);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }

  LoadGameData(argv[1]);
  uint16_t screenCount = GetGameScreenCount();
  if (screenCount == UINT16_MAX) {
    goto cleanup;
  }

  printf("Screen count: %" PRIu32 "\n\n", screenCount);
  for (enum Screen i = 0; i < screenCount; ++i) {
    struct GameScreen screen = {0};
    if (!GetGameScreen(i, &screen)) {
      continue;
    }

    printf("Screen %d\n", i);
    printf(INDENT "Body: \"");
    PrintString(screen.body);
    printf("\"\n" INDENT "Extra text: \"");
    PrintString(screen.extraText);
    puts("\"");

    uint_fast8_t screenButtonCount = GetGameScreenButtonCount(i);
    if (screenButtonCount == UINT_FAST8_MAX) {
      continue;
    }

    printf(INDENT "Button count: %" PRIuFAST8 "\n", screenButtonCount);
    for (uint_fast8_t j = 0; j < screenButtonCount; ++j) {
      struct GameScreenButton button;
      if (!GetGameScreenButton(i, j, &button)) {
        continue;
      }

      printf(INDENT "Button %" PRIuFAST8 "\n", j);
      printf(INDENT INDENT "Title: \"");
      PrintString(button.title);
      puts("\"");
    }
  }

cleanup:
  UnloadGameData();
}
