#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../backend/crossprint.h"
#include "../backend/game.h"
#include "frontend.h"

#if defined(_WIN32)
#include <windows.h>
#include <strsafe.h>
#elif defined(_COSMO_SOURCE)
#include <libc/nt/enum/color.h>
#include <libc/nt/enum/cs.h>
#include <libc/nt/enum/cw.h>
#include <libc/nt/enum/idc.h>
#include <libc/nt/enum/sw.h>
#include <libc/nt/enum/wm.h>
#include <libc/nt/enum/ws.h>
#include <windowsesque.h>

#define COLOR_WINDOW   kNtColorWindow
#define CS_HREDRAW     kNtCsHredraw
#define CS_VREDRAW     kNtCsVredraw
#define CW_USEDEFAULT  kNtCwUsedefault
#define WM_COMMAND     kNtWmCommand
#define WM_DESTROY     kNtWmDestroy
#define WM_PAINT       kNtWmPaint
#define WS_CAPTION     kNtWsCaption
#define WS_CHILD       kNtWsChild
#define WS_MINIMIZEBOX kNtWsMaximizebox
#define WS_OVERLAPPED  kNtWsOverlapped
#define WS_SYSMENU     kNtWsSysmenu
#define WS_TABSTOP     kNtWsTabstop
#define WS_VISIBLE     kNtWsVisible

typedef struct NtMsg MSG;
typedef struct NtPaintStruct PAINTSTRUCT;
typedef struct NtRect RECT;
typedef struct NtWndClass WNDCLASSW;

#define CreateWindowExW  CreateWindowEx
#define DefWindowProcW   DefWindowProc
#define DispatchMessageW DispatchMessage
#define DrawTextW        DrawText
#define GetMessageW      GetMessage
extern BOOL InvalidateRect(HWND, CONST RECT *, BOOL);
extern HANDLE LoadImage(HINSTANCE, LPCWSTR, UINT, int, int, UINT);
#define LoadImageW       LoadImage
#define memcpy_s(d, ds, s, ss) (memcpy(d, s, ss), 0)
#define RegisterClassW   RegisterClass
extern BOOL UpdateWindow(HWND);

// From mingw-w64's ntdef.h
#define UNREFERENCED_PARAMETER(P) {(P) = (P);}

// From mingw-w64's minwindef.h
#define LOWORD(l) ((WORD) (((DWORD_PTR) (l)) & 0xffff))
#define HIWORD(l) ((WORD) ((((DWORD_PTR) (l)) >> 16) & 0xffff))

// From mingw-w64's strsafe.h
HRESULT StringCopyWorkerW(WCHAR *, size_t, CONST WCHAR *);
HRESULT StringCopyWorkerW(WCHAR *pszDest, size_t cchDest, CONST WCHAR *pszSrc) {
  HRESULT hr = S_OK;
  if(cchDest == 0) {
    hr = (HRESULT)0x80070057; // STRSAFE_E_INVALID_PARAMETER
  } else {
    while(cchDest && (*pszSrc != u'\0')) {
      *pszDest++ = *pszSrc++;
      --cchDest;
    }
    if(cchDest ==  0) {
      --pszDest;
      hr = (HRESULT)0x8007007A; //STRSAFE_E_INSUFFICIENT_BUFFER
    }
    *pszDest= u'\0';
  }
  return hr;
}

HRESULT StringCchCopyW(WCHAR *, size_t, CONST WCHAR *);
HRESULT StringCchCopyW(WCHAR *pszDest, size_t cchDest, CONST WCHAR *pszSrc) {
  if(cchDest > 2147483647) { // STRSAFE_MAX_CCH
    return (HRESULT)0x80070057; // STRSAFE_E_INVALID_PARAMETER
  }
  return StringCopyWorkerW(pszDest,cchDest,pszSrc);
}

// From mingw-w64's wingdi.h
#define ANSI_CHARSET          0
#define CLEARTYPE_QUALITY     5
#define CLIP_DEFAULT_PRECIS   0
#define FW_NORMAL           400
#define LF_FACESIZE          32
#define OUT_STROKE_PRECIS     3
#define OUT_TT_ONLY_PRECIS    7
#define TRUETYPE_FONTTYPE     4

typedef struct {
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  WCHAR lfFaceName[LF_FACESIZE];
} LOGFONTW, *LPLOGFONTW;

typedef struct {
  LONG tmHeight;
  LONG tmAscent;
  LONG tmDescent;
  LONG tmInternalLeading;
  LONG tmExternalLeading;
  LONG tmAveCharWidth;
  LONG tmMaxCharWidth;
  LONG tmWeight;
  LONG tmOverhang;
  LONG tmDigitizedAspectX;
  LONG tmDigitizedAspectY;
  WCHAR tmFirstChar;
  WCHAR tmLastChar;
  WCHAR tmDefaultChar;
  WCHAR tmBreakChar;
  BYTE tmItalic;
  BYTE tmUnderlined;
  BYTE tmStruckOut;
  BYTE tmPitchAndFamily;
  BYTE tmCharSet;
} TEXTMETRICW;

typedef int (CALLBACK *FONTENUMPROCW)(CONST LOGFONTW *, CONST TEXTMETRICW *, DWORD, LPARAM);

// From mingw-w64's winuser.h
#define BS_PUSHBUTTON 0L
#define BN_CLICKED 0
#define GWLP_HINSTANCE (-6)
#define MAKEINTRESOURCEW(i) ((LPWSTR)((ULONG_PTR)((WORD)(i))))
#define IDI_APPLICATION MAKEINTRESOURCEW(32512)
#define DT_WORDBREAK 0x00000010

// Cosmo doesn't have these functions so need to fetch manually
typedef HFONT (WINAPI *fCreateFontIndirectW)(CONST LOGFONTW *);
typedef int (WINAPI *fEnumFontFamiliesExW)(HDC, LPLOGFONTW, FONTENUMPROCW, LPARAM, DWORD);
typedef LONG_PTR (WINAPI *fGetWindowLongPtrW)(HWND, int);

fCreateFontIndirectW pCreateFontIndirectW = NULL;
fEnumFontFamiliesExW pEnumFontFamiliesExW = NULL;
fGetWindowLongPtrW   pGetWindowLongPtrW   = NULL;

static HFONT CreateFontIndirectW(CONST LOGFONTW *lplf) {
  if (!pCreateFontIndirectW) {
    HMODULE gdiModule = GetModuleHandleW(u"Gdi32.dll");
    if (!gdiModule) {
      // CreateFontIndirectW returns a null pointer when failing
      return NULL;
    }

    pCreateFontIndirectW = (fCreateFontIndirectW)GetProcAddress(gdiModule, "CreateFontIndirectW");
    if (!pCreateFontIndirectW) {
      return NULL;
    }
  }

  return pCreateFontIndirectW(lplf);
}

static int EnumFontFamiliesExW(HDC hdc, LPLOGFONTW lpLogFont, FONTENUMPROCW lpProc, LPARAM lParam,
                               DWORD dwFlags) {
  if (!pEnumFontFamiliesExW) {
    HMODULE gdiModule = GetModuleHandleW(u"Gdi32.dll");
    if (!gdiModule) {
      // EnumFontFamExProcW returns the last result of lpProc which is nonzero when failing
      return 1;
    }

    pEnumFontFamiliesExW = (fEnumFontFamiliesExW)GetProcAddress(gdiModule, "EnumFontFamiliesExW");
    if (!pEnumFontFamiliesExW) {
      return 1;
    }
  }

  return pEnumFontFamiliesExW(hdc, lpLogFont, lpProc, lParam, dwFlags);
}

static LONG_PTR GetWindowLongPtrW(HWND hWnd, int nIndex) {
  if (!pGetWindowLongPtrW) {
    HMODULE userModule = GetModuleHandleW(u"User32.dll");
    if (!userModule) {
      SetLastError(ERROR_FUNCTION_NOT_CALLED);
      // GetWindowLongPtrW returns 0 when failing
      return 0;
    }

    pGetWindowLongPtrW = (fGetWindowLongPtrW)GetProcAddress(userModule, "EnumFontFamiliesExW");
    if (!pGetWindowLongPtrW) {
      SetLastError(ERROR_FUNCTION_NOT_CALLED);
      return 0;
    }
  }

  return pGetWindowLongPtrW(hWnd, nIndex);
}

#else
#error Building the gdi frontend requires windows support
#endif

static struct GameOutput Output = {0};
static BOOL NeedRedrawButtons = FALSE;
static HWND *buttonHandles = NULL;

// TODO: Switch text rendering from gdi/uniscribe to libschrift for windows versions older than vista.
// TODO: Disable showing console on startup

int CALLBACK EnumFontFamExProcW(CONST LOGFONTW *, CONST TEXTMETRICW *, DWORD, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK EnumFontFamExProcW(CONST LOGFONTW *pFontInfo, CONST TEXTMETRICW *pFontAttribs,
                                DWORD fontType, LPARAM lParam) {
  UNREFERENCED_PARAMETER(pFontAttribs); // Do not need any info about the physical font

  if (fontType != TRUETYPE_FONTTYPE || pFontInfo->lfWeight != FW_NORMAL || pFontInfo->lfItalic
      || pFontInfo->lfUnderline || pFontInfo->lfStrikeOut
      || pFontInfo->lfOutPrecision != OUT_STROKE_PRECIS) {
    return 1;
  }

  memcpy_s((void *)lParam, sizeof(LOGFONTW), pFontInfo, sizeof *pFontInfo);
  return 0;
}

static void HandleOutput(HWND hWnd, HDC hdc, PAINTSTRUCT ps) {
  static size_t buttonHandleCount = 0;

  // TODO: Free previous string
  WCHAR *wcText = s8tows(Output.body);
  if (!wcText) {
    return;
  }

  RECT textPosition = {0};
  textPosition.left = 10;
  textPosition.top = 10;
  textPosition.right = ps.rcPaint.right - 10;
  textPosition.bottom = ps.rcPaint.bottom - 10;
  DrawTextW(hdc, wcText, -1, &textPosition, DT_WORDBREAK);
  free(wcText);

  if (!NeedRedrawButtons) {
    return;
  }
  NeedRedrawButtons = FALSE;

  if (buttonHandles) {
    for (size_t i = 0; i < buttonHandleCount; ++i) {
      DestroyWindow(buttonHandles[i]);
    }
  }

  uint_fast8_t inputCount = 0;
  for (uint_fast8_t i = 0; i < Output.inputCount; ++i) {
    if (!Output.inputs[i].visible) {
      continue;
    }
    ++inputCount;
  }

  if (buttonHandleCount < inputCount) {
    buttonHandleCount = inputCount;
    buttonHandles = realloc(buttonHandles, buttonHandleCount * sizeof *buttonHandles);
    if (!buttonHandles) {
      return;
    }
  }

  RECT clientSize = {0};
  if (!GetClientRect(hWnd, &clientSize)) {
    return;
  }

  int buttonWidth = 100;
  int buttonHeight = 20;
  int maxButtonsPerRow = 3;
  int rowVerticalSeperation = 10;

  int buttonsPerRow = maxButtonsPerRow <= inputCount ? maxButtonsPerRow : inputCount;
  // Fails if inputCount is 0 which should not happen
  int extraRowCount = (inputCount - 1) / maxButtonsPerRow;
  int buttonVerticalSeperation = buttonHeight + rowVerticalSeperation;

  int firstButtonCentreX = clientSize.right / (buttonsPerRow + 1);
  int firstButtonCentreY = clientSize.bottom - buttonHeight
                           - buttonVerticalSeperation * extraRowCount;

  int buttonCornerOffsetX = buttonWidth / 2;
  int firstButtonCornerTopRowY = firstButtonCentreY - buttonHeight / 2;

  HINSTANCE wndInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);

  for (uint_fast8_t i = 0, visibleInputCount = 0; i < Output.inputCount; ++i) {
    if (!Output.inputs[i].visible) {
      continue;
    }

  // TODO: Free previous strings
    wcText = s8tows(Output.inputs[i].title);
    if (!wcText) {
      return;
    }

    // TODO: Use remaining buttons mod buttonsPerRow as buttonsPerRow so that buttons in the bottom
    // row are spaced according to how many are in that row and now how many are in previous rows
    int buttonTopLeftX = (visibleInputCount % buttonsPerRow + 1) * firstButtonCentreX - buttonCornerOffsetX;
    int buttonTopLeftY = firstButtonCornerTopRowY + buttonVerticalSeperation * (visibleInputCount / buttonsPerRow);

    // TODO: Fix tabbing not working despite WS_TABSTOP
    HWND hBtn = CreateWindowExW(0,  u"BUTTON", wcText,
                                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                buttonTopLeftX, buttonTopLeftY, buttonWidth, buttonHeight, hWnd,
                                (HMENU)(intptr_t)visibleInputCount, wndInst, NULL);
    buttonHandles[visibleInputCount] = hBtn;
    free(wcText);
    ++visibleInputCount;
  }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch(msg) {
    case WM_PAINT:
      // Force BeginPaint to allow drawing to the entire screen
      InvalidateRect(hWnd, NULL, TRUE);

      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      if (!hdc) {
        return 0;
      }

      WCHAR *faceName = u"SYMBOLA";
      LOGFONTW fontInfo = {0};
      fontInfo.lfCharSet = ANSI_CHARSET;
      if (FAILED(StringCchCopyW(fontInfo.lfFaceName, LF_FACESIZE, faceName))) {
        goto cleanup_paint;
      }

      LOGFONTW fullFontInfo = {0};
      // This returns whatever the final EnumFontFamExProcW call returned
      // and therefore will return 0 if a font was found and 1 otherwise
      if (EnumFontFamiliesExW(hdc, &fontInfo, EnumFontFamExProcW, (LPARAM)&fullFontInfo, 0)) {
        goto cleanup_paint;
      }

      fullFontInfo.lfHeight        = 40;
      fullFontInfo.lfWidth         = 0;
      fullFontInfo.lfOutPrecision  = OUT_TT_ONLY_PRECIS;
      fullFontInfo.lfClipPrecision = CLIP_DEFAULT_PRECIS;
      // CLEARTYPE_QUALITY requires truetype hence OUT_TT_ONLY_PRECIS
      // If the font is an outline font other than truetype then use ANTIALIASED_QUALITY
      // If on Windows 2000 or older than CLEARTYPE_QUALITY is not supported so use ANTIALIASED_QUALITY
      // If on Windows NT 3.51 or older than ANTIALIASED_QUALITY is not supported so use PROOF_QUALITY
      fullFontInfo.lfQuality       = CLEARTYPE_QUALITY;

      HFONT hFont = CreateFontIndirectW(&fullFontInfo);
      if (!hFont) {
        goto cleanup_paint;
      }
      HFONT hOldFont = SelectObject(hdc, hFont);
      if (!hOldFont) {
        goto cleanup_font;
      }

      HandleOutput(hWnd, hdc, ps);

      SelectObject(hdc, hOldFont);
cleanup_font:
      DeleteObject(hFont);

cleanup_paint:
      EndPaint(hWnd, &ps);
      return 0;

    case WM_COMMAND:
      if (HIWORD(wParam) != BN_CLICKED) {
        return 0;
      }

      enum InputOutcome outcome = HandleGameInput(&Output, LOWORD(wParam));
      switch(outcome) {
        case GetNextOutputOutcome:
          if (GetCurrentGameOutput(&Output)) {
            NeedRedrawButtons = TRUE;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
          }
          // fall through
        case QuitGameOutcome:
          DestroyWindow(hWnd);
          break;
        default:
          break;
      }
      return 0;

    case WM_DESTROY:
      free(buttonHandles);
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// TODO: Empty file on first write?
void PrintError(const char *error, ...) {
  FILE *fp = fopen("stderr.txt", "a");
  bool usingStderr = false;
  if (!fp) {
    fp = stderr;
    usingStderr = true;
  }

  fputs("ERROR: ", fp);

  va_list args;
  va_start(args, error);
  vfprintf(fp, error, args);
  va_end(args);

  fputs(".\n", fp);

  if (!usingStderr) {
    fclose(fp);
  }
}

#if defined(_WIN32)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance); // Always NULL on win32
  UNREFERENCED_PARAMETER(lpCmdLine);     // Don't need command line arguments
#elif defined(_COSMO_SOURCE)
int main(void) {
  struct NtStartupInfo info;
  GetStartupInfo(&info);

  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
  int nCmdShow = info.dwFlags & STARTF_USESHOWWINDOW ? info.wShowWindow : kNtSwNormal;
#endif

  if (!SetupBackend()) {
    return 1;
  }
  if (!GetCurrentGameOutput(&Output)) {
    CleanupGame(&Output);
    return 1;
  }
  NeedRedrawButtons = TRUE;

  WNDCLASSW wnd     = {0};
  wnd.style         = CS_VREDRAW | CS_HREDRAW;
  wnd.lpfnWndProc   = WndProc;
  wnd.hInstance     = hInstance;
#ifdef _COSMO_SOURCE
  wnd.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  wnd.hCursor       = LoadCursor(NULL, kNtIdcArrow);
#else
  wnd.hIcon         = LoadImageW(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0,
                                 LR_DEFAULTSIZE | LR_SHARED);
  wnd.hCursor       = LoadImageW(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0,
                                 LR_DEFAULTSIZE | LR_SHARED);
#endif
  wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wnd.lpszClassName = u"Untitled Text Adventure";
  if (!wnd.hIcon || !wnd.hCursor) {
    return FALSE;
  }

  ATOM class = RegisterClassW(&wnd);
  if (!class) {
    return FALSE;
  }
  void *rClass = MAKEINTRESOURCEW(class);

  HWND hWnd = CreateWindowExW(0, rClass, u"Test window",
                              WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);
  if(!hWnd) {
    return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  if (!UpdateWindow(hWnd)) {
    return FALSE;
  }

  MSG msg;
  while (GetMessageW(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
  }

  // TODO: Add UnregisterClassW support to cosmo
#ifndef _COSMO_SOURCE
  UnregisterClassW(rClass, hInstance);
#endif
  CleanupGame(&Output);
  CleanupBackend();
  return msg.wParam;
}
