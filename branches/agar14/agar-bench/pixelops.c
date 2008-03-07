/*	$Csoft: pixelops.c,v 1.3 2005/10/03 06:15:46 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

static void
T_SDL_LockSurface(void)
{
	SDL_LockSurface(agView->v);
	SDL_UnlockSurface(agView->v);
}

static void
T_ViewPutPixel(void)
{
	AG_VIEW_PUT_PIXEL((Uint8 *)agView->v->pixels, 1);
}

static void
T_ViewPutPixel8(void)
{
	*(Uint8 *)(agView->v->pixels) = 1;
}

static void
T_ViewPutPixel16(void)
{
	*(Uint16 *)((Uint8 *)agView->v->pixels) = 1;
}

static void
T_ViewPutPixel24(void)
{
	((Uint8 *)agView->v->pixels)[0] = (0x1234>>16) & 0xff;
	((Uint8 *)agView->v->pixels)[1] = (0x1234>>8) & 0xff;
	((Uint8 *)agView->v->pixels)[2] =  0x1234 & 0xff;
}

static void
T_ViewPutPixel32(void)
{
	*(Uint32 *)((Uint8 *)agView->v->pixels) = 1;
}

static void
T_ViewPutPixel2(void)
{
	AG_VIEW_PUT_PIXEL2(4, 5, 1);
}

static void
T_ViewPutPixel2ClipIn(void)
{
	AG_VIEW_PUT_PIXEL2_CLIPPED(1, 1, 1);
}

static void
T_ViewPutPixel2ClipOut(void)
{
	AG_VIEW_PUT_PIXEL2_CLIPPED(32000, 32000, 1);
}

static void
T_GetPixel(void)
{
	AG_GET_PIXEL(surface, surface->pixels);
}

static void
T_GetPixel2(void)
{
	AG_GET_PIXEL2(surface, 1, 1);
}

static void
T_PutPixel(void)
{
	AG_PUT_PIXEL(surface, surface->pixels, 1);
}

static void
T_PutPixel2(void)
{
	AG_PUT_PIXEL2(surface, 1, 1, 1);
}

static void
T_PutPixel2ClipIn(void)
{
	AG_PUT_PIXEL2_CLIPPED(surface, 1, 1, 1);
}

static void
T_PutPixel2ClipOut(void)
{
	AG_PUT_PIXEL2_CLIPPED(surface, 35, 100, 1);
}

static void
T_BlendRGBA(void)
{
	AG_BLEND_RGBA(surface, surface->pixels, 10, 10, 10, 100, AG_ALPHA_SRC);
}

static void
T_BlendRGBA2(void)
{
	AG_BLEND_RGBA2(surface, 1, 1, 10, 10, 10, 100, AG_ALPHA_SRC);
}

static void
T_BlendRGBA2Clip(void)
{
	AG_BLEND_RGBA2_CLIPPED(surface, 1, 1, 10, 10, 10, 100, AG_ALPHA_SRC);
}

static struct testfn_ops testfns[] = {
 { "SDL_{Lock,Unlock}Surface()", NULL, NULL, T_SDL_LockSurface, },
 { "AG_VIEW_PUT_PIXEL()", LockView, UnlockView, T_ViewPutPixel },
 { "AG_VIEW_PUT_PIXEL8()", LockView, UnlockView, T_ViewPutPixel8 },
 { "AG_VIEW_PUT_PIXEL16()", LockView, UnlockView, T_ViewPutPixel16 },
 { "AG_VIEW_PUT_PIXEL24()", LockView, UnlockView, T_ViewPutPixel24 },
 { "AG_VIEW_PUT_PIXEL32()", LockView, UnlockView, T_ViewPutPixel32 },
 { "AG_VIEW_PUT_PIXEL2()", LockView, UnlockView, T_ViewPutPixel2 },
 { "AG_VIEW_PUT_PIXEL2_CLIPPED() - Visible", LockView, UnlockView,
   T_ViewPutPixel2ClipIn },
 { "AG_VIEW_PUT_PIXEL2_CLIPPED() - Clipped out", LockView, UnlockView,
   T_ViewPutPixel2ClipOut },
 { "AG_GET_PIXEL()", InitSurface, FreeSurface, T_GetPixel },
 { "AG_GET_PIXEL2()", InitSurface, FreeSurface, T_GetPixel2 },
 { "AG_PUT_PIXEL()", InitSurface, FreeSurface, T_PutPixel },
 { "AG_PUT_PIXEL2()", InitSurface, FreeSurface, T_PutPixel2 },
 { "AG_PUT_PIXEL2_CLIPPED() - Visible", InitSurface, FreeSurface,
   T_PutPixel2ClipIn },
 { "AG_PUT_PIXEL2_CLIPPED() - Clipped out", InitSurface, FreeSurface,
   T_PutPixel2ClipOut },
 { "AG_BLEND_RGBA()", InitSurface, FreeSurface, T_BlendRGBA },
 { "AG_BLEND_RGBA2()", InitSurface, FreeSurface, T_BlendRGBA2 },
 { "AG_BLEND_RGBA2_CLIPPED()", InitSurface, FreeSurface, T_BlendRGBA2Clip },
};

struct test_ops pixelops_test = {
	"Pixel operations",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	TEST_SDL,
	4, 65536, 0
};
