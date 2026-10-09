// Based on https://github.com/ps2dev/ps2sdk/blob/f556130/ee/rpc/pad/samples/pad/pad.c
// and https://github.com/ps2dev/gsKit/blob/8ef73d0/examples/font/font.c

#include <dmaKit.h>
#include <gsKit.h>
#include <gsToolkit.h>
#include <kernel.h>
#include <libpad.h>
#include <loadfile.h>
#include <stdio.h>

#include "backend.h"   // for GameInfo, Screen, Action, backend_cleanup, backend_input, backend_register_extension, backend_setup
#include "gen/ext1.h"  // for Ext1_RoomCount, Ext1_Rooms

static char padBuf[256] __attribute__((aligned(64)));

static void loadModules(void) {
	int ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if (ret < 0) {
		printf("sifLoadModule sio failed: %d\n", ret);
		SleepThread();
	}

	ret = SifLoadModule("rom0:PADMAN", 0, NULL);
	if (ret < 0) {
		printf("sifLoadModule pad failed: %d\n", ret);
		SleepThread();
	}
}

static int waitPadReady(int port, int slot) {
	int state = padGetState(port, slot);
	int lastState = -1;
	char stateString[16];

	while((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
		if (state != lastState) {
		padStateInt2String(state, stateString);
		printf("Please wait, pad(%d,%d) is in state %s\n", port, slot, stateString);
		}
		lastState = state;
		state = padGetState(port, slot);
	}
	// Was the pad ever 'out of sync'?
	if (lastState != -1) {
		printf("Pad OK!\n");
	}
	return 0;
}

static int initializePad(int port, int slot) {
	waitPadReady(port, slot);

	// When using MMODE_LOCK, user cant change mode with Select button
	padSetMainMode(port, slot, PAD_TYPE_DIGITAL, PAD_MMODE_LOCK);

	waitPadReady(port, slot);

	return 1;
}

static int getPadState(int port, int slot, struct padButtonStatus *buttons) {
	int  ret = padGetState(port, slot);
	while(PAD_STATE_STABLE != ret && PAD_STATE_FINDCTP1 != ret) {
		if(PAD_STATE_DISCONN == ret) {
			printf("Pad(%d, %d) is disconnected\n", port, slot);
		}
		ret = padGetState(port, slot);
	}

	return padRead(port, slot, buttons);
}

int main(int argc, char *argv[static argc]) {
	GSGLOBAL *gsGlobal = gsKit_init_global();

	dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
		    D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_GIF);

	uint64_t Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0x00,0x00);
	uint64_t White = GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x80,0x00);
	uint64_t Red   = GS_SETREG_RGBAQ(0xFF,0x00,0x00,0x80,0x00);

	gsGlobal->Mode = GS_MODE_DTV_480P;
	gsGlobal->Interlace = GS_NONINTERLACED;
	gsGlobal->Field = GS_FRAME;
	gsGlobal->Width = 720;
	gsGlobal->Height = 480;
	gsGlobal->PrimAlpha = GS_BLEND_FRONT2BACK;
	gsGlobal->PSM = GS_PSM_CT16;
	gsGlobal->PSMZ = GS_PSMZ_16;

	gsGlobal->DoubleBuffering = GS_SETTING_ON;
	gsGlobal->ZBuffering = GS_SETTING_ON;

	gsKit_init_screen(gsGlobal);
	gsKit_mode_switch(gsGlobal, GS_PERSISTENT);

	GSFONT *gsFont = gsKit_init_font(GSKIT_FTYPE_BMP_DAT, "dejavu.bmp");
	gsKit_font_upload(gsGlobal, gsFont);

	int port = 0; // 0 -> Connector 1, 1 -> Connector 2
	int slot = 0; // Always zero if not using multitap
	size_t selectedIdx = 0;
	size_t selectedId = 0;
	struct padButtonStatus buttons;
	uint32_t oldPadState = 0;
	loadModules();
	padInit(0);

	int ret = padPortOpen(port, slot, padBuf);
	if(0 ==  ret) {
		printf("padOpenPort failed: %d\n", ret);
		SleepThread();
	}

	if(!initializePad(port, slot)) {
		printf("pad initalization failed!\n");
		SleepThread();
	}

	struct GameInfo game;
	if (!backend_setup(&game)) {
		puts("Error in backend_setup");
		backend_cleanup(&game);
		SleepThread();
	}

	if (!backend_register_extension(&game, Ext1_RoomCount, Ext1_Rooms)) {
		puts("Error in backend_register_extension");
		backend_cleanup(&game);
		SleepThread();
  }

	bool needDraw = true;
	while(!game.quit) {
		ret = getPadState(port, slot, &buttons);
		if (ret != 0) {
			int padState = 0xffff ^ buttons.btns;
			int newPadState = padState & ~oldPadState;
			oldPadState = padState;

			if (PAD_UP & newPadState && 0 < selectedIdx) {
				for (size_t i = selectedIdx - 1; i < game.screen->actionCount; --i) {
					const struct Action *action = game.screen->actions[i];
					if (action->visibility_checker(&game, action)) {
						selectedIdx = i;
					  --selectedId;
						needDraw = true;
						break;
					}
				}
			} else if (PAD_DOWN & newPadState) {
				for (size_t i = selectedIdx + 1; i < game.screen->actionCount; ++i) {
					const struct Action *action = game.screen->actions[i];
					if (action->visibility_checker(&game, action)) {
						selectedIdx = i;
						++selectedId;
						needDraw = true;
						break;
					}
				}
			} else if (PAD_CROSS & newPadState) {
				if (!backend_input(&game, selectedId)) {
					puts("Error in backend_input");
					SleepThread();
				}

				selectedId = 0;
				for (size_t i = 0; i < game.screen->actionCount; ++i) {
					const struct Action *action = game.screen->actions[i];
					if (action->visibility_checker(&game, action)) {
						selectedIdx = i;
						break;
					}
				}
				needDraw = true;
			}
		}

		if (needDraw) {
			needDraw = false;

			gsKit_clear(gsGlobal, Black);

			const char *body = game.screen->body_generator(&game, game.screen);
			if (NULL == body) {
				puts("Error in body_generator");
				break;
			}

			gsKit_font_print_scaled(gsGlobal, gsFont, 50, 50, 1, 2.0f, White, body);
			uint8_t lineCount = 2; // Assume 1 line and want line between body and actions
			// TODO: Fix
			// while (NULL != body) {
			// 	body = strchr(body, '\n');
			// 	++lineCount;
			// }
			lineCount += 4;

			char buf[2];
			buf[1] = '\0';
			for (size_t i = 0; i < game.screen->actionCount; ++i) {
				const struct Action *action = game.screen->actions[i];
				if (!action->visibility_checker(&game, action)) {
					continue;
				}

				buf[0] = '1' + i;
				gsKit_font_print_scaled(gsGlobal, gsFont, 50, 50 + lineCount * gsFont->CharHeight * 2.0f, 1, 2.0f, White, action->title);
				if (selectedIdx == i) {
					gsKit_prim_line(gsGlobal, 50, 50 + lineCount * gsFont->CharHeight * 2.0f + 0.85f * gsFont->CharHeight * 2.0f,
																	 200, 50 + lineCount * gsFont->CharHeight * 2.0f + 0.85f * gsFont->CharHeight * 2.0f, 1, Red);
				}
				++lineCount;
			}
		}

		gsKit_queue_exec(gsGlobal);
		gsKit_sync_flip(gsGlobal);
	}

	// Loop and allow `ps2client reset` to re-enter bootloader
	SleepThread();
	while (true) {
	}

	return 0;
}