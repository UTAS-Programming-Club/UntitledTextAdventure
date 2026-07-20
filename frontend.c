#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "backend.h"
#include "ext1.h"

int main() {
  int result = EXIT_SUCCESS;
  struct GameInfo game;
  if (!backend_setup(&game)) {
    fputs("Error in backend_setup\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  if (!backend_register_extension(&game, Ext1ScreenCount, Ext1Screens)) {
    fputs("Error in backend_register_extension\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  while (!game.quit) {
    puts(game.screen->body);
    putchar('\n');

    for (size_t i = 0; i < game.screen->actionCount; ++i) {
      printf("%zu: %s\n", i + 1, game.screen->actions[i]->title);
    }

    uint8_t input;
    if (scanf("%" SCNu8, &input) != 1) {
      fputs("Error in scanf\n", stderr);
      result = EXIT_FAILURE;
      break;
    }

    if (!backend_input(&game, input - 1)) {
      fputs("Error in backend_input", stderr);
      result = EXIT_FAILURE;
      break;
    }

    putchar('\n');
  }

  backend_cleanup(&game);

  return result;
}
