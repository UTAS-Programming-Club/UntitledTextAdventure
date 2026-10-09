#include <inttypes.h>  // for uint32_t, uint_fast8_t
#include <locale.h>    // for LC_ALL, setlocale
#include <raylib.h>    // for WHITE, Image, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, BLACK, BLANK, BeginDrawing, ClearBackground, CloseWindow, Color, DrawTexture, EndDrawing, GetCharPressed, ImageClearBackground, ImageDrawImage, InitWindow, LoadTextureFromImage, Texture2D, UnloadTexture, UpdateTexture, WindowShouldClose
#include <schrift.h>   // for SFT_LMetrics, SFT_Image, SFT, SFT_GMetrics, SFT_Kerning, sft_freefont, SFT_Glyph, sft_lmetrics, SFT_DOWNWARD_Y, SFT_UChar, sft_gmetrics, sft_kerning, sft_loadfile, sft_lookup, sft_render
#include <stdio.h>     // for fputs, stderr, size_t
#include <stdlib.h>    // for EXIT_FAILURE, EXIT_SUCCESS, free, malloc
#include <string.h>    // for strlen
#include <uchar.h>     // for mbrtoc32
#include <wchar.h>     // for mbstate_t

#include "backend.h"   // for GameInfo, Screen, Action, backend_cleanup, backend_input, backend_register_extension, backend_setup
#include "gen/ext1.h"  // for Ext1_RoomCount, Ext1_Rooms

// Based on https://github.com/lhf/libschrift-show/blob/83af300/show.c
static bool SchriftDrawText(SFT *const sft, Image *const textImage, const char *const text, const int posX, const int posY, const Color colour, int *const finalX, int *const finalY) {
  const size_t textLen = strlen(text);

  SFT_LMetrics lmtx;
  if (0 > sft_lmetrics(sft, &lmtx)) {
    return false;
  }

  double x = posX;
  double y = posY + lmtx.ascender + lmtx.lineGap;
  SFT_Glyph oldGid = 0;

  size_t consumedBytes = 0;
  for (size_t i = 0; i < textLen; i += consumedBytes) {
    if ('\n' == text[i]) {
      x = posX;
      y += (int)(lmtx.ascender + lmtx.descender + lmtx.lineGap);
      oldGid = 0;
      continue;
    }

    SFT_UChar codePoint;
    mbstate_t state = {};
    consumedBytes = mbrtoc32(&codePoint, text + i, textLen - i, &state);
    if (0 > consumedBytes) {
      return false;
    }

    SFT_Glyph gid;
    if (0 > sft_lookup(sft, codePoint, &gid)) {
      return false;
    }

    SFT_GMetrics mtx;
    if (0 > sft_gmetrics(sft, gid, &mtx)) {
      return false;
    }

    SFT_Kerning kern;
    if (0 > sft_kerning(sft, oldGid, gid, &kern)) {
      return false;
    }

    x += kern.xShift;

    SFT_Image img = {
      .width  = mtx.minWidth,
      .height = mtx.minHeight,
    };
    char pixels[img.width * img.height];
    img.pixels = pixels;
    if (0 < sft_render(sft, gid, img)) {
      return false;
    }

    Image rlImg = {
      pixels,
      img.width,
      img.height,
      1,
      PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
    };
    ImageDrawImage(textImage, rlImg, (int)(x + mtx.leftSideBearing), (int)(y + mtx.yOffset), colour);

    x += mtx.advanceWidth;
    oldGid = gid;
  }

  if (nullptr != finalX) {
    *finalX = (int)x;
  }
  if (nullptr != finalY) {
    *finalY = (int)y;
  }

  return true;
}

int main() {
  setlocale(LC_ALL, "en_AU.utf8");

  int result = EXIT_SUCCESS;
  struct GameInfo game = {};
  if (!backend_setup(&game)) {
    fputs("Error in backend_setup\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  if (!backend_register_extension(&game, Ext1_RoomCount, Ext1_Rooms)) {
    fputs("Error in backend_register_extension\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  SFT sft = {
		.xScale = 20,
		.yScale = 20,
		.flags  = SFT_DOWNWARD_Y,
	};
  sft.font = sft_loadfile("AtkinsonHyperlegibleMono-VariableFont_wght.ttf");
  if (nullptr == sft.font) {
    fputs("Error in sft_loadfile\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  SFT_LMetrics lmtx;
  if (0 > sft_lmetrics(&sft, &lmtx)) {
    fputs("Error in sft_lmetrics\n", stderr);
    sft_freefont(sft.font);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  constexpr int width = 1280;
  constexpr int height = 720;

  InitWindow(width, height, "Untitled Text Adventure");

  // TODO: Use PIXELFORMAT_UNCOMPRESSED_R8G8B8 to save space?
  uint32_t *textBuffer = malloc(width * height * sizeof textBuffer);
  if (nullptr == textBuffer) {
    fputs("Error in malloc\n", stderr);
    sft_freefont(sft.font);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }
  Image textImage = {
    textBuffer,
    width,
    height,
    1,
    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
  };
  Texture2D textTexture = LoadTextureFromImage(textImage);

  bool updateText = true;
  while (!game.quit && !WindowShouldClose()) {
    if (updateText) {
      updateText = false;

      ImageClearBackground(&textImage, BLANK);

      const char *const body = game.screen->body_generator(&game, game.screen);
      if (nullptr == body) {
        fputs("Error in body_generator\n", stderr);
        result = EXIT_FAILURE;
        break;
      }

      int textY = 0;

      if (!SchriftDrawText(&sft, &textImage, body, 10, textY, WHITE, nullptr, &textY)) {
        fputs("Error in SchriftDrawText\n", stderr);
        result = EXIT_FAILURE;
        break;
      }

      textY += (int)(lmtx.ascender + lmtx.descender + lmtx.lineGap);
      uint_fast8_t id = 0;
      for (size_t i = 0; i < game.screen->actionCount; ++i) {
        const struct Action *action = game.screen->actions[i];
        if (!action->visibility_checker(&game, action)) {
          continue;
        }

        int textX;
        const char digitStr[] = { (char)('0' + id + 1), ':', ' ', '\0' };
        if (!SchriftDrawText(&sft, &textImage, digitStr, 10, textY, WHITE, &textX, nullptr)) {
          fputs("Error in SchriftDrawText\n", stderr);
          result = EXIT_FAILURE;
          game.quit = true;
          break;
        }
        if (!SchriftDrawText(&sft, &textImage, action->title, textX, textY, WHITE, nullptr, &textY)) {
          fputs("Error in SchriftDrawText\n", stderr);
          result = EXIT_FAILURE;
          game.quit = true;
          break;
        }

        ++id;
      }

      UpdateTexture(textTexture, textBuffer);
    }

    unsigned char input = (unsigned char)GetCharPressed();
    if (0 != input) {
      if (!backend_input(&game, input - '1')) {
        fputs("Error in backend_input", stderr);
        result = EXIT_FAILURE;
        break;
      }

      updateText = true;
    }

    BeginDrawing();

    ClearBackground(BLACK);

    DrawTexture(textTexture, 0, 0, WHITE);

    EndDrawing();
  }

  UnloadTexture(textTexture);
  free(textBuffer);
  CloseWindow();
  sft_freefont(sft.font);
  backend_cleanup(&game);

  return result;
}
