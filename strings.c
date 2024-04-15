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
// mingw-w64 for gcc 12 on ubuntu 22.04 wsl still has it but mingw-w64 for gcc 13 for msys2 does not
size_t cpc32rtomb (char *__restrict__ s, char32_t c32) {
  if (c32 <= 0x7F) { /* 7 bits needs 1 byte */
    *s = (char)c32 & 0x7F;
    return 1;
  } else if (c32 <= 0x7FF) { /* 11 bits needs 2 bytes */
     s[1] = 0x80 | (char)(c32 & 0x3F);
     s[0] = 0xC0 | (char)(c32 >> 6);
     return 2;
  } else if (c32 <= 0xFFFF) { /* 16 bits needs 3 bytes */
     s[2] = 0x80 | (char)(c32 & 0x3F);
     s[1] = 0x80 | (char)((c32 >> 6) & 0x3F);
     s[0] = 0xE0 | (char)(c32 >> 12);
     return 3;
  } else if (c32 <= 0x1FFFFF) { /* 21 bits needs 4 bytes */
    s[3] = 0x80 | (char)(c32 & 0x3F);
    s[2] = 0x80 | (char)((c32 >> 6) & 0x3F);
    s[1] = 0x80 | (char)((c32 >> 12) & 0x3F);
    s[0] = 0xF0 | (char)(c32 >> 18);
    return 4;
   }

   return (size_t)-1;
}
