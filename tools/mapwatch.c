#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <threads.h>
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

#define ESC "\x1B"
#define CSI ESC "["

static FILE *fp;
static bool keepLooping = true;

static void HandleIntSig(int unused) {
  (void)unused;
  keepLooping = false;
}

int main(void) {
  signal(SIGINT, HandleIntSig);

  int result = 1;

  printf(CSI "?1049h"); // Switch to alternative buffer

  while (keepLooping) {
    fp = fopen("Map.txt", "rb");
    if (!fp) {
      perror("ERROR: Failed to open Map.txt");
      break;
    }

    printf(CSI "?25l"); // Hide cursor
    printf(CSI "0;0H"); // Move cursor to 0, 0
    printf(CSI "0J");   // Erase entire screen

    int c;
    while ((c = fgetc(fp)) != EOF) {
      putchar(c);
    }

    printf(CSI "?25h"); // Show cursor

    fclose(fp);

    struct timespec sleepTime = {0, 500000000}; // 0.5 seconds
    thrd_sleep(&sleepTime, NULL);
  }
  result = 0;

  printf(CSI "?1049l"); // Restore original buffer
  return result;
}