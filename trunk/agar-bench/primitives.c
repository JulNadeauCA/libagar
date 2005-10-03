/*	$Csoft: primitives.c,v 1.3 2005/10/03 06:15:46 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

#include <engine/widget/widget.h>
#include <engine/widget/primitive.h>

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
test_box(void)
{
	agPrim.box(&wid, 0, 0, wid.w/2, wid.h/2, 1, 0);
}

static void
test_box_chamfered(void)
{
	SDL_Rect r;

	r.x = 0;
	r.y = 0;
	r.w = agView->w/2;
	r.h = agView->h/2;
	agPrim.box_chamfered(&wid, &r, 1, 32, 0);
}

static void
test_frame(void)
{
	agPrim.frame(&wid, 0, 0, wid.w, wid.h, 0);
}

static void
test_circle(void)
{
	agPrim.circle(&wid, 0, 0, wid.w/3, 0);
}

static void
test_line(void)
{
	agPrim.line(&wid, 0, 0, wid.w, wid.h, 0);
	agPrim.line(&wid, 0, 0, wid.w, wid.h/2, 0);
}

static void
test_line_blended(void)
{
	Uint8 c[4];

	agPrim.line_blended(&wid, 0, 0, wid.w, wid.h, c, AG_ALPHA_SRC);
	agPrim.line_blended(&wid, 0, 0, wid.w, wid.h/2, c, AG_ALPHA_SRC);
}

static void
test_hline(void)
{
	agPrim.hline(&wid, 1, wid.w, 1, 0);
}

static void
test_vline(void)
{
	agPrim.vline(&wid, 1, 1, wid.h, 0);
}

static void
test_rect_filled(void)
{
	agPrim.rect_filled(&wid, 1, 1, wid.w/2, wid.h/2, 0);
}

static void
test_rect_blended(void)
{
	Uint8 c[4];

	agPrim.rect_blended(&wid, 1, 1, wid.w/2, wid.h/2, c, AG_ALPHA_SRC);
}

static void
test_tiling16(void)
{
	SDL_Rect rd;

	rd.x = 0;
	rd.y = 0;
	rd.w = wid.w;
	rd.h = wid.h;
	agPrim.tiling(&wid, rd, 16, 0, 255, 0);
}

static void
test_tiling32(void)
{
	SDL_Rect rd;

	rd.x = 0;
	rd.y = 0;
	rd.w = wid.w;
	rd.h = wid.h;
	agPrim.tiling(&wid, rd, 32, 0, 255, 0);
}

static struct testfn_ops testfns[] = {
 { "primitive.box()", InitWidget, FreeWidget, test_box },
 { "primitive.box_chamfered()", InitWidget, FreeWidget, test_box_chamfered },
 { "primitive.frame()", InitWidget, FreeWidget, test_frame },
 { "primitive.circle()", InitWidget, FreeWidget, test_circle },
 { "primitive.line()", InitWidget, FreeWidget, test_line },
 { "primitive.line_blended()", InitWidget, FreeWidget, test_line_blended },
 { "primitive.hline()", InitWidget, FreeWidget, test_hline },
 { "primitive.vline()", InitWidget, FreeWidget, test_vline },
 { "primitive.rect_filled()", InitWidget, FreeWidget, test_rect_filled },
 { "primitive.rect_blended(ALPHA_SRC)", InitWidget, FreeWidget,
    test_rect_blended },
 { "primitive.tiling(16x16)", InitWidget, FreeWidget, test_tiling16 },
};

struct test_ops primitives_test = {
	"Geometric primitives",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	TEST_SDL,
	4, 32
};
