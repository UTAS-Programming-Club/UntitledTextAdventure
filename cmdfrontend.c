#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "crossprint.h"
#include "frontend.h"
#include "game.h"

#define ESC "\x1B"
#define CSI ESC "["

static void SetupConsole(void) {
  printf(CSI "?1049h");
}

static void ResetConsole(void) {
  printf(CSI "?1049l");
}

static void PrintOutputBody(const char32_t *body) {
  printf(CSI "0;0H");
  printf(CSI "0J");
#if __WCHAR_MAX__ > 0x10000 // unix likes
  printf("%ls", (wchar_t *)body);
#else // windows
  wchar_t *wcBody = c32towc(body);
  printf("%ls", wcBody);
  free(wcBody);
#endif
}

static void HandleOutput(void) {
  struct GameOutput output;
  GetCurrentGameOutput(&output);
  PrintOutputBody(output.body);
  getchar();
}

int main(void) {
  SetupConsole();
  HandleOutput();
  CleanupGame();
  ResetConsole();
  return 0;
}