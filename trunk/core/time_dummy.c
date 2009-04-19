/*	Public domain	*/

#include <core/core.h>

static Uint32
GetTicks(void)
{
	static Uint32 t = 0;
	return t++;
}

static void
Delay(Uint32 ticks)
{
}

const AG_TimeOps agTimeOps_dummy = {
	"dummy",
	NULL,
	NULL,
	GetTicks,
	Delay
};
