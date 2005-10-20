/*	$Csoft: primitives.c,v 1.4 2005/10/03 07:17:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>

static SDL_Surface *su;
static AG_Widget wid;

static void
InitWidget(void)
{
	AG_WidgetInit(&wid, "test-widget", NULL, 0);
	wid.cx = 1;
	wid.cy = 1;
	wid.x = 0;
	wid.y = 0;
	wid.w = agView->w-1;
	wid.h = agView->h-1;
	wid.cx2 = wid.cx + wid.w;
	wid.cy2 = wid.cy + wid.h;
}

static void
FreeWidget(void)
{
	AG_WidgetDestroy(&wid);
}

static void
T_Box(void)
{
	agPrim.box(&wid, 0, 0, wid.w/2, wid.h/2, 1, 0);
}

static void
T_BoxChamfered(void)
{
	SDL_Rect r;

	r.x = 0;
	r.y = 0;
	r.w = agView->w/2;
	r.h = agView->h/2;
	agPrim.box_chamfered(&wid, &r, 1, 32, 0);
}

static void
T_Frame(void)
{
	agPrim.frame(&wid, 0, 0, wid.w, wid.h, 0);
}

static void
T_Circle(void)
{
	agPrim.circle(&wid, 0, 0, wid.w/3, 0);
}

static void
T_Line(void)
{
	agPrim.line(&wid, 0, 0, wid.w, wid.h, 0);
	agPrim.line(&wid, 0, 0, wid.w, wid.h/2, 0);
}

static void
T_LineBlended(void)
{
	Uint8 c[4];

	agPrim.line_blended(&wid, 0, 0, wid.w, wid.h, c, AG_ALPHA_SRC);
	agPrim.line_blended(&wid, 0, 0, wid.w, wid.h/2, c, AG_ALPHA_SRC);
}

static void
T_HLine(void)
{
	agPrim.hline(&wid, 1, 479, 1, 0);
}

static void
T_VLine(void)
{
	agPrim.vline(&wid, 1, 1, 479, 0);
}

static void
T_RectFilled(void)
{
	agPrim.rect_filled(&wid, 1, 1, 256, 256, 0);
}

static void
T_RectBlended(void)
{
	Uint8 c[4];

	agPrim.rect_blended(&wid, 1, 1, 128, 128, c, AG_ALPHA_SRC);
}

static void
T_Tiling16(void)
{
	SDL_Rect rd;

	rd.x = 0;
	rd.y = 0;
	rd.w = wid.w;
	rd.h = wid.h;
	agPrim.tiling(&wid, rd, 16, 0, 255, 0);
}

static void
T_Tiling32(void)
{
	SDL_Rect rd;

	rd.x = 0;
	rd.y = 0;
	rd.w = wid.w;
	rd.h = wid.h;
	agPrim.tiling(&wid, rd, 32, 0, 255, 0);
}

static struct testfn_ops testfns[] = {
 { "primitive.box()", InitWidget, FreeWidget, T_Box },
 { "primitive.box_chamfered()", InitWidget, FreeWidget, T_BoxChamfered },
 { "primitive.frame()", InitWidget, FreeWidget, T_Frame },
 { "primitive.circle()", InitWidget, FreeWidget, T_Circle },
 { "primitive.line()", InitWidget, FreeWidget, T_Line },
 { "primitive.line_blended()", InitWidget, FreeWidget, T_LineBlended },
 { "primitive.hline(479px)", InitWidget, FreeWidget, T_HLine },
 { "primitive.vline(479px)", InitWidget, FreeWidget, T_VLine },
 { "primitive.rect_filled(256x256)", InitWidget, FreeWidget, T_RectFilled },
 { "primitive.rect_blended(128x128, ALPHA_SRC)", InitWidget, FreeWidget,
    T_RectBlended },
 { "primitive.tiling(16x16)", InitWidget, FreeWidget, T_Tiling16 },
 { "primitive.tiling(32x32)", InitWidget, FreeWidget, T_Tiling32 },
};

struct test_ops primitives_test = {
	"Primitives",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	TEST_SDL,
	4, 32
};
