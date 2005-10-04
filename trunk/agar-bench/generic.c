/*	$Csoft: generic.c,v 1.1 2005/10/03 17:38:40 vedge Exp $	*/
/*	Public domain	*/

#include <engine/engine.h>
#include "agar-bench.h"

SDL_Surface *surface;
SDL_Surface *surface64;
SDL_Surface *surface128;

void
InitSurface(void)
{
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32,
	    agVideoFmt->Rmask,
	    agVideoFmt->Gmask,
	    agVideoFmt->Bmask,
	    0);
	surface64 = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64, 32,
	    agVideoFmt->Rmask,
	    agVideoFmt->Gmask,
	    agVideoFmt->Bmask,
	    0);
	surface128 = SDL_CreateRGBSurface(SDL_SWSURFACE, 128, 128, 32,
	    agVideoFmt->Rmask,
	    agVideoFmt->Gmask,
	    agVideoFmt->Bmask,
	    0);
}

void
FreeSurface(void)
{
	SDL_FreeSurface(surface);
	SDL_FreeSurface(surface64);
	SDL_FreeSurface(surface128);
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
