/*	$Csoft: pixelops.c,v 1.3 2005/05/24 08:19:34 vedge Exp $	*/
/*	Public domain	*/

#include "test.h"

#include <engine/widget/widget.h>
#include <engine/widget/primitive.h>

static SDL_Surface *su;
static struct widget wid;

static void
init_widget(void)
{
	widget_init(&wid, "test-widget", NULL, 0);
	wid.cx = 1;
	wid.cy = 1;
	wid.x = 0;
	wid.y = 0;
	wid.w = view->w-1;
	wid.h = view->h-1;
	wid.cx2 = wid.cx + wid.w;
	wid.cy2 = wid.cy + wid.h;
}

static void
destroy_widget(void)
{
	widget_destroy(&wid);
}

static void
test_box(void)
{
	primitives.box(&wid, 0, 0, wid.w/2, wid.h/2, 1, 0);
}

static void
test_box_chamfered(void)
{
	SDL_Rect r;

	r.x = 0;
	r.y = 0;
	r.w = view->w/2;
	r.h = view->h/2;
	primitives.box_chamfered(&wid, &r, 1, 32, 0);
}

static void
test_frame(void)
{
	primitives.frame(&wid, 0, 0, wid.w, wid.h, 0);
}

static void
test_circle(void)
{
	primitives.circle(&wid, 0, 0, wid.w/3, 0);
}

static void
test_line(void)
{
	primitives.line(&wid, 0, 0, wid.w, wid.h, 0);
	primitives.line(&wid, 0, 0, wid.w, wid.h/2, 0);
}

static void
test_line_blended(void)
{
	Uint8 c[4];

	primitives.line_blended(&wid, 0, 0, wid.w, wid.h, c, ALPHA_SRC);
	primitives.line_blended(&wid, 0, 0, wid.w, wid.h/2, c, ALPHA_SRC);
}

static void
test_hline(void)
{
	primitives.hline(&wid, 1, wid.w, 1, 0);
}

static void
test_vline(void)
{
	primitives.vline(&wid, 1, 1, wid.h, 0);
}

static void
test_rect_filled(void)
{
	primitives.rect_filled(&wid, 1, 1, wid.w/2, wid.h/2, 0);
}

static void
test_rect_blended(void)
{
	Uint8 c[4];

	primitives.rect_blended(&wid, 1, 1, wid.w/2, wid.h/2, c, ALPHA_SRC);
}

static void
test_tiling16(void)
{
	SDL_Rect rd;

	rd.x = 0;
	rd.y = 0;
	rd.w = wid.w/2;
	rd.h = wid.h/2;
	primitives.tiling(&wid, rd, 16, 0, 255, 0);
}

static void
destroy_su(void)
{
	SDL_FreeSurface(su);
}

static struct testfn_ops testfns[] = {
	{
		"primitive.box()",
		2, 50,
		0,
		init_widget, destroy_widget,
		test_box
	},
	{
		"primitive.box_chamfered()",
		2, 50,
		0,
		init_widget, destroy_widget,
		test_box_chamfered
	},
	{
		"primitive.frame()",
		2, 100,
		0,
		init_widget, destroy_widget,
		test_frame
	},
	{
		"primitive.circle()",
		2, 100,
		0,
		init_widget, destroy_widget,
		test_circle
	},
	{
		"primitive.line()",
		2, 100,
		0,
		init_widget, destroy_widget,
		test_line
	},
	{
		"primitive.line_blended(src)",
		2, 100,
		0,
		init_widget, destroy_widget,
		test_line_blended
	},
	{
		"primitive.hline()",
		3, 200,
		0,
		init_widget, destroy_widget,
		test_hline
	},
	{
		"primitive.vline()",
		2, 100,
		0,
		init_widget, destroy_widget,
		test_vline
	},
	{
		"primitive.rect_filled()",
		2, 30,
		0,
		init_widget, destroy_widget,
		test_rect_filled
	},
	{
		"primitive.rect_blended(src)",
		2, 10,
		0,
		init_widget, destroy_widget,
		test_rect_blended
	},
	{
		"primitive.tiling(16px)",
		2, 10,
		0,
		init_widget, destroy_widget,
		test_tiling16
	},
};

struct test_ops primitives_test = {
	"Geometric primitives",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0])
};
