#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "crossprint.h"
#include "strings.h"

// This is fine under mingw-w64(but unnecessary) but fails under msvc
#ifndef _WIN32
// Runs before main to setup this library
static void __attribute__ ((constructor)) cpinit(void) {
  setlocale(LC_ALL, "en_US.UTF-8");
}
#endif

char *c32toc8(const char32_t *str) {
  size_t strCodeUnitCount = codeunitcount32(str);

  // From https://en.cppreference.com/w/c/string/multibyte/c32rtomb
  // TODO: Check if this extra 1 is needed? It might have been from when codeunitcount32 didn't count the null at the end
  char *pMbStr = malloc((MB_CUR_MAX * strCodeUnitCount + 1) * sizeof(*pMbStr));
  char *pMbStrCur = pMbStr;
  for (size_t n = 0; n < strCodeUnitCount; ++n) {
    size_t byteCount = cpc32rtomb(pMbStrCur, str[n]);
    if(byteCount == (size_t)-1) {
      break;
    }
    pMbStrCur += byteCount;
  }

  return pMbStr;
}

#ifdef _WIN32
char16_t *c32toc16(const char32_t *str) {
  size_t strCodeUnitCount = codeunitcount32(str);

  // From https://en.cppreference.com/w/c/string/multibyte/c32rtomb
  // TODO: Check if this extra 1 is needed? It might have been from when codeunitcount32 didn't count the null at the end
  char *pMbStr = malloc((MB_CUR_MAX * strCodeUnitCount + 1) * sizeof(*pMbStr));
  char *pMbStrCur = pMbStr;
  for (size_t n = 0; n < strCodeUnitCount; ++n) {
    size_t byteCount = cpc32rtomb(pMbStrCur, str[n]);
    if(byteCount == (size_t)-1) {
      break;
    }
    pMbStrCur += byteCount;
  }

  // From https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
  size_t mbStrCodeUnitCount = pMbStrCur - pMbStr;
  int wStrLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pMbStr, (int)mbStrCodeUnitCount, NULL, 0);
  if (wStrLen == 0) {
    strCodeUnitCount = EOF;
    goto cleanup_mbstr;
  }
  ++wStrLen;

  wchar_t *pWStr = malloc(wStrLen * sizeof(*pWStr));
  int ret = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pMbStr, (int)mbStrCodeUnitCount, pWStr, wStrLen);
  if (!ret) {
    strCodeUnitCount = EOF;
    goto cleanup_wstr;
  }
  // Is this needed?
  pWStr[wStrLen - 1] = L'\0';

  free(pMbStr);
  return pWStr;

cleanup_wstr:
  free(pWStr);
cleanup_mbstr:
  free(pMbStr);
  return NULL;
}
#endif
