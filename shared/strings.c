#include <stdlib.h>
#include <uchar.h>

#include "strings.h"

// From https://stackoverflow.com/a/14528742
int codeunitcount32(const char32_t *str) {
   if(!str) {
     return -1;
   }
   const char32_t *originalStr = str;
   for(; *str; ++str) ;
   // Need to account for terminating null
   return str - originalStr + 1;
}

// From https://github.com/mirror/mingw-w64/blob/master/mingw-w64-crt/misc/uchar_c32rtomb.c
// c32rtomb support was removed from mingw-w64 recently
// mingw-w64 with gcc 10 on ubuntu 22.04 wsl still has it but mingw-w64 with gcc 13 for msys2 does not
size_t cpc32rtomb(char *restrict s, char32_t c32) {
  if (c32 <= 0x7F) { // 7 bits needs 1 byte
    *s = (char)c32 & 0x7F;
    return 1;
  } else if (c32 <= 0x7FF) { // 11 bits needs 2 bytes
     s[1] = 0x80 | (char)(c32 & 0x3F);
     s[0] = 0xC0 | (char)(c32 >> 6);
     return 2;
  } else if (c32 <= 0xFFFF) { // 16 bits needs 3 bytes
     s[2] = 0x80 | (char)(c32 & 0x3F);
     s[1] = 0x80 | (char)((c32 >> 6) & 0x3F);
     s[0] = 0xE0 | (char)(c32 >> 12);
     return 3;
  } else if (c32 <= 0x1FFFFF) { // 21 bits needs 4 bytes
    s[3] = 0x80 | (char)(c32 & 0x3F);
    s[2] = 0x80 | (char)((c32 >> 6) & 0x3F);
    s[1] = 0x80 | (char)((c32 >> 12) & 0x3F);
    s[0] = 0xF0 | (char)(c32 >> 18);
    return 4;
   }

   return (size_t)-1;
}

// From https://github.com/mirror/mingw-w64/blob/master/mingw-w64-crt/misc/uchar_mbrtoc32.c
size_t cpmbrtoc32(char32_t *restrict pc32, const char *restrict s, size_t n) {
  if (*s == 0) {
    *pc32 = 0;
    return 0;
  }

  // ASCII character - high bit unset
  if ((*s & 0x80) == 0) {
    *pc32 = *s;
    return 1;
  }

  // Multibyte chars
  if ((*s & 0xE0) == 0xC0) { // 110xxxxx needs 2 bytes
    if (n < 2) {
      return (size_t)-2;
    }
    *pc32 = ((s[0] & 31) << 6) | (s[1] & 63);
    return 2;
  } else if ((*s & 0xf0) == 0xE0) { // 1110xxxx needs 3 bytes
    if (n < 3) {
      return (size_t)-2;
    }
    *pc32 = ((s[0] & 15) << 12) | ((s[1] & 63) << 6) | (s[2] & 63);
    return 3;
  } else if ((*s & 0xF8) == 0xF0) { // 11110xxx needs 4 bytes
    if (n < 4) {
      return (size_t)-2;
    }
    *pc32 = ((s[0] & 7) << 18) | ((s[1] & 63) << 12) | ((s[2] & 63) << 6) | (s[3] & 63);
    return 4;
  }

  return (size_t)-1;
}
