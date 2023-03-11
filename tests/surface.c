/*	Public domain	*/

/*
 * Tests for AG_Surface(3).
 */

#include "agartest.h"

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

#include "config/have_rand48.h"

#include <string.h>
#include <stdlib.h>

typedef struct {
	AG_TestInstance _inherit;
	AG_Surface *_Nullable Schampden;
	AG_Surface *_Nullable Sparrot;
	AG_Surface *_Nullable S[24];
	int nSurfaces;
	AG_Color randColorA;
	AG_Color randColorNoA;
} MyTestInstance;

static void
RandomizeColors(MyTestInstance *ti)
{
#ifdef HAVE_RAND48
	ti->randColorNoA.r = ti->randColorA.r = (AG_Component)lrand48();
	ti->randColorNoA.g = ti->randColorA.g = (AG_Component)lrand48();
	ti->randColorNoA.b = ti->randColorA.b = (AG_Component)lrand48();
 	                     ti->randColorA.a = (AG_Component)lrand48();
#else
	ti->randColorNoA.r = ti->randColorA.r = AG_OPAQUE;
	ti->randColorNoA.g = ti->randColorA.g = AG_OPAQUE;
	ti->randColorNoA.b = ti->randColorA.b = 0;
 	                     ti->randColorA.a = AG_OPAQUE;
#endif
	ti->randColorNoA.a = AG_OPAQUE;
}

static void
ClearSurfaces(MyTestInstance *ti)
{
	AG_Color cWhite;
	int i;

	AG_ColorWhite(&cWhite);

	for (i = 0; i < ti->nSurfaces; i++) {
		if (i == 0) {
			AG_FillRect(ti->S[i], NULL, &cWhite);
		} else {
			AG_FillRect(ti->S[i], NULL, &ti->randColorNoA);
		}
	}
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;
	const int w = 128;
	const int h = 128;

	if ((ti->Schampden = AG_SurfaceFromPNG("champden.png")) == NULL ||
	    (ti->Sparrot = AG_SurfaceFromPNG("parrot.png")) == NULL)
		TestMsgS(ti, AG_GetError());

	ti->S[0]  = AG_SurfaceIndexed(w,h, 1, 0);       /* 1-bit Monochrome */
	ti->S[1]  = AG_SurfaceIndexed(w,h, 2, 0);          /* 2-bit Indexed */
	ti->S[2]  = AG_SurfaceIndexed(w,h, 4, 0);          /* 4-bit Indexed */
	ti->S[3]  = AG_SurfaceIndexed(w,h, 8, 0);          /* 8-bit Indexed */

	ti->S[4]  = AG_SurfaceGrayscale(w,h, 16, 0);    /* 16-bit Grayscale */
	ti->S[5]  = AG_SurfaceGrayscale(w,h, 32, 0);    /* 32-bit Grayscale */

	ti->S[6]  = AG_SurfaceRGB(w,h, 16, 0,                 /* 16-bit RGB */
	    0xf000, 0x0f00, 0x00f0);
	ti->S[7]  = AG_SurfaceRGB(w,h, 16, 0,                 /* 16-bit BGR */
	    0x00f0, 0x0f00, 0xf000);
	ti->S[8]  = AG_SurfaceRGBA(w,h, 16, 0,               /* 16-bit RGBA */
	    0xf000, 0x0f00, 0x00f0, 0x000f);
	ti->S[9] = AG_SurfaceRGBA(w,h, 16, 0,                /* 16-bit BGRA */
	    0x00f0, 0x0f00, 0xf000, 0x000f);
	ti->S[10] = AG_SurfaceRGBA(w,h, 16, 0,               /* 16-bit ABGR */
	    0x000f, 0x00f0, 0x0f00, 0xf000);
	ti->S[11] = AG_SurfaceRGB(w,h, 24, 0,                 /* 24-bit RGB */
	    0xff0000, 0x00ff00, 0x0000ff);
	ti->S[12] = AG_SurfaceRGB(w,h, 24, 0,                 /* 24-bit BGR */
	    0x0000ff, 0x00ff00, 0xff0000);
	ti->S[13] = AG_SurfaceRGBA(w,h, 32, 0,               /* 32-bit RGBA */
	    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	ti->S[14] = AG_SurfaceRGBA(w,h, 32, 0,               /* 32-bit ARGB */
	    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	ti->S[15] = AG_SurfaceRGBA(w,h, 32, 0,               /* 32-bit ABGR */
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

#if AG_MODEL == AG_LARGE
	ti->S[16] = AG_SurfaceGrayscale(w,h, 64, 0);    /* 64-bit Grayscale */
	ti->S[17] = AG_SurfaceRGB(w,h, 40, 0,                 /* 40-bit RGB */
	    0x000003ff,
	    0x000ffc00,
	    0x3ff00000);
	ti->S[18] = AG_SurfaceRGB(w,h, 40, 0,                 /* 40-bit BGR */
	    0x3ff00000,
	    0x000ffc00,
	    0x000003ff);
	ti->S[19] = AG_SurfaceRGB(w,h, 48, 0,                 /* 48-bit RGB */
	    0xffff00000000,
	    0x0000ffff0000,
	    0x00000000ffff);
	ti->S[20] = AG_SurfaceRGB(w,h, 48, 0,                 /* 48-bit BGR */
	    0x00000000ffff,
	    0x0000ffff0000,
	    0xffff00000000);
	ti->S[21] = AG_SurfaceRGBA(w,h, 64, 0,               /* 64-bit RGBA */
	    0xffff000000000000,
	    0x0000ffff00000000,
	    0x00000000ffff0000,
	    0x000000000000ffff);
	ti->S[22] = AG_SurfaceRGBA(w,h, 64, 0,               /* 64-bit ABGR */
	    0x000000000000ffff,
	    0x00000000ffff0000,
	    0x0000ffff00000000,
	    0xffff000000000000);
	ti->S[23] = AG_SurfaceRGBA(w,h, 64, 0,               /* 64-bit BGRA */
	    0x00000000ffff0000,
	    0x0000ffff00000000,
	    0xffff000000000000,
	    0x000000000000ffff);
	ti->nSurfaces = 24;
#else
	ti->nSurfaces = 16;
#endif

	RandomizeColors(ti);
	ClearSurfaces(ti);
	return (0);
}

static void
Destroy(void *obj)
{
	MyTestInstance *ti = obj;
	int i;

	if (ti->Schampden != NULL)
		AG_SurfaceFree(ti->Schampden);
	if (ti->Sparrot != NULL)
		AG_SurfaceFree(ti->Sparrot);

	for (i = 0; i < ti->nSurfaces; i++)
		AG_SurfaceFree(ti->S[i]);
}

static void
RandomizeSurfaces(MyTestInstance *ti)
{
	int i;

	for (i = 0; i < ti->nSurfaces; i++) {
		AG_Surface *S = ti->S[i];
		int x, y;

		if (S->format.mode == AG_SURFACE_INDEXED) {
			AG_Pixel pxMax;
		
			pxMax = AG_PixelFormatMaximum(&S->format);

			if (S->format.BitsPerPixel <= 8) {
				for (y = 0; y < S->h; y++) {
					for (x = 0; x < S->w; x++) {
						Uint8 px;
#ifdef HAVE_RAND48
						px = (Uint8)lrand48();
#else
						px = (Uint8)(x * y);
#endif
						px = px % (pxMax+1);
						AG_SurfacePut8(S, x,y, px);
					}
				}
			}
		} else {
			Uint8 *p = S->pixels;

			for (y = 0; y < S->h; y++) {
				for (x = 0; x < S->w; x++) {
					AG_Color c;
#ifdef HAVE_RAND48
					c.r = (AG_Component)lrand48();
					c.g = (AG_Component)lrand48();
					c.b = (AG_Component)lrand48();
#else
					c.r = c.g = c.b = (x*y);
#endif
					c.a = AG_OPAQUE;

					AG_SurfacePut_At(S, p,
					    AG_MapPixel(&S->format, &c));

					p += S->format.BytesPerPixel;
				}
				p += S->padding;
			}
		}
	}
}

static int
Test(void *obj)
{
	MyTestInstance *ti = obj;
	int i;

	/*
	 * Show blitter information.
	 */
	for (i = 0; i < AG_SURFACE_MODE_LAST; i++) {
		const AG_LowerBlit *blits = agLowerBlits[i];
		const int count = agLowerBlits_Count[i];
		int j;
		
		TestMsg(ti, "");
		TestMsg(ti, AGSI_BOLD "%s" AGSI_RST " mode blitters:",
		    agSurfaceModeNames[i]);

		for (j = 0; j < count; j++) {
			const AG_LowerBlit *b = &blits[j];

			TestMsg(ti, "Blit Fn %p depth "
			      AGSI_BOLD "%d" AGSI_RST "->"
			      AGSI_BOLD "%d" AGSI_RST ", "
			    "caps 0x%x, cpu 0x%x",
			    b->fn, b->depthSrc, b->depthDst,
			    b->caps, b->cpuExts);
			if (i == AG_SURFACE_PACKED) {
#if AG_MODEL == AG_LARGE
				TestMsg(ti, " ("
				    AGSI_RED  "0x%016lx" AGSI_RST " -> "
				    AGSI_RED  "0x%016lx" AGSI_RST ", "
				    AGSI_GRN " 0x%016lx" AGSI_RST " -> "
				    AGSI_GRN  "0x%016lx" AGSI_RST ",",
				    b->Rsrc, b->Rdst, b->Gsrc, b->Gdst);
				TestMsg(ti, " "
				    AGSI_BLU " 0x%016lx" AGSI_RST " -> "
				    AGSI_BLU  "0x%016lx" AGSI_RST ", "
				    AGSI_WHT " 0x%016lx" AGSI_RST " -> "
				    AGSI_WHT  "0x%016lx" AGSI_RST ")",
				    b->Bsrc, b->Bdst, b->Asrc, b->Adst);
#else
				TestMsg(ti, " ("
				    AGSI_RED "0x%08lx" AGSI_RST " -> "
				    AGSI_RED "0x%08lx" AGSI_RST ", "
				    AGSI_GRN "0x%08lx" AGSI_RST " -> "
				    AGSI_GRN "0x%08lx" AGSI_RST ", "
				    AGSI_BLU "0x%08lx" AGSI_RST " -> "
				    AGSI_BLU "0x%08lx" AGSI_RST ", "
				    AGSI_WHT "0x%08lx" AGSI_RST " -> "
				    AGSI_WHT "0x%08lx" AGSI_RST ")",
				    b->Rsrc, b->Rdst, b->Gsrc, b->Gdst,
				    b->Bsrc, b->Bdst, b->Asrc, b->Adst);
#endif
			}
		}
	}

	RandomizeSurfaces(ti);
	return (0);
}

static void
Clear(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Button *btnRefresh = AG_BUTTON_PTR(2);

	RandomizeColors(ti);
	ClearSurfaces(ti);
	AG_PostEvent(btnRefresh, "button-pushed", "%i", 1);
}


static void
Randomize(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Button *btnRefresh = AG_BUTTON_PTR(2);

	RandomizeColors(ti);
	RandomizeSurfaces(ti);
	AG_PostEvent(btnRefresh, "button-pushed", "%i", 1);
}

static void
Champden(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Button *btnRefresh = AG_BUTTON_PTR(2);
	const AG_Surface *Schampden = ti->Schampden;
	int i;

	for (i = 0; i < ti->nSurfaces; i++) {
		AG_Surface *S = ti->S[i];
		int xWiggle, yWiggle;

		xWiggle = S->w - Schampden->w;
		yWiggle = S->h - Schampden->h;
#ifdef HAVE_RAND48
		if (xWiggle > 0) { xWiggle = (int)lrand48() % xWiggle; }
		if (yWiggle > 0) { yWiggle = (int)lrand48() % yWiggle; }
#endif
		AG_SurfaceBlit(Schampden, NULL, S, xWiggle, yWiggle);
	}

	AG_PostEvent(btnRefresh, "button-pushed", "%i", 1);
}

static void
Parrot(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Button *btnRefresh = AG_BUTTON_PTR(2);
	const AG_Surface *Sparrot = ti->Sparrot;
	int i;

	for (i = 0; i < ti->nSurfaces; i++) {
		AG_Surface *S = ti->S[i];
		int xWiggle, yWiggle;

		xWiggle = S->w - Sparrot->w;
		yWiggle = S->h - Sparrot->h;
#ifdef HAVE_RAND48
		if (xWiggle > 0) { xWiggle = (int)lrand48() % xWiggle; }
		if (yWiggle > 0) { yWiggle = (int)lrand48() % yWiggle; }
#endif
		AG_SurfaceBlit(Sparrot, NULL, S, xWiggle, yWiggle);
	}

	AG_PostEvent(btnRefresh, "button-pushed", "%i", 1);
}

static void
Refresh(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Scrollview *sv = AG_SCROLLVIEW_PTR(2);
	int i;

	AG_ObjectFreeChildrenOfType(sv, "AG_Widget:AG_Box:*");

	for (i = 0; i < ti->nSurfaces; ) {
		AG_Box *hBox;
		int x;

		hBox = AG_BoxNewHoriz(sv, 0);
		for (x = 0; x < 4; x++) {
			const AG_Surface *S = ti->S[i];

			AG_LabelNew(hBox, 0, " %02d) ", i);
			AG_PixmapFromSurface(hBox, 0, S);
			if (++i >= ti->nSurfaces)
				break;
		}
	}

	AG_WindowUpdate(AG_ParentWindow(sv));
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_Label *lbl;
	AG_Scrollview *sv;
	AG_Box *btnBox;

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL,
	    _(AGSI_IDEOGRAM AGSI_SAVE_IMAGE AGSI_RST
	     " Test for AG_Surface "
	      AGSI_IDEOGRAM AGSI_LOAD_IMAGE));
	AG_SetFontFamily(lbl, "league-spartan");
	AG_SetFontSize(lbl, "160%");

	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND);

	btnBox = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_Button *btnRef;

		btnRef = AG_ButtonNewFn(btnBox, AG_BUTTON_VFILL, _("Refresh"),
		    Refresh, "%p,%p", ti, sv);

		AG_ButtonNewFn(btnBox, AG_BUTTON_VFILL, _("Randomize"),
		    Randomize, "%p,%p", ti, btnRef);
		AG_ButtonNewFn(btnBox, AG_BUTTON_VFILL, _("Champden"),
		    Champden, "%p,%p", ti, btnRef);
		AG_ButtonNewFn(btnBox, AG_BUTTON_VFILL, _("Parrot"),
		    Parrot, "%p,%p", ti, btnRef);
		AG_ButtonNewFn(btnBox, AG_BUTTON_VFILL, _("Clear"),
		    Clear, "%p,%p", ti, btnRef);

		AG_PostEvent(btnRef, "button-pushed", "%i", 1);
	}

	return (0);
}

/*
 * Rectangle Fills.
 */
static void
Bench_FillRect_A(void *obj, int arg)
{
	MyTestInstance *ti = obj;
	AG_FillRect(ti->S[arg], NULL, &ti->randColorA);
}
static void
Bench_FillRect_NoA(void *obj, int arg)
{
	MyTestInstance *ti = obj;
	AG_FillRect(ti->S[arg], NULL, &ti->randColorNoA);
}
static struct ag_benchmark_fn fillRectOpsFns[] = {
	{ "AG_FillRect(1-bit Indexed  <- a)",      Bench_FillRect_A,    0 },
	{ "AG_FillRect(1-bit Indexed <- !a)",      Bench_FillRect_NoA,  0 },
	{ "AG_FillRect(2-bit Indexed  <- a)",      Bench_FillRect_A,    1 },
	{ "AG_FillRect(2-bit Indexed <- !a)",      Bench_FillRect_NoA,  1 },
	{ "AG_FillRect(4-bit Indexed  <- a)",      Bench_FillRect_A,    2 },
	{ "AG_FillRect(4-bit Indexed <- !a)",      Bench_FillRect_NoA,  2 },
	{ "AG_FillRect(8-bit Indexed  <- a)",      Bench_FillRect_A,    3 },
	{ "AG_FillRect(8-bit Indexed <- !a)",      Bench_FillRect_NoA,  3 },
	{ "AG_FillRect(16-bit Grayscale  <- a)",   Bench_FillRect_A,    4 },
	{ "AG_FillRect(16-bit Grayscale <- !a)",   Bench_FillRect_NoA,  4 },
	{ "AG_FillRect(32-bit Grayscale  <- a)",   Bench_FillRect_A,    5 },
	{ "AG_FillRect(32-bit Grayscale <- !a)",   Bench_FillRect_NoA,  5 },
	{ "AG_FillRect(16-bit RGB  <- a)",         Bench_FillRect_A,    6 },
	{ "AG_FillRect(16-bit RGB <- !a)",         Bench_FillRect_NoA,  6 },
	{ "AG_FillRect(16-bit BGR  <- a)",         Bench_FillRect_A,    7 },
	{ "AG_FillRect(16-bit BGR <- !a)",         Bench_FillRect_NoA,  7 },
	{ "AG_FillRect(16-bit RGBA  <- a)",        Bench_FillRect_A,    8 },
	{ "AG_FillRect(16-bit RGBA <- !a)",        Bench_FillRect_NoA,  8 },
	{ "AG_FillRect(16-bit BGRA  <- a)",        Bench_FillRect_A,    9 },
	{ "AG_FillRect(16-bit BGRA <- !a)",        Bench_FillRect_NoA,  9 },
	{ "AG_FillRect(16-bit ABGR  <- a)",        Bench_FillRect_A,   10 },
	{ "AG_FillRect(16-bit ABGR <- !a)",        Bench_FillRect_NoA, 10 },
	{ "AG_FillRect(24-bit RGB  <- a)",         Bench_FillRect_A,   11 },
	{ "AG_FillRect(24-bit RGB <- !a)",         Bench_FillRect_NoA, 11 },
	{ "AG_FillRect(24-bit BGR  <- a)",         Bench_FillRect_A,   12 },
	{ "AG_FillRect(24-bit BGR <- !a)",         Bench_FillRect_NoA, 12 },
	{ "AG_FillRect(32-bit RGBA  <- a)",        Bench_FillRect_A,   13 },
	{ "AG_FillRect(32-bit RGBA <- !a)",        Bench_FillRect_NoA, 13 },
	{ "AG_FillRect(32-bit ARGB  <- a)",        Bench_FillRect_A,   14 },
	{ "AG_FillRect(32-bit ARGB <- !a)",        Bench_FillRect_NoA, 14 },
	{ "AG_FillRect(32-bit ABGR  <- a)",        Bench_FillRect_A,   15 },
	{ "AG_FillRect(32-bit ABGR <- !a)",        Bench_FillRect_NoA, 15 },
#if AG_MODEL == AG_LARGE
	{ "AG_FillRect(64-bit Grayscale  <- a)",   Bench_FillRect_A,   16 },
	{ "AG_FillRect(64-bit Grayscale <- !a)",   Bench_FillRect_NoA, 16 },
	{ "AG_FillRect(48-bit RGB  <- a)",         Bench_FillRect_A,   17 },
	{ "AG_FillRect(48-bit RGB <- !a)",         Bench_FillRect_NoA, 17 },
	{ "AG_FillRect(48-bit BGR  <- a)",         Bench_FillRect_A,   18 },
	{ "AG_FillRect(48-bit BGR <- !a)",         Bench_FillRect_NoA, 18 },
	{ "AG_FillRect(64-bit RGBA  <- a)",        Bench_FillRect_A,   19 },
	{ "AG_FillRect(64-bit RGBA <- !a)",        Bench_FillRect_NoA, 19 },
	{ "AG_FillRect(64-bit ABGR  <- a)",        Bench_FillRect_A,   20 },
	{ "AG_FillRect(64-bit ABGR <- !a)",        Bench_FillRect_NoA, 20 },
	{ "AG_FillRect(64-bit BGRA  <- a)",        Bench_FillRect_A,   21 },
	{ "AG_FillRect(64-bit BGRA <- !a)",        Bench_FillRect_NoA, 21 },
#endif
};
struct ag_benchmark fillRectOps = {
	"AG_FillRect(3)",
	&fillRectOpsFns[0],
	sizeof(fillRectOpsFns) / sizeof(fillRectOpsFns[0]),
	10, 1000, 1000000
};

/*
 * Blitters.
 */
static void
Bench_Blit_Inside(void *obj, int arg)
{
	MyTestInstance *ti = obj;
	AG_SurfaceBlit(ti->Schampden, NULL, ti->S[arg], 0,0);
}
static void
Bench_Blit_T_Clip(void *obj, int arg)
{
	MyTestInstance *ti = obj;
	AG_SurfaceBlit(ti->Schampden, NULL, ti->S[arg], 0,-24);
}
static void
Bench_Blit_R_Clip(void *obj, int arg)
{
	MyTestInstance *ti = obj;
	AG_SurfaceBlit(ti->Schampden, NULL, ti->S[arg], 40,0);
}
static void
Bench_Blit_B_Clip(void *obj, int arg)
{
	MyTestInstance *ti = obj;
	AG_SurfaceBlit(ti->Schampden, NULL, ti->S[arg], 0,40);
}
static void
Bench_Blit_L_Clip(void *obj, int arg)
{
	MyTestInstance *ti = obj;
	AG_SurfaceBlit(ti->Schampden, NULL, ti->S[arg], -24,0);
}
static struct ag_benchmark_fn blitOpsFns[] = {
	{ "AG_SurfaceBlit(1-bit Indexed <- Inside)", Bench_Blit_Inside, 0 },
	{ "AG_SurfaceBlit(1-bit Indexed <- T-Clip)", Bench_Blit_T_Clip, 0 },
	{ "AG_SurfaceBlit(1-bit Indexed <- R-Clip)", Bench_Blit_R_Clip, 0 },
	{ "AG_SurfaceBlit(1-bit Indexed <- B-Clip)", Bench_Blit_B_Clip, 0 },
	{ "AG_SurfaceBlit(1-bit Indexed <- L-Clip)", Bench_Blit_L_Clip, 0 },
	{ "AG_SurfaceBlit(2-bit Indexed <- Inside)", Bench_Blit_Inside, 1 },
	{ "AG_SurfaceBlit(2-bit Indexed <- T-Clip)", Bench_Blit_T_Clip, 1 },
	{ "AG_SurfaceBlit(2-bit Indexed <- R-Clip)", Bench_Blit_R_Clip, 1 },
	{ "AG_SurfaceBlit(2-bit Indexed <- B-Clip)", Bench_Blit_B_Clip, 1 },
	{ "AG_SurfaceBlit(2-bit Indexed <- L-Clip)", Bench_Blit_L_Clip, 1 },
	{ "AG_SurfaceBlit(4-bit Indexed <- Inside)", Bench_Blit_Inside, 2 },
	{ "AG_SurfaceBlit(4-bit Indexed <- T-Clip)", Bench_Blit_T_Clip, 2 },
	{ "AG_SurfaceBlit(4-bit Indexed <- R-Clip)", Bench_Blit_R_Clip, 2 },
	{ "AG_SurfaceBlit(4-bit Indexed <- B-Clip)", Bench_Blit_B_Clip, 2 },
	{ "AG_SurfaceBlit(4-bit Indexed <- L-Clip)", Bench_Blit_L_Clip, 2 },
#if 0
	{ "AG_SurfaceBlit(8-bit Indexed <- Inside)", Bench_Blit_Inside, 3 },
	{ "AG_SurfaceBlit(8-bit Indexed <- T-Clip)", Bench_Blit_T_Clip, 3 },
	{ "AG_SurfaceBlit(8-bit Indexed <- R-Clip)", Bench_Blit_R_Clip, 3 },
	{ "AG_SurfaceBlit(8-bit Indexed <- B-Clip)", Bench_Blit_B_Clip, 3 },
	{ "AG_SurfaceBlit(8-bit Indexed <- L-Clip)", Bench_Blit_L_Clip, 3 },
	{ "AG_SurfaceBlit(16-bit Grayscale <- Inside)", Bench_Blit_Inside, 4 },
	{ "AG_SurfaceBlit(16-bit Grayscale <- T-Clip)", Bench_Blit_T_Clip, 4 },
	{ "AG_SurfaceBlit(16-bit Grayscale <- R-Clip)", Bench_Blit_R_Clip, 4 },
	{ "AG_SurfaceBlit(16-bit Grayscale <- B-Clip)", Bench_Blit_B_Clip, 4 },
	{ "AG_SurfaceBlit(16-bit Grayscale <- L-Clip)", Bench_Blit_L_Clip, 4 },
	{ "AG_SurfaceBlit(32-bit Grayscale <- Inside)", Bench_Blit_Inside, 5 },
	{ "AG_SurfaceBlit(32-bit Grayscale <- T-Clip)", Bench_Blit_T_Clip, 5 },
	{ "AG_SurfaceBlit(32-bit Grayscale <- R-Clip)", Bench_Blit_R_Clip, 5 },
	{ "AG_SurfaceBlit(32-bit Grayscale <- B-Clip)", Bench_Blit_B_Clip, 5 },
	{ "AG_SurfaceBlit(32-bit Grayscale <- L-Clip)", Bench_Blit_L_Clip, 5 },
	{ "AG_SurfaceBlit(16-bit RGB <- Inside)", Bench_Blit_Inside, 6 },
	{ "AG_SurfaceBlit(16-bit RGB <- T-Clip)", Bench_Blit_T_Clip, 6 },
	{ "AG_SurfaceBlit(16-bit RGB <- R-Clip)", Bench_Blit_R_Clip, 6 },
	{ "AG_SurfaceBlit(16-bit RGB <- B-Clip)", Bench_Blit_B_Clip, 6 },
	{ "AG_SurfaceBlit(16-bit RGB <- L-Clip)", Bench_Blit_L_Clip, 6 },
	{ "AG_SurfaceBlit(16-bit BGR <- Inside)", Bench_Blit_Inside, 7 },
	{ "AG_SurfaceBlit(16-bit BGR <- T-Clip)", Bench_Blit_T_Clip, 7 },
	{ "AG_SurfaceBlit(16-bit BGR <- R-Clip)", Bench_Blit_R_Clip, 7 },
	{ "AG_SurfaceBlit(16-bit BGR <- B-Clip)", Bench_Blit_B_Clip, 7 },
	{ "AG_SurfaceBlit(16-bit BGR <- L-Clip)", Bench_Blit_L_Clip, 7 },
	{ "AG_SurfaceBlit(16-bit RGBA <- Inside)", Bench_Blit_Inside, 8 },
	{ "AG_SurfaceBlit(16-bit RGBA <- T-Clip)", Bench_Blit_T_Clip, 8 },
	{ "AG_SurfaceBlit(16-bit RGBA <- R-Clip)", Bench_Blit_R_Clip, 8 },
	{ "AG_SurfaceBlit(16-bit RGBA <- B-Clip)", Bench_Blit_B_Clip, 8 },
	{ "AG_SurfaceBlit(16-bit RGBA <- L-Clip)", Bench_Blit_L_Clip, 8 },
	{ "AG_SurfaceBlit(16-bit BGRA <- Inside)", Bench_Blit_Inside, 9 },
	{ "AG_SurfaceBlit(16-bit BGRA <- T-Clip)", Bench_Blit_T_Clip, 9 },
	{ "AG_SurfaceBlit(16-bit BGRA <- R-Clip)", Bench_Blit_R_Clip, 9 },
	{ "AG_SurfaceBlit(16-bit BGRA <- B-Clip)", Bench_Blit_B_Clip, 9 },
	{ "AG_SurfaceBlit(16-bit BGRA <- L-Clip)", Bench_Blit_L_Clip, 9 },
	{ "AG_SurfaceBlit(16-bit ABGR <- Inside)", Bench_Blit_Inside, 10 },
	{ "AG_SurfaceBlit(16-bit ABGR <- T-Clip)", Bench_Blit_T_Clip, 10 },
	{ "AG_SurfaceBlit(16-bit ABGR <- R-Clip)", Bench_Blit_R_Clip, 10 },
	{ "AG_SurfaceBlit(16-bit ABGR <- B-Clip)", Bench_Blit_B_Clip, 10 },
	{ "AG_SurfaceBlit(16-bit ABGR <- L-Clip)", Bench_Blit_L_Clip, 10 },
#endif
};
struct ag_benchmark blitOps = {
	"AG_SurfaceBlit(3)",
	&blitOpsFns[0],
	sizeof(blitOpsFns) / sizeof(blitOpsFns[0]),
	10, 100, 10000000
};

static int
Bench(void *obj)
{
	AG_TestInstance *ti = obj;

	TestMsg(ti, "");
	TestMsg(ti, AGSI_LEAGUE_SPARTAN "A G _ S U R F A C E   M I C R O B E N C H M A R K S");

	TestMsg(ti, "AG_SurfaceBlit():");
	TestExecBenchmark(obj, &blitOps);

//	TestMsg(ti, "AG_FillRect():");
//	TestExecBenchmark(obj, &fillRectOps);

	return (0);
}

const AG_TestCase surfaceTest = {
	AGSI_IDEOGRAM AGSI_LOAD_IMAGE AGSI_RST,
	"surface",
	N_("Test AG_Surface(3). Run multiple times to create threads "
	   AGSI_IDEOGRAM AGSI_THREADS AGSI_RST),
	"1.7.0",
	0,
	sizeof(MyTestInstance),
	Init,
	Destroy,
	Test,
	TestGUI,
	Bench
};
