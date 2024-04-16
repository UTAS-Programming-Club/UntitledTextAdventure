#include <uchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <b64.h>

#include "../shared/base64.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }

  char *enc;
  if (!c8toc32base64(argv[1], &enc)) {
    return 1;
  }

  printf("BASE64: %s\n", enc);
  free(enc);
  return 0;
}
