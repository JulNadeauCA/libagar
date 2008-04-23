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
	surface = AG_SurfaceVideoRGB(32,32);
	surface64 = AG_SurfaceVideoRGB(64,64);
	surface128 = AG_SurfaceVideo(128,128);
}

void
FreeSurface(void)
{
	AG_SurfaceFree(surface);
	AG_SurfaceFree(surface64);
	AG_SurfaceFree(surface128);
}

void
LockView(void)
{
	AG_SurfaceLock(agView->v);
}

void
UnlockView(void)
{
	AG_SurfaceUnlock(agView->v);
}
