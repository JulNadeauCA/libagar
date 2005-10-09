/*	$Csoft: timeout.h,v 1.6 2005/02/08 15:56:34 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TIMEOUT_H_
#define _AGAR_TIMEOUT_H_
#include "begin_code.h"

typedef struct ag_timeout {
	Uint32 (*fn)(void *p, Uint32 ival, void *arg);
	void *arg;
	int running;			/* Callback is executing */
	int flags;
#define AG_TIMEOUT_DETACHABLE	0x01	/* Don't cancel in AG_ObjectDetach() */
#define AG_TIMEOUT_LOADABLE	0x02	/* Don't cancel in AG_ObjectLoad() */
	Uint32 ticks;			/* Expiry time in SDL ticks */
	Uint32 ival;			/* Interval in ticks */

	CIRCLEQ_ENTRY(ag_timeout) timeouts;	/* Priority queue */
} AG_Timeout;

SLIST_HEAD(ag_timeoutq, ag_timeout);

__BEGIN_DECLS
void	AG_SetTimeout(AG_Timeout *, Uint32 (*)(void *, Uint32, void *), void *,
	              int);
void	AG_ScheduleTimeout(void *, AG_Timeout *, Uint32, int);
int	AG_TimeoutIsScheduled(void *, AG_Timeout *);
#define AG_AddTimeout(p,to,dt) AG_ScheduleTimeout((p), (to), (dt), 0)
#define AG_ReplaceTimeout(p,to,dt) AG_ScheduleTimeout((p), (to), (dt), 1)
void	AG_DelTimeout(void *, AG_Timeout *);

__inline__ void AG_ProcessTimeout(Uint32);
__inline__ void AG_LockTimeouts(void *);
__inline__ void AG_UnlockTimeouts(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_TIMEOUT_H_ */
