/*	$Csoft: pixelops.c,v 1.1 2005/09/27 03:48:58 vedge Exp $	*/
/*	Public domain	*/

#include "test.h"

static void
Test_DupSurface(void)
{
	SDL_Surface *dup;

	dup = AG_DupSurface(surface);
	SDL_FreeSurface(dup);
}

static struct testfn_ops testfns[] = {
	{
		"DupSurface+FreeSurface (32x32x32)",
		5, 500,
		0,
		InitSurface, FreeSurface,
		Test_DupSurface
	}
};

struct test_ops surfaceops_test = {
	"Surface operations",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0])
};
