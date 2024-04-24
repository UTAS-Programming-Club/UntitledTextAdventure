#ifndef PCGAME_CROSSPRINT_H
#define PCGAME_CROSSPRINT_H

#include <uchar.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(_COSMO_SOURCE)
#include <windowsesque.h>
#endif

// Requires freeing returned memory
#if defined(_WIN32) || defined(_COSMO_SOURCE)
WCHAR *c32towc(const char32_t *str);
#endif

#endif // PCGAME_CROSSPRINT_H
