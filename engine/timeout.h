/*	$Csoft: timeout.h,v 1.4 2004/05/15 06:07:11 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TIMEOUT_H_
#define _AGAR_TIMEOUT_H_
#include "begin_code.h"

SLIST_HEAD(timeoutq, timeout);

struct timeout {
	Uint32 (*fn)(void *p, Uint32 ival, void *arg);
	void *arg;
	int running;				/* Callback is executing */
	int flags;
#define TIMEOUT_DETACHABLE	0x01	/* Don't cancel in object_detach() */
#define TIMEOUT_LOADABLE	0x02	/* Don't cancel in object_load() */

	Uint32 ticks;				/* Expiry time in SDL ticks */
	Uint32 ival;				/* Interval in ticks */
	CIRCLEQ_ENTRY(timeout) timeouts;	/* Priority queue */
};

__BEGIN_DECLS
void		timeout_set(struct timeout *,
	                    Uint32 (*)(void *, Uint32, void *), void *, int);
void		timeout_add(void *, struct timeout *, Uint32);
int		timeout_scheduled(void *, struct timeout *);
void		timeout_del(void *, struct timeout *);
__inline__ void	timeout_replace(void *, struct timeout *, Uint32);
__inline__ void timeout_process(Uint32);
__inline__ void lock_timeout(void *);
__inline__ void unlock_timeout(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_TIMEOUT_H_ */
