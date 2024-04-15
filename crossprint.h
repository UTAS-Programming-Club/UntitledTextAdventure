#ifndef PCGAME_CROSSPRINT_H
#define PCGAME_CROSSPRINT_H

#include <uchar.h>

// Requires freeing returned memory
#ifdef _WIN32
wchar_t *c32towc(const char32_t *str);
#endif

#endif // PCGAME_CROSSPRINT_H
