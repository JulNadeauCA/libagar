/*	$Csoft: event.h,v 1.31 2005/06/11 12:05:45 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

#define EVENT_ARGS_MAX	16
#define EVENT_NAME_MAX	31

enum evarg_type {
	EVARG_POINTER,
	EVARG_STRING,
	EVARG_CHAR,
	EVARG_UCHAR,
	EVARG_INT,
	EVARG_UINT,
	EVARG_LONG,
	EVARG_ULONG,
	EVARG_FLOAT,
	EVARG_DOUBLE,
	EVARG_LAST
};

typedef union evarg {
	void	*p;
	char	*s;
	int	 i;
	long int li;
	double	 f;
} *evargs;

struct event {
	char	name[EVENT_NAME_MAX];
	u_int	flags;
#define	EVENT_ASYNC	0x01	/* Event handler runs in own thread */
#define EVENT_PROPAGATE	0x02	/* Propagate event to object descendents */
#define EVENT_SCHEDULED	0x04	/* Timing-dependent (read-only flag) */

	void		(*handler)(int, union evarg *);
	int		 argc, argc_base;
	union evarg	 argv[EVENT_ARGS_MAX];	/* Argument data vector */
	Uint8		 argt[EVENT_ARGS_MAX];	/* Argument types */
	struct timeout	 timeout;		/* Timer for scheduled events */

	TAILQ_ENTRY(event) events;
};

__BEGIN_DECLS
struct event	*event_new(void *, const char *, void (*)(int, union evarg *),
		           const char *, ...);
struct event	*event_add(void *, const char *, void (*)(int, union evarg *),
		           const char *, ...);
void		 event_remove(void *, const char *);
void		 event_loop(void);
int		 event_post(void *, void *, const char *, const char *, ...);
int		 event_schedule(void *, void *, Uint32, const char *,
		                const char *, ...);
int		 event_resched(void *, const char *, Uint32);
int		 event_cancel(void *, const char *);
__inline__ void	 event_execute(void *, const char *);
void		 event_forward(void *, const char *, int, union evarg *);
#ifdef DEBUG
struct window	*event_fps_window(void);
#endif
__END_DECLS

#ifdef DEBUG

#define EVENT_INSERT_ARG(eev, ap, tname, member, type) do {	\
	if ((eev)->argc >= EVENT_ARGS_MAX-1) {			\
		fatal("excess evargs");				\
	}							\
	(eev)->argv[(eev)->argc].member = va_arg((ap), type);	\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)
#define EVENT_INSERT_VAL(eev, tname, member, val) do {		\
	if ((eev)->argc >= EVENT_ARGS_MAX-1) {			\
		fatal("excess evargs");				\
	}							\
	(eev)->argv[(eev)->argc].member = (val);		\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)

#else /* !DEBUG */

#define EVENT_INSERT_ARG(eev, ap, tname, member, type) do {	\
	(eev)->argv[(eev)->argc].member = va_arg((ap), type);	\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)
#define EVENT_INSERT_VAL(eev, tname, member, val) do {		\
	(eev)->argv[(eev)->argc].member = (val);		\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)

#endif /* DEBUG */

#define EVENT_PUSH_ARG(ap, fmt, eev)					\
	switch (fmt) {							\
	case 'p':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_POINTER, p, void *); \
		break;							\
	case 'i':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_INT, i, int);	\
		break;							\
	case 'u':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_UINT, i, int);	\
		break;							\
	case 'f':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_FLOAT, f, double);	\
		break;							\
	case 'd':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_DOUBLE, f, double);	\
		break;							\
	case 's':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_STRING, s, char *);	\
		break;							\
	case 'c':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_UCHAR, i, int);	\
		break;							\
	case 'C':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_CHAR, i, int);	\
		break;							\
	case 'U':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_ULONG, li, long int); \
		break;							\
	case 'D':							\
		EVENT_INSERT_ARG((eev), (ap), EVARG_LONG, li, long int); \
		break;							\
	case ' ':							\
	case ',':							\
	case '%':							\
		break;							\
	default:							\
		fatal("bad evarg spec");				\
	}

#include "close_code.h"
