/*	Public domain	*/

#include <config/have_gettimeofday.h>
#ifdef HAVE_GETTIMEOFDAY

#include <sys/time.h>
#include <core/core.h>
#include <errno.h>

static struct timeval t0;

static void
Init(void)
{
	gettimeofday(&t0, NULL);
}

static Uint32
GetTicks(void)
{
	struct timeval t;

	gettimeofday(&t, NULL);
	return (Uint32)(t.tv_sec - t0.tv_sec)*1000 +
	               (t.tv_usec - t0.tv_usec)/1000;
}

static void
Delay(Uint32 ticks)
{
	Uint32 t1, t, Telapsed, Tdelay = ticks;
	struct timeval tv;
	int rv;

	t1 = GetTicks();
	for (;;) {
		t = GetTicks();
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
	Init,
	NULL,
	GetTicks,
	Delay
};

#endif /* HAVE_GETTIMEOFDAY */
