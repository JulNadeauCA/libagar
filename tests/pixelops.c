/*	$Csoft: pixelops.c,v 1.3 2005/05/24 08:19:34 vedge Exp $	*/
/*	Public domain	*/

#include "test.h"

static SDL_Surface *su;

static void
init_su(void)
{
	su = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32,
	    vfmt->Rmask,
	    vfmt->Gmask,
	    vfmt->Bmask,
	    0);
}

static void
init_view(void)
{
	SDL_LockSurface(view->v);
}

static void
destroy_view(void)
{
	SDL_UnlockSurface(view->v);
}

static void
test_view_putpixel(void)
{
	Uint8 *p = view->v->pixels;

	VIEW_PUT_PIXEL(p, 1);
}

static void
test_view_putpixel2(void)
{
	VIEW_PUT_PIXEL2(4, 5, 1);
}

static void
test_view_putpixel2_clipped(void)
{
	VIEW_PUT_PIXEL2_CLIPPED(1, 1, 1);
}

static void
test_view_putpixel2_clipped_out(void)
{
	VIEW_PUT_PIXEL2_CLIPPED(32000, 32000, 1);
}

static void
test_get_pixel(void)
{
	GET_PIXEL(su, su->pixels);
}

static void
test_get_pixel2(void)
{
	GET_PIXEL2(su, 1, 1);
}

static void
test_put_pixel(void)
{
	PUT_PIXEL(su, su->pixels, 1);
}

static void
test_put_pixel2(void)
{
	PUT_PIXEL2(su, 1, 1, 1);
}

static void
test_put_pixel2_clipped(void)
{
	PUT_PIXEL2_CLIPPED(su, 1, 1, 1);
}

static void
test_put_pixel2_clipped_out(void)
{
	PUT_PIXEL2_CLIPPED(su, 35, 100, 1);
}

static void
test_blend_rgba(void)
{
	BLEND_RGBA(su, su->pixels, 10, 10, 10, 100, ALPHA_SRC);
}

static void
test_blend_rgba2(void)
{
	BLEND_RGBA2(su, 1, 1, 10, 10, 10, 100, ALPHA_SRC);
}

static void
test_blend_rgba2_clipped(void)
{
	BLEND_RGBA2_CLIPPED(su, 1, 1, 10, 10, 10, 100, ALPHA_SRC);
}


static void
destroy_su(void)
{
	SDL_FreeSurface(su);
}

static struct testfn_ops testfns[] = {
	{
		"VIEW_PUT_PIXEL()",
		5, 500000,
		TEST_SDL,
		init_view, destroy_view,
		test_view_putpixel
	}, {
		"VIEW_PUT_PIXEL2()",
		5, 500000,
		TEST_SDL,
		init_view, destroy_view,
		test_view_putpixel2
	}, {
		"VIEW_PUT_PIXEL2_CLIPPED() - Visible",
		5, 500000,
		TEST_SDL,
		init_view, destroy_view,
		test_view_putpixel2_clipped
	}, {
		"VIEW_PUT_PIXEL2_CLIPPED() - Clipped",
		5, 500000,
		TEST_SDL,
		init_view, destroy_view,
		test_view_putpixel2_clipped_out
	}, {
		"VIEW_GET_PIXEL()",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_get_pixel
	}, {
		"VIEW_GET_PIXEL2()",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_get_pixel2
	}, {
		"PUT_PIXEL()",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_put_pixel
	}, {
		"PUT_PIXEL2()",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_put_pixel2
	}, {
		"PUT_PIXEL2_CLIPPED() - Visible",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_put_pixel2_clipped
	}, {
		"PUT_PIXEL2_CLIPPED() - Clipped",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_put_pixel2_clipped_out
	}, {
		"BLEND_RGBA()",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_blend_rgba
	}, {
		"BLEND_RGBA2()",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_blend_rgba2
	}, {
		"BLEND_RGBA2_CLIPPED()",
		5, 500000,
		TEST_SDL,
		init_su, destroy_su,
		test_blend_rgba2_clipped
	}
};

struct test_ops pixelops_test = {
	"Pixel operations",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0])
};
