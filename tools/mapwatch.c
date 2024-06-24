#define _POSIX_C_SOURCE 199309L
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <fcntl.h>
#include <windows.h>
#elif defined(_POSIX_VERSION)
#include <time.h>
#else // Hope for c threads
#include <threads.h>
#endif

#define ESC "\x1B"
#define CSI ESC "["

static FILE *fp;
static bool keepLooping = true;

static void HandleIntSig(int unused) {
  (void)unused;
  keepLooping = false;
}

static void SleepMS(long msec) {
  if (msec > 999999999) {
    return;
  }

#if defined(_WIN32)
  // From https://stackoverflow.com/a/17283549
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -10000 * msec;

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
#elif defined(_POSIX_VERSION)
    struct timespec sleepTime = {0, 1000000 * msec};
    nanosleep(&sleepTime, NULL);
#else // Hope for c threads
    struct timespec sleepTime = {0, 1000000 * msec};
    thrd_sleep(&sleepTime, NULL);
#endif
}

int main(void) {
  signal(SIGINT, HandleIntSig);

#ifdef _WIN32
   SetConsoleOutputCP(CP_UTF8);
#endif

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

    SleepMS(500);
  }
  result = 0;

  printf(CSI "?1049l"); // Restore original buffer
  return result;
}