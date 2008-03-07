/*	$Csoft: surfaceops.c,v 1.3 2005/10/03 07:17:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

static void
T_DupSurface(void)
{
	SDL_Surface *dup;

	dup = AG_DupSurface(surface);
	SDL_FreeSurface(dup);
}

static void T_Scale32To32(void) {
	AG_ScaleSurface(surface, 32, 32, &surface);
}
static void T_Scale32To64(void) {
	AG_ScaleSurface(surface, 64, 64, &surface64);
}
static void T_Scale64To32(void) {
	AG_ScaleSurface(surface64, 32, 32, &surface);
}
static void T_Scale32To128(void) {
	AG_ScaleSurface(surface, 128, 128, &surface128);
}
static void T_Scale128To32(void) {
	AG_ScaleSurface(surface128, 32, 32, &surface);
}
static void T_Flip128(void) {
	AG_FlipSurface(surface128->pixels, surface128->h,
	    surface128->pitch);
}

static void T_Scale32To64Copy(void) {
	SDL_Surface *s2 = NULL;

	AG_ScaleSurface(surface, 64, 64, &s2);
	SDL_FreeSurface(s2);
}

static struct testfn_ops testfns[] = {
 { "DupSurface+Free (32)", InitSurface, FreeSurface, T_DupSurface },
 { "Scale(32->32)", InitSurface, FreeSurface, T_Scale32To32 },
 { "Scale(32->64)", InitSurface, FreeSurface, T_Scale32To64 },
 { "Scale(64->32)", InitSurface, FreeSurface, T_Scale64To32 },
 { "Scale(32->64)+Copy", InitSurface, FreeSurface, T_Scale32To64Copy },
 { "Scale(32->128)", InitSurface, FreeSurface, T_Scale32To128 },
 { "Scale(128->32)", InitSurface, FreeSurface, T_Scale128To32 },
 { "Flip(128)", InitSurface, FreeSurface, T_Flip128 },
};

struct test_ops surfaceops_test = {
	"Surface",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	TEST_SDL,
	4, 64, 0
};
