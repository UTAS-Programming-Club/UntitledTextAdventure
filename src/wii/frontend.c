#include <inttypes.h>  // for uint8_t, SCNu8
#include <stdio.h>     // for fputs, stderr, putchar, NULL, printf, puts, scanf, size_t
#include <stdlib.h>    // for EXIT_FAILURE, EXIT_SUCCESS

#include "backend.h"   // for GameInfo, Screen, backend_cleanup, Action, backend_input, backend_register_extension, backend_setup
#include "gen/ext1.h"  // for Ext1_RoomCount, Ext1_Rooms

#include <string.h>    // for memset
#include <malloc.h>    // for memalign
#include <gccore.h>
#include <wiiuse/wpad.h>

#define FIFO_SIZE (256*1024)

static GXRModeObj *vmode;
static void *xfb = NULL;

static struct Text {
  int x;
  int y;
  int size;
  u32 colour;
} text;

sys_fontheader *fontdata;
GXTexObj fonttex;

// Sets video mode + basic pipeline
void setup_video(void);
// Sets various bindings for graphics
void setup_gx(void);
// Sets up rom font
void setup_font(void);
// Sets framebuffer -> viewport info
void setup_viewport(void);

// text processing functions
void draw_font_cell(int16_t x1, int16_t y1, 
					uint32_t c, int16_t s1, int16_t t1) {
  int16_t x2 = x1 + fontdata->cell_width * text.size  / fontdata->cell_height;
  int16_t y2 = y1 - text.size;

  int16_t s2 = s1 + fontdata->cell_width;
  int16_t t2 = t1 + fontdata->cell_height;

  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

  GX_Position2s16(x1, y2);
  GX_Color1u32(c);
  GX_TexCoord2s16(s1, t1);

  GX_Position2s16(x2, y2);
  GX_Color1u32(c);
  GX_TexCoord2s16(s2, t1);

  GX_Position2s16(x2, y1);
  GX_Color1u32(c);
  GX_TexCoord2s16(s2, t2);

  GX_Position2s16(x1, y1);
  GX_Color1u32(c);
  GX_TexCoord2s16(s1, t2);

  GX_End();
}

int process_string(const char *s) {
  void *image;
  int32_t xpos, ypos, width, x, fx; // inconsistency!
  
  x = text.x;
  fx = x;
  for (; *s != '\0'; s++) {
    char c = *s;
	if (c == '\n') {
	  text.y += text.size + 1;
	  x = fx;
	}
	if (c < fontdata->first_char)
	  continue;
	SYS_GetFontTexture(c, &image, &xpos, &ypos, &width);
	draw_font_cell(x, text.y, text.colour, xpos, ypos);
	x += width * text.size / fontdata->cell_height;
  }

  return x - text.x;
}

int main(void) {
  int result = EXIT_SUCCESS;
  struct GameInfo game;
  if (!backend_setup(&game)) {
	// wii may have a tty for debug
    // fputs("Error in backend_setup\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  if (!backend_register_extension(&game, Ext1_RoomCount, Ext1_Rooms)) {
    // fputs("Error in backend_register_extension\n", stderr);
    backend_cleanup(&game);
    return EXIT_FAILURE;
  }

  // setup WII
  VIDEO_Init();
  WPAD_Init();

  setup_video();
  setup_gx();
  setup_font();
  setup_viewport();  

  int option = 0;

  while (!game.quit && SYS_MainLoop()) {
	WPAD_ScanPads();
	
	const char *body = game.screen->body_generator(&game, game.screen);
    if (NULL == body) {
      // fputs("Error in body_generator\n", stderr);
      result = EXIT_FAILURE;
      break;
    }
	
	u32 pressed = WPAD_ButtonsDown(0);
	if (pressed & WPAD_BUTTON_DOWN)
	   ++option;
	if (pressed & WPAD_BUTTON_UP)
	  --option;

	text.x = 10;
	text.y = 24;
	text.colour = 0xFFFFFFFF;
	text.size = 24;
	process_string(body);
	
	text.y += 50;
	text.size = 20;
    uint8_t id = 0;
    for (size_t i = 0; i < game.screen->actionCount; ++i) {
      const struct Action *action = game.screen->actions[i];
      if (!action->visibility_checker(&game, action)) {
        continue;
      }
	  
	  text.x = 10;
	  if (option == id) {
        text.x += process_string("> ");
      }
	  process_string(action->title);
      text.y += 21;

	  ++id;
    }
	
	if (option < 0)
	  option = id;
	if (option >= (int)id)
	  option = 0;

	if (pressed & WPAD_BUTTON_A) {
		if (!backend_input(&game, option)) {
		  //fputs("Error in backend_input", stderr);
		  result = EXIT_FAILURE;
		  break;
		}
	}
	
	GX_DrawDone();
    GX_CopyDisp(xfb, GX_TRUE);
    GX_Flush();
	
	VIDEO_WaitVSync();
  }

  backend_cleanup(&game);

  return result;
}

void setup_video(void) {
  // video mode
  vmode = VIDEO_GetPreferredMode(NULL);
  // framebuffer
  xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));

  VIDEO_Configure(vmode);
  VIDEO_SetNextFramebuffer(xfb);
  VIDEO_SetBlack(false); // really?
  VIDEO_Flush();

  VIDEO_WaitVSync();
  if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}

void setup_viewport() {
  Mtx44 proj;
  u32 w, h;

  w = vmode->fbWidth;
  h = vmode->efbHeight;

  guOrtho(proj, 0, h, 0, w, 0, 1);
  GX_LoadProjectionMtx(proj, GX_ORTHOGRAPHIC);

  GX_SetViewport(0, 0, w, h, 0, 1);

  f32 yscale = GX_GetYScaleFactor(h, vmode->xfbHeight);
  GX_SetDispCopyYScale(yscale);
  GX_SetScissor(0, 0, w, h);
  GX_SetDispCopySrc(0, 0, w, h);
  GX_SetDispCopyDst(w, vmode->xfbHeight);
  // should check what these even are
  GX_SetCopyFilter(vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
}

void activate_font_texture() {
  u32 texture_size;
  void *texels;

  texels = (void *)fontdata + fontdata->sheet_image;
  GX_InitTexObj(&fonttex, texels,
				 fontdata->sheet_width, fontdata->sheet_height,
				 fontdata->sheet_format, GX_CLAMP, GX_CLAMP, GX_FALSE);
  GX_InitTexObjLOD(&fonttex, GX_LINEAR, GX_LINEAR, 0., 0., 0.,
				    GX_TRUE, GX_TRUE, GX_ANISO_1);
  GX_LoadTexObj(&fonttex, GX_TEXMAP0);

  texture_size = GX_GetTexBufferSize(fontdata->sheet_width,
								     fontdata->sheet_height,
									 fontdata->sheet_format,
									 GX_FALSE, 0);
  DCStoreRange(texels, texture_size);
  GX_InvalidateTexAll();
}

void setup_font(void) {
  if (SYS_GetFontEncoding() == 0)
    fontdata = memalign(32, SYS_FONTSIZE_ANSI);
  else
	fontdata = memalign(32, SYS_FONTSIZE_SJIS);
  
  SYS_InitFont(fontdata);
  activate_font_texture();

  text.size = fontdata->cell_height;
}

void setup_gx(void) {
  GXColor backgroundColour = {0, 0, 0, 255};
  void *fifoBuffer = NULL;
  Mtx mv;

  // setup buffer for usage
  fifoBuffer = MEM_K0_TO_K1(memalign(32, FIFO_SIZE));
  memset(fifoBuffer, 0, FIFO_SIZE);
  GX_Init(fifoBuffer, FIFO_SIZE);

  // vertex bindings
  GX_ClearVtxDesc();
  GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);  // pos
  GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT); // colour
  GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT); // texture
  GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16 , 0);
  GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8 , 0);
  GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16 , 0);

  // 1 unit <=> 1 texel ?
  GX_SetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 1, 1);

  GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

  GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GX_SetCopyClear(backgroundColour, GX_MAX_Z24);
  
  // more texture setup?
  GX_SetNumChans(1);
  GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, 
				 GX_SRC_VTX, GX_SRC_VTX, 0,
				 GX_DF_NONE, GX_AF_NONE);
  GX_SetNumTexGens(1);
  GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);

  // setup initial translation
  guMtxIdentity(mv);
  guMtxTransApply(mv, mv, 0.4, 0.4, 0);
  GX_LoadPosMtxImm(mv, GX_PNMTX0);  
}
