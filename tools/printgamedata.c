#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <uchar.h>

#include "../shared/crossprint.h"
#include "../shared/parser.h"

#define INDENT "  "

static void PrintS32(const char32_t *str) {
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
    PrintS32(screen.body);
    printf("\"\n" INDENT "Extra text: \"");
    PrintS32(screen.extraText);
    puts("\"");

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
      printf(INDENT INDENT "Title: \"");
      PrintS32(button.title);
      puts("\"");
      FreeGameScreenButton(&button);
    }
  }

cleanup:
  UnloadGameData();
}
