/*	$Csoft: pixelops.c,v 1.2 2005/10/02 16:06:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

static void
test_view_putpixel(void)
{
	Uint8 *p = agView->v->pixels;

	AG_VIEW_PUT_PIXEL(p, 1);
}

static void
test_view_putpixel2(void)
{
	AG_VIEW_PUT_PIXEL2(4, 5, 1);
}

static void
test_view_putpixel2_clipped(void)
{
	AG_VIEW_PUT_PIXEL2_CLIPPED(1, 1, 1);
}

static void
test_view_putpixel2_clipped_out(void)
{
	AG_VIEW_PUT_PIXEL2_CLIPPED(32000, 32000, 1);
}

static void
test_get_pixel(void)
{
	AG_GET_PIXEL(surface, surface->pixels);
}

static void
test_get_pixel2(void)
{
	AG_GET_PIXEL2(surface, 1, 1);
}

static void
test_put_pixel(void)
{
	AG_PUT_PIXEL(surface, surface->pixels, 1);
}

static void
test_put_pixel2(void)
{
	AG_PUT_PIXEL2(surface, 1, 1, 1);
}

static void
test_put_pixel2_clipped(void)
{
	AG_PUT_PIXEL2_CLIPPED(surface, 1, 1, 1);
}

static void
test_put_pixel2_clipped_out(void)
{
	AG_PUT_PIXEL2_CLIPPED(surface, 35, 100, 1);
}

static void
test_blend_rgba(void)
{
	AG_BLEND_RGBA(surface, surface->pixels, 10, 10, 10, 100, AG_ALPHA_SRC);
}

static void
test_blend_rgba2(void)
{
	AG_BLEND_RGBA2(surface, 1, 1, 10, 10, 10, 100, AG_ALPHA_SRC);
}

static void
test_blend_rgba2_clipped(void)
{
	AG_BLEND_RGBA2_CLIPPED(surface, 1, 1, 10, 10, 10, 100, AG_ALPHA_SRC);
}


static struct testfn_ops testfns[] = {
	{
		"VIEW_PUT_PIXEL()",
		5, 500000,
		TEST_SDL,
		LockView, UnlockView,
		test_view_putpixel
	}, {
		"VIEW_PUT_PIXEL2()",
		5, 500000,
		TEST_SDL,
		LockView, UnlockView,
		test_view_putpixel2
	}, {
		"VIEW_PUT_PIXEL2_CLIPPED() - Visible",
		5, 500000,
		TEST_SDL,
		LockView, UnlockView,
		test_view_putpixel2_clipped
	}, {
		"VIEW_PUT_PIXEL2_CLIPPED() - Clipped",
		5, 500000,
		TEST_SDL,
		LockView, UnlockView,
		test_view_putpixel2_clipped_out
	}, {
		"VIEW_GET_PIXEL()",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_get_pixel
	}, {
		"VIEW_GET_PIXEL2()",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_get_pixel2
	}, {
		"PUT_PIXEL()",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_put_pixel
	}, {
		"PUT_PIXEL2()",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_put_pixel2
	}, {
		"PUT_PIXEL2_CLIPPED() - Visible",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_put_pixel2_clipped
	}, {
		"PUT_PIXEL2_CLIPPED() - Clipped",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_put_pixel2_clipped_out
	}, {
		"BLEND_RGBA()",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_blend_rgba
	}, {
		"BLEND_RGBA2()",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_blend_rgba2
	}, {
		"BLEND_RGBA2_CLIPPED()",
		5, 500000,
		TEST_SDL,
		InitSurface, FreeSurface,
		test_blend_rgba2_clipped
	}
};

struct test_ops pixelops_test = {
	"Pixel operations",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0])
};
