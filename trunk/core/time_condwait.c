/*	Public domain	*/

#include <config/ag_threads.h>
#include <config/have_gettimeofday.h>
#include <config/have_clock_gettime.h>
#include <config/have_cygwin.h>
#if defined(AG_THREADS) && defined(HAVE_GETTIMEOFDAY) && \
    defined(HAVE_CLOCK_GETTIME) && !defined(HAVE_CYGWIN)

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#include "core.h"
#include <errno.h>

AG_Cond agCondBeginRender;
AG_Cond agCondEndRender;
AG_Mutex agCondRenderLock;
static struct timeval t0;

static void
Init(void)
{
	AG_CondInit(&agCondBeginRender);
	AG_CondInit(&agCondEndRender);
	AG_MutexInit(&agCondRenderLock);
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
Delay(Uint32 Tdelay)
{
	Uint32 t0, t, Telapsed = 0;
	struct timespec ts;

	AG_MutexLock(&agCondRenderLock);
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += (Tdelay/1000);
	ts.tv_nsec += (Tdelay % 1000)*1000000;
	while (Telapsed < Tdelay) {
		t0 = GetTicks();
		if (AG_CondTimedWait(&agCondBeginRender,
		    &agCondRenderLock, &ts) != 0) {
			t = GetTicks();
			Telapsed += (t - t0);
		} else {
			AG_CondWait(&agCondEndRender, &agCondRenderLock);
		}
	}
	AG_MutexUnlock(&agCondRenderLock);
}

const AG_TimeOps agTimeOps_condwait = {
	"condwait",
	Init,
	NULL,
	GetTicks,
	Delay
};

#endif /* AG_THREADS and HAVE_GETTIMEOFDAY and 
          HAVE_CLOCK_GETTIME and !HAVE_CYGWIN */
