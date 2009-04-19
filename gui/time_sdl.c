/*	Public domain	*/

#include <core/core.h>
#include <gui/view.h>

static Uint32
GetTicks(void)
{
	return SDL_GetTicks();
}

static void
Delay(Uint32 ticks)
{
	SDL_Delay(ticks);
}

const AG_TimeOps agTimeOps_SDL = {
	"sdl",
	NULL,
	NULL,
	GetTicks,
	Delay
};
