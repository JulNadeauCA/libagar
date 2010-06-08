/*	Public domain	*/

#include <agar/core/begin.h>

typedef struct ag_timeout {
	Uint32 (*fn)(void *p, Uint32 ival, void *arg);
	void *arg;
	Uint flags;
#define AG_CANCEL_ONDETACH	0x01	/* Cancel on ObjectDetach() */
#define AG_CANCEL_ONLOAD	0x02	/* Cancel on ObjectLoad() */
#define AG_TIMEOUT_ACTIVE	0x04	/* In queue for execution */
	Uint32 ticks;			/* Expiry time in ms */
	Uint32 ival;			/* Interval in ticks */

	AG_TAILQ_ENTRY(ag_timeout) timeouts;	/* Priority queue */
} AG_Timeout;

AG_SLIST_HEAD(ag_timeoutq, ag_timeout);

#define AG_TIMEOUT_INITIALIZER { NULL, NULL, 0, 0, 1 }
#define AG_TIMEOUTS_QUEUED() (!AG_TAILQ_EMPTY(&agTimeoutObjQ))

#define AG_LockTiming() AG_MutexLock(&agTimingLock)
#define AG_UnlockTiming() AG_MutexUnlock(&agTimingLock)

__BEGIN_DECLS
extern struct ag_objectq agTimeoutObjQ;
extern struct ag_object agTimeoutMgr;
#ifdef AG_THREADS
extern AG_Mutex agTimingLock;
#endif

void	AG_InitTimeouts(void);
void	AG_DestroyTimeouts(void);

void	AG_SetTimeout(AG_Timeout *, Uint32 (*)(void *, Uint32, void *), void *,
	              Uint);
void	AG_ScheduleTimeout(void *, AG_Timeout *, Uint32);
void	AG_DelTimeout(void *, AG_Timeout *);
int	AG_TimeoutWait(void *, AG_Timeout *, Uint32);
void	AG_ProcessTimeouts(Uint32);

static __inline__ int
AG_TimeoutIsScheduled(void *obj, AG_Timeout *to)
{
	return (to->flags & AG_TIMEOUT_ACTIVE);
}

#ifdef AG_LEGACY
# define AG_ProcessTimeout(x) AG_ProcessTimeouts(x)
# define AG_AddTimeout(p,to,dt) AG_ScheduleTimeout((p),(to),(dt))
# define AG_ReplaceTimeout(p,to,dt) AG_ScheduleTimeout((p),(to),(dt))
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/core/close.h>
