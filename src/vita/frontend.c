#include <inttypes.h>  // for uint8_t, SCNu8
#include <stdio.h>     // for fputs, stderr, putchar, printf, puts, scanf, size_t
#include <stdlib.h>    // for EXIT_FAILURE, EXIT_SUCCESS

#include "backend.h"   // for GameInfo, Screen, backend_cleanup, Action, backend_input, backend_register_extension, backend_setup
#include "gen/ext1.h"  // for Ext1_RoomCount, Ext1_Rooms

#include <string.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

int main(void) {
  int result = EXIT_SUCCESS;

  SceCtrlData pad;
  vita2d_pgf *pgf;
  vita2d_pvf *pvf;

  vita2d_init();
  vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

  pgf = vita2d_load_default_pgf();
  pvf = vita2d_load_default_pvf();

  memset(&pad, 0, sizeof(pad));

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

  int option = 0;
  int pad_mask[3] = { 0, 0, 0 }; // down, up, 0 -> not pressed, 1 -> pressing, 2 -> just let go
  while (!game.quit) {
	// read input
	sceCtrlPeekBufferPositive(0, &pad, 1);
	
	// trying? might fail
	if ((pad.buttons & SCE_CTRL_DOWN) == SCE_CTRL_DOWN) {
		pad_mask[0] = 1;
	} else {
		if (pad_mask[0] == 1)
			pad_mask[0] = 2;
		else
			pad_mask[0] = 0;
	}
	
	if ((pad.buttons & SCE_CTRL_UP) == SCE_CTRL_UP) {
		pad_mask[1] = 1;
	} else {
		if (pad_mask[1] == 1)
			pad_mask[1] = 2;
		else
			pad_mask[1] = 0;
	}
	
	if ((pad.buttons & SCE_CTRL_CIRCLE) == SCE_CTRL_CIRCLE) {
		pad_mask[2] = 1;
	} else {
		if (pad_mask[2] == 1)
			pad_mask[2] = 2;
		else
			pad_mask[2] = 0;
	}

	if (pad_mask[1] == 2)
		--option;
	if (pad_mask[0] == 2)
		++option;

    const char *const body = game.screen->body_generator(&game, game.screen);
    if (nullptr == body) {
      fputs("Error in body_generator\n", stderr);
      result = EXIT_FAILURE;
      break;
    }

	uint8_t amount = 0;
	for (size_t i = 0; i < game.screen->actionCount; ++i) {
      const struct Action *action = game.screen->actions[i];
      if (!action->visibility_checker(&game, action)) {
        continue;
      }

	  ++amount;
	}
	
	if (option < 0)
		option = amount - 1;
	option %= amount;
	
	// rendering
	vita2d_start_drawing();
	vita2d_clear_screen();

	
	vita2d_pgf_draw_text(pgf, 30, 60, RGBA8(255,255,255,255), 2.0f, body);

    uint8_t id = 0;
    for (size_t i = 0; i < game.screen->actionCount; ++i) {
      const struct Action *action = game.screen->actions[i];
      if (!action->visibility_checker(&game, action)) {
        continue;
      }
	  
	  // 15 per character so far?
	  size_t height = 400 + id * 40;

	  if (option == id)
		vita2d_pvf_draw_text(pvf, 30, height, RGBA8(255, 255, 255, 255), 2.0f, ">");
	  vita2d_pvf_draw_text(pvf, 60, height, RGBA8(255, 255, 255, 255), 2.0f, action->title);
      ++id;
    }
	
	if (pad_mask[2] == 2) {
      if (!backend_input(&game, option)) {
        fputs("Error in backend_input", stderr);
        result = EXIT_FAILURE;
        break;
      }
	}
	
	vita2d_end_drawing();
	vita2d_swap_buffers();
  }

  backend_cleanup(&game);
  vita2d_fini();

  // it is BSD I should be lazier with this
  vita2d_free_pgf(pgf);
  vita2d_free_pvf(pvf);

  sceKernelExitProcess(0);
  return result;
}
