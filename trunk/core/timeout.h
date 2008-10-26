/*	Public domain	*/

#include <agar/core/begin.h>

typedef struct ag_timeout {
	Uint32 (*fn)(void *p, Uint32 ival, void *arg);
	void *arg;
	int running;			/* Callback is executing */
	Uint flags;
#define AG_CANCEL_ONDETACH	0x01	/* Cancel on ObjectDetach() */
#define AG_CANCEL_ONLOAD	0x02	/* Cancel on ObjectLoad() */
	Uint32 ticks;			/* Expiry time in SDL ticks */
	Uint32 ival;			/* Interval in ticks */

	AG_TAILQ_ENTRY(ag_timeout) timeouts;	/* Priority queue */
} AG_Timeout;

AG_SLIST_HEAD(ag_timeoutq, ag_timeout);

#define AG_TIMEOUT_INITIALIZER { NULL, NULL, 0, 0, 0, 1 }
#define AG_TIMEOUTS_QUEUED() (!AG_TAILQ_EMPTY(&agTimeoutObjQ))

#define AG_LockTiming() AG_MutexLock(&agTimingLock)
#define AG_UnlockTiming() AG_MutexUnlock(&agTimingLock)

__BEGIN_DECLS
extern struct ag_objectq agTimeoutObjQ;
#ifdef THREADS
extern AG_Mutex agTimingLock;
#endif

void	AG_InitTimeouts(void);
void	AG_DestroyTimeouts(void);

void	AG_SetTimeout(AG_Timeout *, Uint32 (*)(void *, Uint32, void *), void *,
	              Uint);
void	AG_ScheduleTimeout(void *, AG_Timeout *, Uint32, int);
int	AG_TimeoutIsScheduled(void *, AG_Timeout *);
#define AG_AddTimeout(p,to,dt) AG_ScheduleTimeout((p), (to), (dt), 0)
#define AG_ReplaceTimeout(p,to,dt) AG_ScheduleTimeout((p), (to), (dt), 1)
void	AG_DelTimeout(void *, AG_Timeout *);
int	AG_TimeoutWait(void *, AG_Timeout *, Uint32);
void	AG_ProcessTimeouts(Uint32);
void	AG_LockTimeouts(void *);
void	AG_UnlockTimeouts(void *);

/* Legacy */
#define	AG_ProcessTimeout(x) AG_ProcessTimeouts(x)
__END_DECLS

#include <agar/core/close.h>
