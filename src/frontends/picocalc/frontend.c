#include <hardware/clocks.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2ckbd.h>
#include <lcdspi.h>

#include "backend.h"
#include "ext1.h"

// TODO: Add printf impl for lcd
int main() {
	int result = EXIT_FAILURE;

	struct GameInfo game = {};
	if (!backend_setup(&game)) {
		fputs("Error in backend_setup\n", stderr);
		goto cleanup_backend;
	}

	if (!backend_register_extension(&game, Ext1_RoomCount, Ext1_Rooms)) {
		fputs("Error in backend_register_extension\n", stderr);
		goto cleanup_backend;
	}

	set_sys_clock_khz(133000, true);
	lcd_init();
	init_i2c_kbd();

	result = EXIT_SUCCESS;

	bool updateText = true;
	while (!game.quit) {
		const int8_t input = read_i2c_kbd();
		if (1 <= input) {
			if (!backend_input(&game, input - '1')) {
				fputs("Error in backend_input", stderr);
				result = EXIT_FAILURE;
				break;
			}

			updateText = true;
		}

		if (updateText) {
			updateText = false;

			lcd_clear();
			lcd_reset_coords();

			const char *const body = game.screen->body_generator(&game, game.screen);
			if (nullptr == body) {
				fputs("Error in body_generator\n", stderr);
				result = EXIT_FAILURE;
				break;
			}

			lcd_print_string((char *)body);
			lcd_put_char('\n', 1);

			uint_fast8_t id = 0;
			for (size_t i = 0; i < game.screen->actionCount; ++i) {
				const struct Action *action = game.screen->actions[i];
				if (!action->visibility_checker(&game, action)) {
					continue;
				}

				lcd_put_char('1' + id, 1);
				lcd_print_string(": ");
				lcd_print_string((char *)action->title);
				lcd_put_char('\n', 1);

				++id;
			}
		}
	}

cleanup_backend:
	backend_cleanup(&game);

	return result;
}

