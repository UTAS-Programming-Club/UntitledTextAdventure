#ifndef CROSSPRINT_H
#define CROSSPRINT_H

#include <uchar.h>


// Both require freeing returned memory
char *c32toc8(const char32_t *str);
#ifdef _WIN32
char16_t *c32toc16(const char32_t *str);
#endif

#endif // CROSSPRINT_H
