// Based on https://github.com/ps2dev/ps2sdk/blob/f556130/ee/rpc/pad/samples/pad/pad.c
// and https://github.com/ps2dev/gsKit/blob/8ef73d0/examples/font/font.c

#include <gsKit.h>
#include <gsToolkit.h>
#include <kernel.h>
#include <libpad.h>
#include <loadfile.h>
#include <schrift.h>
#include <stdio.h>
#include <util/utf8_to_utf32.h>  // for utf8_to_utf32

#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"

#include "backend.h"   // for GameInfo, Screen, Action, backend_cleanup, backend_input, backend_register_extension, backend_setup
#include "gen/ext1.h"  // for Ext1_RoomCount, Ext1_Rooms

static char padBuf[256] __attribute__((aligned(64)));

constexpr uint16_t ScreenWidth = 720;
constexpr uint16_t ScreenHeight = 480;

// Based on https://github.com/raysan5/raylib/blob/656d42c4/src/rtextures.c#L4062-L4212
static void imageDrawImage(unsigned char *const dst, const SFT_Image *const src, const uint16_t posX, const uint16_t posY, const uint64_t tint) {
	if (nullptr == src || nullptr == dst || nullptr == src->pixels || 0 == src->width || 0 == src->height ||
		posX + src->width >= ScreenWidth || posY + src->height >= ScreenHeight) {
		return;
	}

	const size_t strideSrc = ((size_t)src->width * 8) >> 3;
	const size_t bytesPerPixelSrc = strideSrc / (size_t)src->width;

	const size_t strideDst = (ScreenWidth * 24) >> 3;
	const size_t bytesPerPixelDst = strideDst / ScreenWidth;

	unsigned char *pSrcBase = src->pixels;
	unsigned char *pDstBase = dst + ((size_t)posY * ScreenWidth + posX) * bytesPerPixelDst;

	for (int y = 0; y < src->height; y++) {
		unsigned char *pSrc = pSrcBase;
		unsigned char *pDst = pDstBase;

		for (int x = 0; x < src->width; x++) {
			pDst[0] = pSrc[0]; // * ((tint >>  0) & 0xFF);
			pDst[1] = pSrc[0]; // * ((tint >>  8) & 0xFF);
			pDst[2] = pSrc[0]; // * ((tint >> 16) & 0xFF);

			pDst += bytesPerPixelDst;
			pSrc += bytesPerPixelSrc;
		}

		pSrcBase += strideSrc;
		pDstBase += strideDst;
	}
}


// Based on https://github.com/lhf/libschrift-show/blob/83af300/show.c
static bool schriftDrawText(SFT *const sft, unsigned char *const dst, const char *const text, const uint16_t posX, const uint16_t posY, const uint64_t colour, uint16_t *const finalX, uint16_t *const finalY) {
	const size_t textLen = strlen(text);
	SFT_UChar codePoints[textLen];
	int codePointCount = utf8_to_utf32((const uint8_t *)text, codePoints, (int)textLen + 1);
	if (0 == codePointCount) {
		return false;
	}

	SFT_LMetrics lmtx;
	if (0 > sft_lmetrics(sft, &lmtx)) {
		return false;
	}

	double x = posX;
	double y = posY + lmtx.ascender + lmtx.lineGap;
	SFT_Glyph oldGid = 0;

	for (size_t i = 0; i < codePointCount; ++i) {
		if ('\n' == text[i]) {
			x = posX;
			y += lmtx.ascender + lmtx.descender + lmtx.lineGap;
			oldGid = 0;
			continue;
		}

		SFT_Glyph gid;
		if (0 > sft_lookup(sft, codePoints[i], &gid)) {
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

		imageDrawImage(dst, &img, (uint16_t)(x + mtx.leftSideBearing), (uint16_t)(y + mtx.yOffset), colour);

		x += mtx.advanceWidth;
		oldGid = gid;
	}

	if (nullptr != finalX) {
		*finalX = (uint16_t)x;
	}
	if (nullptr != finalY) {
		*finalY = (uint16_t)y;
	}

	return true;
}


static void loadModules(void) {
	int ret = SifLoadModule("rom0:SIO2MAN", 0, nullptr);
	if (ret < 0) {
		printf("sifLoadModule sio failed: %d\n", ret);
		SleepThread();
	}

	ret = SifLoadModule("rom0:PADMAN", 0, nullptr);
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
	int ret = padGetState(port, slot);
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

	uint64_t Black = GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x00);
	uint64_t White = GS_SETREG_RGBA(0xFF, 0xFF, 0xFF, 0x80);
	uint64_t Red   = GS_SETREG_RGBA(0xFF, 0x00, 0x00, 0x80);

	gsGlobal->Mode = GS_MODE_DTV_480P;
	gsGlobal->Interlace = GS_NONINTERLACED;
	gsGlobal->Field = GS_FRAME;
	gsGlobal->Width = ScreenWidth;
	gsGlobal->Height = ScreenHeight;
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
	if(0 == ret) {
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

	SFT sft = {
		.xScale = 20,
		.yScale = 20,
		.flags  = SFT_DOWNWARD_Y,
	};
	sft.font = sft_loadfile("AtkinsonHyperlegibleMono-VariableFont_wght.ttf");
	if (nullptr == sft.font) {
		puts("Error in sft_loadfile\n");
		backend_cleanup(&game);
		return EXIT_FAILURE;
	}

	SFT_LMetrics lmtx;
	if (0 > sft_lmetrics(&sft, &lmtx)) {
		puts("Error in sft_lmetrics\n");
		sft_freefont(sft.font);
		backend_cleanup(&game);
		return EXIT_FAILURE;
	}

	unsigned char *const bmp = malloc(3 * ScreenWidth * ScreenHeight);
	GSTEXTURE Bitmap = { 0 };

	bool needDraw = true;
	while(!game.quit) {
		if (0 != getPadState(port, slot, &buttons)) {
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

			memset(bmp, 0, 3 * ScreenWidth * ScreenHeight);

			const char *const body = game.screen->body_generator(&game, game.screen);
			if (nullptr == body) {
				puts("Error in body_generator");
				break;
			}

			uint16_t textY = 0;

			if (!schriftDrawText(&sft, bmp, body, 10, textY, White, nullptr, &textY)) {
				puts("Error in schriftDrawText");
				break;
			}

			textY += (uint16_t)(lmtx.ascender + lmtx.descender + lmtx.lineGap);
			uint_fast8_t id = 0;
			for (size_t i = 0; i < game.screen->actionCount; ++i) {
				const struct Action *action = game.screen->actions[i];
				if (!action->visibility_checker(&game, action)) {
					continue;
				}

				uint16_t textX = 10;
				if (selectedIdx == i && !schriftDrawText(&sft, bmp, "> ", textX, textY, White, &textX, nullptr)) {
					puts("Error in SchriftDrawText");
					game.quit = true;
					break;
				}

				if (!schriftDrawText(&sft, bmp, action->title, textX, textY, White, nullptr, &textY)) {
					puts("Error in SchriftDrawText");
					game.quit = true;
					break;
				}

				++id;
			}

			loadbmp_encode_file("screen.bmp", bmp, ScreenWidth, ScreenHeight, LOADBMP_RGB);

			gsKit_vram_clear(gsGlobal);
			gsKit_texture_bmp(gsGlobal, &Bitmap, "screen.bmp");

			gsKit_clear(gsGlobal, Black);

			gsKit_prim_sprite_texture(gsGlobal,	&Bitmap, 0, 0, 0, 0,
																Bitmap.Width, Bitmap.Height,
																Bitmap.Width, Bitmap.Height,
																2, White);
		}

		gsKit_queue_exec(gsGlobal);
		gsKit_sync_flip(gsGlobal);
	}

	// Loop and allow `ps2client reset` to re-enter bootloader
	SleepThread();
	while (true) {
	}

	// free(bmp);
	// backend_cleanup(&game);
	// sft_freefont(sft.font);

	return EXIT_SUCCESS;
}
