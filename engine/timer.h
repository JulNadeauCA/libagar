/*	$Csoft: timer.h,v 1.1 2003/01/19 12:11:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TIMER_H_
#define _AGAR_TIMER_H_
#include "begin_code.h"

struct timer {
	struct object	 obj;
	enum {
		TIMER_RUNNING,
		TIMER_STOP,
		TIMER_STOPPED
	} state;
	SDL_TimerID	 id;
	Uint32		 ival;			/* Interval in milliseconds */
	Uint32		(*callback)(Uint32 ival, void *arg);
	void		 *arg;

	pthread_mutex_t	 lock;
};

__BEGIN_DECLS
extern DECLSPEC void		 timer_init(struct timer *, const char *,
				            Uint32, Uint32 (*)(Uint32, void *),
					    void *);
extern DECLSPEC void		 timer_start(struct timer *);
extern DECLSPEC void		 timer_stop(struct timer *);
extern DECLSPEC struct timer	*timer_new(const char *, Uint32,
				           Uint32 (*)(Uint32, void *), void *);
extern DECLSPEC void		 timer_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TIMER_H_ */
