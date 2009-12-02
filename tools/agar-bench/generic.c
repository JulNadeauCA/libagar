/*	$Csoft: generic.c,v 1.1 2005/10/03 17:38:40 vedge Exp $	*/
/*	Public domain	*/

#include <agar/core.h>

#include "agar-bench.h"

AG_Surface *surface;
AG_Surface *surface64;
AG_Surface *surface128;

void
InitSurface(void)
{
	surface = AG_SurfaceStdRGB(32,32);
	surface64 = AG_SurfaceStdRGB(64,64);
	surface128 = AG_SurfaceStdRGB(128,128);
}

void
FreeSurface(void)
{
	AG_SurfaceFree(surface);
	AG_SurfaceFree(surface64);
	AG_SurfaceFree(surface128);
}
