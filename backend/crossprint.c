#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
// TODO: Does cosmo need this now that utf-8 can be used?
#elif defined(_COSMO_SOURCE)
#include <windowsesque.h>
extern int MultiByteToWideChar(UINT, DWORD, LPCCH, int, LPWSTR, int);

// From mingw-w64's winnls.h
#define MB_ERR_INVALID_CHARS 0x8
#endif

#include "crossprint.h"

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
