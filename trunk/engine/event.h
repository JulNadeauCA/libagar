/*	$Csoft: event.h,v 1.25 2004/05/06 06:20:08 vedge Exp $	*/
/*	Public domain	*/

#include <config/floating_point.h>

#include "begin_code.h"

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

#define EVENT_ARGS_MAX		16
#define EVENT_NAME_MAX		32
#define EVENTSEQ_NAME_MAX	16

struct event {
	char	name[EVENT_NAME_MAX];		/* Event type */
	int	flags;
#define	EVENT_ASYNC	0x01	/* Event handler runs in own thread */
#define EVENT_PROPAGATE	0x02	/* Propagate event to descendents */
#define EVENT_REPEAT	0x04	/* Reschedule after handler execution */

	Uint32	start;		/* Scheduled start time relative to the
				   sequence start (for real-time events) */

	void		(*handler)(int, union evarg *);
	union evarg	 argv[EVENT_ARGS_MAX];
	int		 argc;

	TAILQ_ENTRY(event) events;
};

extern int event_idle;		/* Enable idling? */

__BEGIN_DECLS
struct event	*event_new(void *, const char *, void (*)(int, union evarg *),
		           const char *, ...);
void		 event_remove(void *, const char *);
void		 event_loop(void);
int		 event_post(void *, void *, const char *, const char *, ...);
void		 event_forward(void *, const char *, int, union evarg *);
#ifdef DEBUG
struct window	*event_fps_window(void);
#endif
__END_DECLS

#define EVENT_INSERT_ARG(eev, ap, member, type) do {		\
	if ((eev)->argc >= EVENT_ARGS_MAX-1) {			\
		fatal("excess evargs");				\
	}							\
	(eev)->argv[(eev)->argc++].member = va_arg((ap), type);	\
} while (0)

#define EVENT_PUSH_ARG(ap, fmt, eev)				\
	switch ((fmt)) {					\
	case 'i':						\
	case 'o':						\
	case 'u':						\
	case 'x':						\
	case 'X':						\
		EVENT_INSERT_ARG((eev), (ap), i, int);		\
		break;						\
	case 'D':						\
	case 'O':						\
	case 'U':						\
		EVENT_INSERT_ARG((eev), (ap), li, long int);	\
		break;						\
	case 'e':						\
	case 'E':						\
	case 'f':						\
	case 'g':						\
	case 'G':						\
		EVENT_INSERT_ARG((eev), (ap), f, double);	\
		break;						\
	case 'c':						\
		EVENT_INSERT_ARG((eev), (ap), c, int);		\
		break;						\
	case 's':						\
		EVENT_INSERT_ARG((eev), (ap), s, char *);	\
		break;						\
	case 'p':						\
		EVENT_INSERT_ARG((eev), (ap), p, void *);	\
		break;						\
	case ' ':						\
	case ',':						\
	case ';':						\
	case '%':						\
		break;						\
	default:						\
		fatal("bad evarg spec");			\
	}

#include "close_code.h"
