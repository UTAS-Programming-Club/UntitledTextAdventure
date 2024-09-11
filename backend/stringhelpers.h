#ifndef PCGAME_STRINGHELPERS_H
#define PCGAME_STRINGHELPERS_H

#include <arena.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(_COSMO_SOURCE)
#include <windowsesque.h>
#endif

// Requires freeing returned memory
#if defined(_WIN32) || defined(_COSMO_SOURCE)
WCHAR *s8tows(const char *str);
#endif

struct DStr {
  Arena *arena;
  char *str;  // utf-8
  size_t len;
};

struct DStr *DStrNew(Arena *const restrict);
bool DStrAppend(struct DStr *const restrict, const char *const restrict);
struct DStr *DStrPrintf(struct DStr *const restrict, const char *const restrict, ...);

const char *CreateString(Arena *const, const char *const, ...);

#endif // PCGAME_STRINGHELPERS_H
