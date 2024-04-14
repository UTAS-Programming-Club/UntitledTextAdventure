#include <stdio.h>

#define ESC "\x1B"
#define CSI ESC "["

static void setupconsole(void) {
  puts(CSI "?1049h");
}

static void resetconsole(void) {
  puts(CSI "?1049l");
}

static void edit(char *text) {
  puts(CSI "0;0H");
  puts(CSI "0J");
  printf("%s", text);
}

int main(void) {
  setupconsole();
  edit("Untitled text adventure game\n"
       "----------------------------\n"
       "By the UTAS Programming Club\n"
       "\n"
       "Currently unimplemented :(");
  getchar();
  resetconsole();
  return 0;
}