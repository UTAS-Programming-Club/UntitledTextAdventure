#include <memory.h>
#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>

#include "../backend/game.h"
#include "../shared/crossprint.h"
#include "frontend.h"

static struct GameOutput Output = {0};
static BOOL NeedRedrawButtons = FALSE;
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

  wchar_t *wcText = c32towc(Output.body);
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

  if (!NeedRedrawButtons) {
    return;
  }
  NeedRedrawButtons = FALSE;

  if (buttonHandles) {
    for (size_t i = 0; i < buttonHandleCount; ++i) {
      DestroyWindow(buttonHandles[i]);
    }
  }

  if (buttonHandleCount < Output.inputCount) {
    buttonHandleCount = Output.inputCount;
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

  int buttonsPerRow = maxButtonsPerRow <= Output.inputCount ? maxButtonsPerRow : Output.inputCount;
  int extraRowCount = Output.inputCount / maxButtonsPerRow;
  int buttonVerticalSeperation = buttonHeight + rowVerticalSeperation;

  int firstButtonCentreX = clientSize.right / (buttonsPerRow + 1);
  int firstButtonCentreY = clientSize.bottom - buttonHeight
                           - buttonVerticalSeperation * extraRowCount;

  int buttonCornerOffsetX = buttonWidth / 2;
  int firstButtonCornerTopRowY = firstButtonCentreY - buttonHeight / 2;

  HINSTANCE wndInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);

  for (uint8_t i = 0; i < Output.inputCount; ++i) {
    wcText = c32towc(Output.inputs[i].title);
    if (!wcText) {
      return;
    }

    // TODO: Use remaining buttons mod buttonsPerRow as buttonsPerRow so that buttons in the bottom
    // row are spaced according to how many are in that row and now how many are in previous rows
    int buttonTopLeftX = (i % buttonsPerRow + 1) * firstButtonCentreX - buttonCornerOffsetX;
    int buttonTopLeftY = firstButtonCornerTopRowY + buttonVerticalSeperation * (i / buttonsPerRow);

    // TODO: Fix tabbing not working despite WS_TABSTOP
    HWND hBtn = CreateWindowW(L"BUTTON", wcText,
                              WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                              buttonTopLeftX, buttonTopLeftY, buttonWidth, buttonHeight, hWnd,
                              (HMENU)(intptr_t)i, wndInst, NULL);
    buttonHandles[i] = hBtn;
    free(wcText);
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

      enum GameInputOutcome outcome = HandleGameInput(Output.screenID, LOWORD(wParam));
      switch(outcome) {
        case GetNextOutput:
          FreeScreen(&Output);
          if (GetCurrentGameOutput(&Output)) {
            NeedRedrawButtons = TRUE;
            InvalidateRect(hWnd, NULL, TRUE);
          }
          break;
        case QuitGame:
          FreeScreen(output);
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
  if (!GetCurrentGameOutput(&Output)) {
    CleanupGame();
  }
  NeedRedrawButtons = TRUE;

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

  HWND hWnd = CreateWindowW(rClass, L"Test window",
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

  UnregisterClassW(rClass, hInstance);
  CleanupGame();
  return msg.wParam;
}
