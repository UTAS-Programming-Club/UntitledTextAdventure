#ifndef PCGAME_CROSSPRINT_H
#define PCGAME_CROSSPRINT_H

#if defined(_WIN32)
#include <windows.h>
#elif defined(_COSMO_SOURCE)
#include <windowsesque.h>
#endif

// Requires freeing returned memory
#if defined(_WIN32) || defined(_COSMO_SOURCE)
WCHAR *s8tows(const char *str);
#endif

#endif // PCGAME_CROSSPRINT_H
