/*	Public domain	*/
/*
 * Time backend for monotonically-increasing clock on POSIX platforms.
 */

#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <agar/core/core.h>

static struct timespec t0;

static void
POSIX_Init(void)
{
	clock_gettime(CLOCK_MONOTONIC, &t0);
}

static Uint32
POSIX_GetTicks(void)
{
	struct timespec t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	return (Uint32)(t.tv_sec - t0.tv_sec)*1000 +
	               (t.tv_nsec - t0.tv_nsec)/1000000L;
}

static void
POSIX_Delay(Uint32 ticks)
{
	struct timespec tv, tvElapsed;

	tvElapsed.tv_sec = ticks/1000;
	tvElapsed.tv_nsec = (ticks % 1000)*1000000L;
	for (;;) {
		tv.tv_sec = tvElapsed.tv_sec;
		tv.tv_nsec = tvElapsed.tv_nsec;
		if (nanosleep(&tv, &tvElapsed) == -1 &&
		    errno == EINTR) {
			continue;
		}
		break;
	}
}

const AG_TimeOps agTimeOps_posix = {
	"posix",
	POSIX_Init,
	NULL,
	POSIX_GetTicks,
	POSIX_Delay
};
