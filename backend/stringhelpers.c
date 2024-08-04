#include <arena.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
// TODO: Does cosmo need this now that utf-8 can be used?
#elif defined(_COSMO_SOURCE)
#include <windowsesque.h>
extern int MultiByteToWideChar(UINT, DWORD, LPCCH, int, LPWSTR, int);

// From mingw-w64's winnls.h
#define MB_ERR_INVALID_CHARS 0x8
#endif

#include "stringhelpers.h"

#if defined(_WIN32) || defined(_COSMO_SOURCE)
WCHAR *s8tows(const char *str) {
  // From https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
  size_t strCodeUnitCount = strlen(str);
  int wStrLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, (int)strCodeUnitCount, NULL, 0);
  if (wStrLen == 0) {
    return NULL;
  }
  ++wStrLen;

  WCHAR *pWStr = malloc(wStrLen * sizeof *pWStr);
  if (!pWStr) {
    return NULL;
  }

  int ret = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, (int)strCodeUnitCount, pWStr, wStrLen);
  if (!ret) {
    free(pWStr);
    return NULL;
  }
  // Is this needed?
  pWStr[wStrLen - 1] = L'\0';

  return pWStr;
}
#endif

struct DStr *DStrNew(Arena *const restrict arena) {
  struct DStr *str = arena_alloc(arena, sizeof *str);
  if (!str) {
    return NULL;
  }

  str->arena = arena;
  str->len = 16;
  str->str = arena_alloc(arena, str->len);
  if (!str->str) {
    return NULL;
  }

  str->str[0] = '\0';
  return str;
}

bool DStrAppend(struct DStr *const restrict str, const char *const restrict catStr) {
  size_t strLen = strlen(str->str);
  size_t catStrLen = strlen(catStr);
  size_t requiredLen = strLen + catStrLen + 1;

  if (requiredLen > str->len) {
    char *newStr = arena_realloc(str->arena, str->str, str->len, 2 * requiredLen);
    if (!newStr) {
      return false;
    }
    str->str = newStr;
    str->len = 2 * requiredLen;
  }

  memcpy(str->str + strLen, catStr, catStrLen + 1);
  return true;
}

// TODO: Use arena's arena_sprintf?
struct DStr *DStrPrintf(struct DStr *const restrict str, const char *const restrict format, ...) {
  va_list args1, args2;
  va_start(args1, format);
  va_start(args2, format);

  int formattedLen = vsnprintf(NULL, 0, format, args1);
  va_end(args1);
  if (formattedLen <= 0) {
    goto cleanup;
  }
  ++formattedLen;

  size_t strLen = strlen(str->str);
  size_t requiredLen = strLen + formattedLen;

  if (requiredLen > str->len) {
    char *newStr = arena_realloc(str->arena, str->str, str->len, 2 * requiredLen);
    if (!newStr) {
      goto cleanup;
    }
    str->str = newStr;
    str->len = 2 * requiredLen;
  }

  vsnprintf(str->str + strLen, formattedLen, format, args2);

cleanup:
  va_end(args2);
  return str;
}


// TODO: Use arena's arena_sprintf?
const char *CreateString(Arena *const arena, const char *const format, ...) {
  va_list args1, args2;
  va_start(args1, format);
  va_start(args2, format);

  char *res = NULL;

  int formattedLen = vsnprintf(NULL, 0, format, args1);
  va_end(args1);
  if (formattedLen <= 0) {
    goto cleanup;
  }
  ++formattedLen;

  res = arena_alloc(arena, formattedLen);
  if (!res) {
    goto cleanup;
  }

  if (vsnprintf(res, formattedLen, format, args2) <= 0) {
    res = NULL;
  }

cleanup:
  va_end(args2);
  return res;
}
