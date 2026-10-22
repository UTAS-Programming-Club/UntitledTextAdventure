// Based on https://github.com/ps2dev/ps2sdk/blob/f556130/ee/rpc/pad/samples/pad/pad.c
// and https://github.com/ps2dev/gsKit/blob/8ef73d0/examples/font/font.c

#include <gsKit.h>							 // for DMA_CHANNEL_GIF, D_CTRL_MFD_OFF, D_CTRL_RCYC_8, D_CTRL_RELE_OFF, D_CTRL_STD_OFF, D_CTRL_STS_UNSPEC, dmaKit_chan_init, dmaKit_init
																 //     gsKit_clear, gsKit_mode_switch, gsKit_queue_exec, gsKit_sync_flip, gsKit_vram_clear
																 //     GS_SETREG_RGBA, GS_SETTING_ON, GSGLOBAL, GSTEXTURE, GS_BLEND_FRONT2BACK, GS_FRAME, GS_MODE_DTV_480P, GS_NONINTERLACED, GS_PERSISTENT, GS_PSMZ_16, GS_PSM_CT16, gsKit_init_global, gsKit_init_screen
																 //     gsKit_prim_sprite_texture
#include <gsToolkit.h>           // for gsKit_texture_bmp
#include <kernel.h>              // for SleepThread
#include <libpad.h>              // for padGetState, PAD_STATE_FINDCTP1, PAD_STATE_STABLE, padButtonStatus, PAD_CROSS, PAD_DOWN, PAD_MMODE_LOCK, PAD_STATE_DISCONN, PAD_TYPE_DIGITAL, PAD_UP, padInit, padPortOpen, padRead, padSetMainMode, padStateInt2String
#include <loadfile.h>            // for SifLoadModule
#include <malloc.h>
#include <schrift.h>             // for SFT_Image, SFT_LMetrics, SFT_GMetrics, SFT, SFT_Kerning, SFT_Glyph, sft_lmetrics, SFT_DOWNWARD_Y, SFT_UChar, sft_gmetrics, sft_kerning, sft_loadfile, sft_lookup, sft_render
#include <stdint.h>              // for uint16_t, uint64_t, uint8_t, uint32_t, uint_fast8_t
#include <stdio.h>               // for size_t, stderr, fputs, printf, fprintf
#include <stdlib.h>              // for EXIT_FAILURE, EXIT_SUCCESS, malloc
#include <string.h>              // for memset, strlen
#include <util/utf8_to_utf32.h>  // for utf8_to_utf32

#include "backend.h"             // for GameInfo, Screen, Action, backend_input, backend_register_extension, backend_setup
#include "ext1.h"                // for Ext1_RoomCount, Ext1_Rooms

extern int gsKit_texture_finish(GSGLOBAL *gsGlobal, GSTEXTURE *Texture);

static char padBuf[256] __attribute__((aligned(64)));

constexpr uint16_t ScreenWidth = 720;
constexpr uint16_t ScreenHeight = 480;

// Based on https://github.com/raysan5/raylib/blob/656d42c4/src/rtextures.c#L4062-L4212
static void imageDrawImage(uint8_t *const dst, const SFT_Image *const src, const uint16_t posX, const uint16_t posY, const uint64_t tint) {
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
static bool schriftDrawText(SFT *const sft, uint8_t *const textImage, const char *const text, const uint16_t posX, const uint16_t posY, const uint64_t colour, uint16_t *const finalX, uint16_t *const finalY) {
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

		imageDrawImage(textImage, &img, (uint16_t)(x + mtx.leftSideBearing), (uint16_t)(y + mtx.yOffset), colour);

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

int gsKit_texture_framebuffer(GSGLOBAL *gsGlobal, GSTEXTURE *Texture, const uint8_t *imageData, const size_t imageSize) {
	Texture->Width = ScreenWidth;
	Texture->Height = ScreenHeight;
	Texture->Filter = GS_FILTER_NEAREST;
	Texture->PSM = GS_PSM_CT24;
	Texture->VramClut = 0;
	Texture->Clut = NULL;

	uint32_t TextureSize = gsKit_texture_size_ee(Texture->Width, Texture->Height, Texture->PSM);
	Texture->Mem = memalign(128, TextureSize);

	memcpy(Texture->Mem, imageData, imageSize);

	return gsKit_texture_finish(gsGlobal, Texture);
}


static bool loadModules() {
	int ret = SifLoadModule("rom0:SIO2MAN", 0, nullptr);
	if (0 > ret) {
		fprintf(stderr, "sifLoadModule sio failed: %d\n", ret);
		return false;
	}

	ret = SifLoadModule("rom0:PADMAN", 0, nullptr);
	if (0 > ret) {
		fprintf(stderr, "sifLoadModule pad failed: %d\n", ret);
		return false;
	}

	return true;
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

	SFT sft = {
		.xScale = 20,
		.yScale = 20,
		.flags  = SFT_DOWNWARD_Y,
	};
	sft.font = sft_loadfile("AtkinsonHyperlegibleMono-VariableFont_wght.ttf");
	if (nullptr == sft.font) {
		fputs("Error in sft_loadfile\n", stderr);
		goto cleanup_backend;
	}

	SFT_LMetrics lmtx;
	if (0 > sft_lmetrics(&sft, &lmtx)) {
		fputs("Error in sft_lmetrics\n", stderr);
		goto cleanup_font;
	}

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

	int port = 0; // 0 -> Connector 1, 1 -> Connector 2
	int slot = 0; // Always zero if not using multitap
	size_t selectedIdx = 0;
	size_t selectedId = 0;
	struct padButtonStatus buttons;
	uint32_t oldPadState = 0;
	if (!loadModules()) {
		fputs("loadModules failed!\n", stderr);
		goto cleanup_font;
	}
	padInit(0);

	int ret = padPortOpen(port, slot, padBuf);
	if(0 == ret) {
		fprintf(stderr, "padOpenPort failed: %d\n", ret);
		goto cleanup_font;
	}

	if(!initializePad(port, slot)) {
		fputs("pad initalization failed!\n", stderr);
		goto cleanup_font;
	}

	const size_t bmpSize = 3 * ScreenWidth * ScreenHeight;
	unsigned char *const bmp = malloc(bmpSize);
	if (nullptr == bmp) {
		fputs("malloc failed!\n", stderr);
		goto cleanup_font;
	}
	GSTEXTURE Bitmap = { 0 };

	result = EXIT_SUCCESS;

	bool updateText = true;
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
						updateText = true;
						break;
					}
				}
			} else if (PAD_DOWN & newPadState) {
				for (size_t i = selectedIdx + 1; i < game.screen->actionCount; ++i) {
					const struct Action *action = game.screen->actions[i];
					if (action->visibility_checker(&game, action)) {
						selectedIdx = i;
						++selectedId;
						updateText = true;
						break;
					}
				}
			} else if (PAD_CROSS & newPadState) {
				if (!backend_input(&game, selectedId)) {
					fputs("Error in backend_input\n", stderr);
					result = EXIT_FAILURE;
					break;
				}

				selectedId = 0;
				for (size_t i = 0; i < game.screen->actionCount; ++i) {
					const struct Action *action = game.screen->actions[i];
					if (action->visibility_checker(&game, action)) {
						selectedIdx = i;
						break;
					}
				}
				updateText = true;
			}
		}

		if (updateText) {
			updateText = false;

			memset(bmp, 0, bmpSize);

			const char *const body = game.screen->body_generator(&game, game.screen);
			if (nullptr == body) {
				fputs("Error in body_generator\n", stderr);
				result = EXIT_FAILURE;
				break;
			}

			uint16_t textY = 0;
			if (!schriftDrawText(&sft, bmp, body, 10, textY, White, nullptr, &textY)) {
				fputs("Error in schriftDrawText\n", stderr);
				result = EXIT_FAILURE;
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
					fputs("Error in schriftDrawText\n", stderr);
					result = EXIT_FAILURE;
					game.quit = true;
					break;
				}

				if (!schriftDrawText(&sft, bmp, action->title, textX, textY, White, nullptr, &textY)) {
					fputs("Error in schriftDrawText\n", stderr);
					result = EXIT_FAILURE;
					game.quit = true;
					break;
				}

				++id;
			}

			if (game.quit) {
				break;
			}

			gsKit_vram_clear(gsGlobal);
			gsKit_texture_framebuffer(gsGlobal, &Bitmap, bmp, bmpSize);

			// TODO: Check if this is needed
			gsKit_clear(gsGlobal, Black);

			gsKit_prim_sprite_texture(gsGlobal,	&Bitmap, 0, 0, 0, 0,
																Bitmap.Width, Bitmap.Height,
																Bitmap.Width, Bitmap.Height,
																2, White);
		}

		gsKit_queue_exec(gsGlobal);
		gsKit_sync_flip(gsGlobal);
	}

	// free(bmp);

cleanup_font:
	// sft_freefont(sft.font);

cleanup_backend:
	// backend_cleanup(&game);

	printf("Exited with %i\n", result);

	// Loop and allow `ps2client reset` to re-enter bootloader
	SleepThread();
	while (true) {
	}

	return result;
}
