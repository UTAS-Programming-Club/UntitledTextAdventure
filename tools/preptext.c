#include <uchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <b64.h>

#include "../shared/strings.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
   }

  // From https://en.cppreference.com/w/c/string/multibyte/mbrtoc32
  char *str8 = argv[1];
  size_t str8CodeUnitCount = strlen(str8) + 1;

  // printf("Processing %zu UTF-8 code units: [ ", str8CodeUnitCount);
  // for (size_t i = 0; i < str8CodeUnitCount; ++i)
  //     printf("0x%02x ", (unsigned char)str8[i]);
  // puts("]");

  char *str8Cur = str8;
  char *end = str8 + str8CodeUnitCount;
  char32_t *str32 = malloc(str8CodeUnitCount * sizeof *str32);
  char32_t *str32Cur = str32;
  size_t ret;
  while ((ret = cpmbrtoc32(str32Cur, str8Cur, end - str8Cur))) {
      if (ret == -1u) { // Encoding error
        break;
      }
      if (ret == -2u) { // Incomplete utf8 code point
        break;
      }
      str8Cur  += ret;
      str32Cur += 1;
  }

  size_t str32CodePointCount = str32Cur - str32 + 1;

  //printf("Test: %ls\n", (wchar_t *)str32);
  //printf("into %zu UTF-32 code units: [ ", str32CodePointCount);
  //for (size_t i = 0; i < str32CodePointCount; ++i)
  //    printf("0x%08X(%lc) ", str32[i], str32[i]);
  //puts("]");

  char *enc = b64_encode((unsigned char *)str32, str32CodePointCount * sizeof *str32);
  printf("BASE64: %s\n", enc);
  free(str32);
  return 0;
}
