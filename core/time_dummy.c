/*	Public domain	*/

#include <agar/core/core.h>

static Uint32
DUMMY_GetTicks(void)
{
	static Uint32 t = 0;
	return t++;
}

static void
DUMMY_Delay(Uint32 ticks)
{
}

const AG_TimeOps agTimeOps_dummy = {
	"dummy",
	NULL,			/* init */
	NULL,			/* destroy */
	DUMMY_GetTicks,
	DUMMY_Delay
};
