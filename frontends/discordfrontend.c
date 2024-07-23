#include <stdarg.h>
#include <stdio.h>

#include "frontend.h"

void PrintError(const char *error, ...) {
  fputs("ERROR: ", stderr);

  va_list args;
  va_start(args, error);
  vfprintf(stderr, error, args);
  va_end(args);

  fputs(".\n", stderr);
}
