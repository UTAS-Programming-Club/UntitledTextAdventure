#include <inttypes.h>  // for uint8_t
#include <raylib.h>    // for DrawText, WHITE, BLACK, BeginDrawing, ClearBackground, CloseWindow, EndDrawing, GetCharPressed, GetFontDefault, InitWindow, MeasureTextEx, WindowShouldClose
#include <stdio.h>     // for fputs, stderr, size_t
#include <stdlib.h>    // for EXIT_FAILURE, EXIT_SUCCESS

#include "backend.h"   // for GameInfo, Screen, Action, backend_cleanup, backend_input, backend_register_extension, backend_setup
#include "gen/ext1.h"  // for Ext1_RoomCount, Ext1_Rooms

int main() {
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

  InitWindow(1280, 720, "Untitled Text Adventure");

  constexpr int fontSize = 20;
  constexpr int textLineSpacing = 2;
  while (!game.quit && !WindowShouldClose()) {
    const char *const body = game.screen->body_generator(&game, game.screen);
    if (nullptr == body) {
      fputs("Error in body_generator\n", stderr);
      result = EXIT_FAILURE;
      break;
    }

    unsigned char input = (unsigned char)GetCharPressed();
    if (0 != input && !backend_input(&game, input - '1')) {
      fputs("Error in backend_input", stderr);
      result = EXIT_FAILURE;
      break;
    }

    BeginDrawing();

    ClearBackground(BLACK);

    int textY = (int)MeasureTextEx(GetFontDefault(), body, fontSize, 1).y;
    DrawText(body, 10, 10, fontSize, WHITE);

    textY += fontSize + textLineSpacing;
    uint8_t id = 0;
    for (size_t i = 0; i < game.screen->actionCount; ++i) {
      const struct Action *action = game.screen->actions[i];
      if (!action->visibility_checker(&game, action)) {
        continue;
      }

      const char digitStr[] = { (char)('0' + id + 1), ':', '\0' };
      DrawText(digitStr, 10, 10 + textY, fontSize, WHITE);
      DrawText(action->title, 30, 10 + textY, fontSize, WHITE);

      ++id;
      textY += fontSize + textLineSpacing;
    }

    EndDrawing();
  }

  CloseWindow();
  return result;
}
