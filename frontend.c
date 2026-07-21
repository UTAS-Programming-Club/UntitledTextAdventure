#include "backend.h"   // for GameInfo, Screen, backend_cleanup, Action, backend_input, backend_register_extension, backend_setup
#include "ext1.h"      // for Ext1RoomCount, Ext1Rooms

int main(void) {
  int result = EXIT_SUCCESS;
  struct GameInfo game;
  if (!backend_setup(&game)) {
    fputs("Error in backend_setup\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  if (!backend_register_extension(&game, Ext1RoomCount, Ext1Rooms)) {
    fputs("Error in backend_register_extension\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  while (!game.quit) {
    const char *body = game.screen->body_generator(&game);
    if (NULL == body) {
      fputs("Error in body_generator\n", stderr);
      result = EXIT_FAILURE;
      break;
    }

    puts(body);
    putchar('\n');

    uint8_t id = 0;
    for (size_t i = 0; i < game.screen->actionCount; ++i) {
      const struct Action *action = game.screen->actions[i];
      if (!action->visibility_checker(&game, action)) {
        continue;
      }

      putchar('1' + id);
      fputs(": ", stdout);
      puts(action->title);
      ++id;
    }

    uint8_t input = 0;
    while (input < 1 || input > 9) {
      char inputChar;
      if (scant(sizeof(uint8_t), &inputChar) != 1) {
        fputs("Error in scanf\n", stderr);
        result = EXIT_FAILURE;
        break;
      }

      input = inputChar - '0';
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

void _start(void) {
	int out = main();

	// exit
	__asm__ (
		"movq %0, %%rdi\n\t" // value
		"movq $60, %%rax\n\t" // exit
		"syscall"
		: /* no out */
		: "g" ((unsigned long long int)out)
		: "%rdi", "%rax"
	);
}
