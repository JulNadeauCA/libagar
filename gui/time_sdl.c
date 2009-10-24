/*	Public domain	*/

/*
 * SDL time interface.
 */

#include <config/have_sdl.h>
#ifdef HAVE_SDL

#include <core/core.h>
#include <core/types.h>

static Uint32
GetTicks(void)
{
	extern Uint32 SDL_GetTicks(void);
	return SDL_GetTicks();
}

static void
Delay(Uint32 ticks)
{
	extern void SDL_Delay(Uint32);
	SDL_Delay(ticks);
}

const AG_TimeOps agTimeOps_SDL = {
	"sdl",
	NULL,
	NULL,
	GetTicks,
	Delay
};

#endif /* HAVE_SDL */
