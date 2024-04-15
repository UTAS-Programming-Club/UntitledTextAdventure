#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "crossprint.h"
#include "frontend.h"
#include "game.h"

#define ESC "\x1B"
#define CSI ESC "["

static void setupconsole(void) {
  printf(CSI "?1049h");
}

static void resetconsole(void) {
  printf(CSI "?1049l");
}

static void print(const char32_t *text) {
  printf(CSI "0;0H");
  printf(CSI "0J");
#if __WCHAR_MAX__ > 0x10000 // unix likes
  printf("%ls", (wchar_t *)text);
#else // windows
  char16_t *c16Text = c32toc16(text);
  printf("%ls", (wchar_t *)c16Text);
  free(c16Text);
#endif
}

int main(void) {
  setupconsole();
  print(outputgame());
  getchar();
  resetconsole();
  return 0;
}