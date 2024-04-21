#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <uchar.h>

#include "../shared/parser.h"

#define INDENT "  "

int main(int argc, char *argv[]) {
  // The program cheats and uses %ls(wchar_t print) for char32_t
  static_assert(sizeof(wchar_t) == sizeof(char32_t));

  if (argc != 2) {
    return 1;
  }

  LoadGameData(argv[1]);
  uint32_t screenCount = GetGameScreenCount();
  if (screenCount == UINT32_MAX) {
    goto cleanup;
  }

  printf("Screen count: %" PRIu32 "\n\n", screenCount);
  for (uint32_t i = 0; i < screenCount; ++i) {
    struct GameScreen screen;
    if (!GetGameScreen(i, &screen)) {
      continue;
    }

    printf("Screen %d\n", i);
    printf(INDENT "Body: \"%ls\"\n", (wchar_t *)screen.body);
    printf(INDENT "Extra text: \"%ls\"\n", (wchar_t *)screen.extraText);

    uint8_t screenButtonCount = GetGameScreenButtonCount(i);
    if (screenButtonCount == UINT8_MAX) {
      continue;
    }

    printf(INDENT "Button count: %" PRIu8 "\n", screenButtonCount);
    for (uint8_t j = 0; j < screenButtonCount; ++j) {
      struct GameScreenButton button;
      if (!GetGameScreenButton(i, j, &button)) {
        continue;
      }

      printf(INDENT "Button %d\n", j);
      printf(INDENT INDENT "Title: \"%ls\"\n", (wchar_t *)button.title);
      FreeGameScreenButton(&button);
    }
  }

cleanup:
  UnloadGameData();
}
