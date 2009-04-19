/*	Public domain	*/

#ifdef _WIN32

#include <windows.h>
#include <mmsystem.h>

#include <core/core.h>

static DWORD t0;

static void
Init(void)
{
	timeBeginPeriod(1);
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

#endif /* _WIN32 */
