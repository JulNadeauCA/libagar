/*	Public domain	*/

#ifndef _AGAR_TIMEOUT_H_
#define _AGAR_TIMEOUT_H_
#include "begin_code.h"

typedef struct ag_timeout {
	Uint32 (*fn)(void *p, Uint32 ival, void *arg);
	void *arg;
	int running;			/* Callback is executing */
	Uint flags;
#define AG_CANCEL_ONDETACH	0x01	/* Cancel on ObjectDetach() */
#define AG_CANCEL_ONLOAD	0x02	/* Cancel on ObjectLoad() */
	Uint32 ticks;			/* Expiry time in SDL ticks */
	Uint32 ival;			/* Interval in ticks */

	CIRCLEQ_ENTRY(ag_timeout) timeouts;	/* Priority queue */
} AG_Timeout;

SLIST_HEAD(ag_timeoutq, ag_timeout);

#define AG_TIMEOUT_INITIALIZER { NULL, NULL, 0, 0, 0, 1 }

__BEGIN_DECLS
void	AG_SetTimeout(AG_Timeout *, Uint32 (*)(void *, Uint32, void *), void *,
	              Uint);
void	AG_ScheduleTimeout(void *, AG_Timeout *, Uint32, int);
int	AG_TimeoutIsScheduled(void *, AG_Timeout *);
#define AG_AddTimeout(p,to,dt) AG_ScheduleTimeout((p), (to), (dt), 0)
#define AG_ReplaceTimeout(p,to,dt) AG_ScheduleTimeout((p), (to), (dt), 1)
void	AG_DelTimeout(void *, AG_Timeout *);
int	AG_TimeoutWait(void *, AG_Timeout *, Uint32);

__inline__ void AG_ProcessTimeout(Uint32);
__inline__ void AG_LockTimeouts(void *);
__inline__ void AG_UnlockTimeouts(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_TIMEOUT_H_ */
