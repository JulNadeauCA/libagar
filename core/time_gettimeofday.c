/*	Public domain	*/
/*
 * Time backend for the traditional gettimeofday() interface.
 * Delay() is implemented using the "timeout" argument of select().
 */

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <agar/core/core.h>

static struct timeval t0;

static void
GTOD_Init(void)
{
	gettimeofday(&t0, NULL);
}

static Uint32
GTOD_GetTicks(void)
{
	struct timeval t;

	gettimeofday(&t, NULL);
	return (Uint32)(t.tv_sec - t0.tv_sec)*1000 +
	               (t.tv_usec - t0.tv_usec)/1000;
}

static void
GTOD_Delay(Uint32 ticks)
{
	Uint32 t1, t, Telapsed, Tdelay = ticks;
	struct timeval tv;
	int rv;

	t1 = GTOD_GetTicks();
	for (;;) {
		t = GTOD_GetTicks();
		Telapsed = (t - t1);
		t1 = t;
		if (Telapsed >= Tdelay) {
			break;
		}
		Tdelay -= Telapsed;
		tv.tv_sec = Tdelay/1000;
		tv.tv_usec = (Tdelay % 1000)*1000;
		if ((rv = select(0, NULL, NULL, NULL, &tv)) == -1 &&
		    errno == EINTR) {
			continue;
		}
		break;
	}
}

const AG_TimeOps agTimeOps_gettimeofday = {
	"gettimeofday",
	GTOD_Init,
	NULL,
	GTOD_GetTicks,
	GTOD_Delay
};
