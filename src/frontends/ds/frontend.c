#include <inttypes.h>  // for uint8_t, SCNu8
#include <nds.h>       // just look at all the weird functions idk
#include <stdio.h>     // for fputs, stderr, putchar, printf, puts, scanf, size_t
#include <stdlib.h>    // for EXIT_FAILURE, EXIT_SUCCESS

#include "backend.h"   // for GameInfo, Screen, backend_cleanup, Action, backend_input, backend_register_extension, backend_setup
#include "ext1.h"      // for Ext1_RoomCount, Ext1_Rooms

int main() {
  int result = EXIT_SUCCESS;

  consoleDemoInit();

  // copied mostly from Graphics/Printing/print_both_screens
  PrintConsole topScreen;
  PrintConsole bottomScreen;

  videoSetMode(MODE_0_2D);
  videoSetModeSub(MODE_0_2D);

  vramSetBankA(VRAM_A_MAIN_BG);
  vramSetBankC(VRAM_C_SUB_BG);

  consoleInit(&topScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
  consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

  size_t option = 0; // used for selecting where to go

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

  while (pmMainLoop() && !game.quit) {
    swiWaitForVBlank();
    const char *const body = game.screen->body_generator(&game, game.screen);
    if (nullptr == body) {
      result = EXIT_FAILURE;
      break;
    }

    consoleSelect(&topScreen);
    consoleClear();
    iprintf("\n%s\n\n", body);

    consoleSelect(&bottomScreen);
    consoleClear();

    size_t nopt = 0;
    for (size_t i = 0; i < game.screen->actionCount; ++i) {
      const struct Action *action = game.screen->actions[i];
      if (!action->visibility_checker(&game, action)) {
        continue;
      }

      if (option == nopt) {
        iprintf("> ");
      }

      iprintf("%s\n", action->title);

      ++nopt;
    }

    // fine to be broken for a frame
    option %= nopt;

    scanKeys();
    int keys = keysDown();

    if (keys != 0) {
      if (keys & KEY_START) {
        if (!backend_input(&game, option)) {
          break;
        }
        option = 0;
      }

      if (keys & KEY_DOWN) {
        ++option;
      }
      if (keys & KEY_UP) {
        --option;
      }
    }
  }

  backend_cleanup(&game);

  return result;
}
