/*	$Csoft: event.h,v 1.14 2003/03/02 01:00:13 vedge Exp $	*/
/*	Public domain	*/

#include <config/floating_point.h>

typedef union evarg {
	void	*p;
	char	*s;
	int	 i;
	char	 c;
	long int li;
#ifdef FLOATING_POINT
	double	 f;
#endif
} *evargs;

#define EVENT_MAX_ARGS	16

struct event {
	char		*name;
	int		 flags;
#define	EVENT_ASYNC	0x01	/* Event handler runs in own thread */
	union evarg	 argv[EVENT_MAX_ARGS];
	int		 argc;
	void		(*handler)(int, union evarg *);
	TAILQ_ENTRY(event) events;
};

void		 event_loop(void);
struct event	*event_new(void *, char *, void (*)(int, union evarg *),
		     const char *, ...);
void		 event_post(void *, char *, const char *fmt, ...);
void		 event_forward(void *, char *, int, union evarg *);
#ifdef DEBUG
struct window	*event_show_fps_counter(void);
#endif

extern int	 event_idle;

