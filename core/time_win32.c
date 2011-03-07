/*	Public domain	*/

#include <config/have_clock_win32.h>
#if defined(_WIN32) && defined(HAVE_CLOCK_WIN32)

#include <core/queue_close.h>			/* Conflicts */
#ifdef _XBOX
#include <xtl.h>
#else
#include <windows.h>
#include <mmsystem.h>
#endif
#include <core/queue_close.h>			/* Conflicts */
#include <core/queue.h>

#include <core/core.h>

static DWORD t0;

static void
Init(void)
{
#ifndef _XBOX
	timeBeginPeriod(1);
#endif
	t0 = timeGetTime();
}

static Uint32
GetTicks(void)
{
	DWORD t;

	t = timeGetTime();
	if (t < t0) {
		return ((~(DWORD)0) - t0) + t;
	} else {
		return (t - t0);
	}
}

static void
Delay(Uint32 ticks)
{
	Sleep(ticks);
}

const AG_TimeOps agTimeOps_win32 = {
	"win32",
	Init,
	NULL,
	GetTicks,
	Delay
};

#endif /* _WIN32 and HAVE_CLOCK_WIN32 */
