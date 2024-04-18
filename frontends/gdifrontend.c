#include <memory.h>
#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>

#include "../backend/game.h"
#include "../shared/crossprint.h"
#include "frontend.h"

static uint32_t ScreenID = UINT32_MAX;
static HWND *buttonHandles = NULL;

// TODO: Switch text rendering from gdi/uniscribe to libschrift for windows versions older than vista.
// TODO: Disable showing console on startup

int CALLBACK EnumFontFamExProcW(const LOGFONTW *, const TEXTMETRICW *, DWORD, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK EnumFontFamExProcW(const LOGFONTW *pFontInfo, const TEXTMETRICW *pFontAttribs,
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

  struct GameOutput output;
  if (!GetCurrentGameOutput(&output)) {
    return;
  }

  wchar_t *wcText = c32towc(output.body);
  if (!wcText) {
    return;
  }

  RECT textPosition = {0};
  textPosition.left = 10;
  textPosition.top = 10;
  textPosition.right = ps.rcPaint.right - 10;
  textPosition.bottom = ps.rcPaint.bottom - 10;
  DrawTextW(hdc, wcText, -1, &textPosition, 0);
  free(wcText);

  if (ScreenID == output.screenID) {
    return;
  }
  ScreenID = output.screenID;

  if (buttonHandles) {
    for (size_t i = 0; i < buttonHandleCount; ++i) {
      DestroyWindow(buttonHandles[i]);
    }
  }
  if (buttonHandleCount < output.inputCount) {
    buttonHandleCount = output.inputCount;
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
  int firstButtonCentreX = clientSize.right / (output.inputCount + 1);
  int buttonCentreY = clientSize.bottom - buttonHeight;

  HINSTANCE wndInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);

  for (uint8_t i = 0; i < output.inputCount; ++i) {
    wcText = c32towc(output.inputs[i].title);
    if (!wcText) {
      return;
    }
    int buttonTopLeftX = (i + 1) * firstButtonCentreX - buttonWidth / 2;
    int buttonTopLeftY = buttonCentreY - buttonHeight / 2;

    HWND hBtn = CreateWindowW(L"BUTTON", wcText, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                  buttonTopLeftX, buttonTopLeftY, buttonWidth, buttonHeight, hWnd, (HMENU)(intptr_t)i,
                  wndInst, NULL);
    buttonHandles[i] = hBtn;
    free(wcText);
  }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(wParam); // Not relevant for any messages used
  UNREFERENCED_PARAMETER(lParam); // Not relevant for any messages used

  switch(msg) {
    case WM_PAINT: ;
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      if (!hdc) {
        return 0;
      }

      WCHAR *faceName = L"SYMBOLA";
      LOGFONTW fontInfo = {0};
      fontInfo.lfCharSet = ANSI_CHARSET;
      if (StringCchCopyW(fontInfo.lfFaceName, LF_FACESIZE, faceName)) {
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

      enum GameInputOutcome outcome = HandleGameInput(ScreenID, LOWORD(wParam));
      switch(outcome) {
        case GetNextOutput:
          InvalidateRect(hWnd, NULL, TRUE);
          break;
        case QuitGame:
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance); // Always NULL on win32
  UNREFERENCED_PARAMETER(lpCmdLine);     // Don't need command line arguments

  if (!SetupGame()) {
    return 1;
  }

  WNDCLASSEXW wnd = {0};
  wnd.cbSize        = sizeof wnd;
  wnd.style         = CS_VREDRAW | CS_HREDRAW;
  wnd.lpfnWndProc   = WndProc;
  wnd.hInstance     = hInstance;
  wnd.hIcon         = LoadImageW(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0,
                                 LR_DEFAULTSIZE | LR_SHARED);
  wnd.hCursor       = LoadImageW(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0,
                                 LR_DEFAULTSIZE | LR_SHARED);
  wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wnd.lpszClassName = L"Untitled Text Adventure";
  if (!wnd.hIcon || !wnd.hCursor) {
    return FALSE;
  }

  ATOM class = RegisterClassExW(&wnd);
  if (!class) {
    return FALSE;
  }
  void *rClass = MAKEINTRESOURCEW(class);

  HWND hWnd = CreateWindowW(rClass, L"Test window", WS_OVERLAPPEDWINDOW,
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

  UnregisterClassW(rClass, hInstance);
  CleanupGame();
  return msg.wParam;
}
