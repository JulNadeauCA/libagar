/*	$Csoft: world.h,v 1.23 2002/11/27 05:05:30 vedge Exp $	*/
/*	Public domain	*/

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

void		 timer_init(struct timer *, const char *, Uint32,
		     Uint32 (*)(Uint32, void *), void *);
void		 timer_start(struct timer *);
void		 timer_stop(struct timer *);
struct timer	*timer_new(const char *, Uint32,
		     Uint32 (*)(Uint32, void *), void *);
void		 timer_destroy(void *);

