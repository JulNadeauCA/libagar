/*	$Csoft: surfaceops.c,v 1.2 2005/10/03 06:15:46 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"

static void
Test_DupSurface(void)
{
	SDL_Surface *dup;

	dup = AG_DupSurface(surface);
	SDL_FreeSurface(dup);
}

static struct testfn_ops testfns[] = {
 { "DupSurface+FreeSurface (32x32x32)", InitSurface, FreeSurface,
   Test_DupSurface }
};

struct test_ops surfaceops_test = {
	"Surface operations",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	TEST_SDL,
	4, 64
};
