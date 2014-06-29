/*	Public domain	*/
/*
 * Rendering-aware time backend. This is useful for applications performing
 * offline rendering, where the outcome of the rendering may be influenced by
 * different threads, each using AG_Delay() calls or Agar timers.
 *
 * Using this backend, the start of a rendering cycle causes AG_Delay() to
 * block until the rendering cycle is complete.
 */

#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <agar/core/core.h>

AG_Cond agCondBeginRender;
AG_Cond agCondEndRender;
AG_Mutex agCondRenderLock;

static struct timespec t0;
static int inited = 0;

static void
RENDERER_Init(void)
{
	if (!inited) {
		AG_CondInit(&agCondBeginRender);
		AG_CondInit(&agCondEndRender);
		AG_MutexInit(&agCondRenderLock);
		inited = 1;
	}
	clock_gettime(CLOCK_MONOTONIC, &t0);
}

static Uint32
RENDERER_GetTicks(void)
{
	struct timespec t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	return (Uint32)(t.tv_sec - t0.tv_sec)*1000 +
	               (t.tv_nsec - t0.tv_nsec)/1000000;
}

static void
RENDERER_Delay(Uint32 Tdelay)
{
	struct timespec ts, tsNow;
	int rv;

	AG_MutexLock(&agCondRenderLock);
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec += (Tdelay/1000);
	ts.tv_nsec += (Tdelay % 1000)*1000000;

	for (;;) {
		rv = AG_CondTimedWait(&agCondBeginRender, &agCondRenderLock, &ts);
		if (rv != 0) {
			clock_gettime(CLOCK_MONOTONIC, &tsNow);
			if (tsNow.tv_sec < ts.tv_sec) {
				continue;
			}
			if (tsNow.tv_sec > ts.tv_sec ||
			    tsNow.tv_nsec >= ts.tv_nsec)
				break;
		}
	}
	AG_MutexUnlock(&agCondRenderLock);
}

const AG_TimeOps agTimeOps_renderer = {
	"renderer",
	RENDERER_Init,
	NULL,
	RENDERER_GetTicks,
	RENDERER_Delay
};
