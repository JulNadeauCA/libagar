/*	$Csoft$	*/
/*	Public domain	*/

#include <engine/engine.h>
#include "agar-bench.h"

SDL_Surface *surface;

void
InitSurface(void)
{
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32,
	    agVideoFmt->Rmask,
	    agVideoFmt->Gmask,
	    agVideoFmt->Bmask,
	    0);
}

void
FreeSurface(void)
{
	SDL_FreeSurface(surface);
}

void
LockView(void)
{
	SDL_LockSurface(agView->v);
}

void
UnlockView(void)
{
	SDL_UnlockSurface(agView->v);
}
