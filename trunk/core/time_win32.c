/*	Public domain	*/
/*
 * Time backend for the timeGetTime() interface on Windows platforms.
 */

#if defined(_WIN32)

#include <agar/core/queue_close.h>			/* Conflicts */
#ifdef _XBOX
#include <xtl.h>
#else
#include <windows.h>
#include <mmsystem.h>
#endif
#include <agar/core/queue_close.h>			/* Conflicts */
#include <agar/core/queue.h>

#include <agar/core/core.h>

static DWORD t0;

static void
WIN32_Init(void)
{
#ifndef _XBOX
	timeBeginPeriod(1);
#endif
	t0 = timeGetTime();
}

static Uint32
WIN32_GetTicks(void)
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
WIN32_Delay(Uint32 ticks)
{
	Sleep(ticks);
}

const AG_TimeOps agTimeOps_win32 = {
	"win32",
	WIN32_Init,
	NULL,
	WIN32_GetTicks,
	WIN32_Delay
};

#endif /* _WIN32 */
