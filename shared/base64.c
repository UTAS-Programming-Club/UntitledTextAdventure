#include <stdbool.h>
#include <stdlib.h>
#include <b64.h>
#include <string.h>
#include <uchar.h>

#include "base64.h"
#include "strings.h"

#ifdef PREPTEXT
bool c8toc32base64(char *str8, char **base64Str32) {
  if (!str8 || !base64Str32) {
    return false;
  }

  bool success = false;

  // From https://en.cppreference.com/w/c/string/multibyte/mbrtoc32
  size_t str8CodeUnitCount = strlen(str8) + 1;

  char *str8Cur = str8;
  char *str8End = str8 + str8CodeUnitCount;
  char32_t *str32 = malloc(str8CodeUnitCount * sizeof *str32);
  char32_t *str32Cur = str32;

  size_t ret;
  while ((ret = cpmbrtoc32(str32Cur, str8Cur, str8End - str8Cur))) {
      if (ret == -1u) { // Encoding error
        goto cleanup;
      }
      if (ret == -2u) { // Incomplete utf8 code point
        goto cleanup;
      }
      str8Cur  += ret;
      str32Cur += 1;
  }

  size_t str32CodePointCount = str32Cur - str32 + 1;

  *base64Str32 = b64_encode((unsigned char *)str32, str32CodePointCount * sizeof *str32);
  if (*base64Str32) {
    success = true;
  }

cleanup:
  free(str32);
  return success;
}
#endif

#ifdef BACKEND
char32_t *c32base64toc32(char *base64Str32) {
  size_t base64Str32Len = strlen(base64Str32);
  unsigned char *str832 = b64_decode(base64Str32, base64Str32Len);
  return (char32_t *)str832;
}
#endif
