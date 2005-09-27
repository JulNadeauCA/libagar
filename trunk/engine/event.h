/*	$Csoft: event.h,v 1.32 2005/09/20 13:46:29 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

#define AG_EVENT_ARGS_MAX	16
#define AG_EVENT_NAME_MAX	31

enum ag_evarg_type {
	AG_EVARG_POINTER,
	AG_EVARG_STRING,
	AG_EVARG_CHAR,
	AG_EVARG_UCHAR,
	AG_EVARG_INT,
	AG_EVARG_UINT,
	AG_EVARG_LONG,
	AG_EVARG_ULONG,
	AG_EVARG_FLOAT,
	AG_EVARG_DOUBLE,
	AG_EVARG_LAST
};

typedef union evarg {
	void	*p;
	char	*s;
	int	 i;
	long int li;
	double	 f;
} AG_EvArg;

typedef struct ag_event {
	char	name[AG_EVENT_NAME_MAX];
	u_int	flags;
#define	AG_EVENT_ASYNC		0x01	/* Event handler runs in own thread */
#define AG_EVENT_PROPAGATE	0x02	/* Relay event to object descendents */
#define AG_EVENT_SCHEDULED	0x04	/* Timing-dependent (read-only flag) */

	void (*handler)(int, union evarg *);
	int	 argc;
	int	 argc_base;
	AG_EvArg argv[AG_EVENT_ARGS_MAX];	/* Argument data vector */
	Uint8    argt[AG_EVENT_ARGS_MAX];	/* Argument types */
	AG_Timeout timeout;			/* Timer for scheduled events */

	TAILQ_ENTRY(ag_event) events;
} AG_Event;

__BEGIN_DECLS
void	AG_EventLoop_FixedFPS(void);
#define AG_EventLoop() AG_EventLoop_FixedFPS()

AG_Event	*AG_SetEvent(void *, const char *, void (*)(int, union evarg *),
		           const char *, ...);
AG_Event	*AG_AddEvent(void *, const char *, void (*)(int, union evarg *),
		           const char *, ...);
void		 AG_UnsetEvent(void *, const char *);
int		 AG_PostEvent(void *, void *, const char *, const char *, ...);
int		 AG_SchedEvent(void *, void *, Uint32, const char *,
		               const char *, ...);
int		 AG_ReschedEvent(void *, const char *, Uint32);
int		 AG_CancelEvent(void *, const char *);
__inline__ void	 AG_ExecEvent(void *, const char *);
void		 AG_ForwardEvent(void *, const char *, int, union evarg *);
__END_DECLS

#ifdef DEBUG

#define AG_EVENT_INSERT_ARG(eev, ap, tname, member, type) do {	\
	if ((eev)->argc >= AG_EVENT_ARGS_MAX-1) {		\
		fatal("excess evargs");				\
	}							\
	(eev)->argv[(eev)->argc].member = va_arg((ap), type);	\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)
#define AG_EVENT_INSERT_VAL(eev, tname, member, val) do {	\
	if ((eev)->argc >= AG_EVENT_ARGS_MAX-1) {		\
		fatal("excess evargs");				\
	}							\
	(eev)->argv[(eev)->argc].member = (val);		\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)

#else /* !DEBUG */

#define AG_EVENT_INSERT_ARG(eev, ap, tname, member, type) do {	\
	(eev)->argv[(eev)->argc].member = va_arg((ap), type);	\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)
#define AG_EVENT_INSERT_VAL(eev, tname, member, val) do {	\
	(eev)->argv[(eev)->argc].member = (val);		\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argc++;						\
} while (0)

#endif /* DEBUG */

#define AG_EVENT_PUSH_ARG(ap, fmt, eev)					\
	switch (fmt) {							\
	case 'p':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_POINTER, p, void *); \
		break;							\
	case 'i':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_INT, i, int);	\
		break;							\
	case 'u':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_UINT, i, int); \
		break;							\
	case 'f':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_FLOAT, f, double); \
		break;							\
	case 'd':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_DOUBLE, f, double); \
		break;							\
	case 's':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_STRING, s, char *); \
		break;							\
	case 'c':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_UCHAR, i, int);  \
		break;							\
	case 'C':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_CHAR, i, int); \
		break;							\
	case 'U':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_ULONG, li, long int);\
		break;							\
	case 'D':							\
		AG_EVENT_INSERT_ARG((eev), (ap), AG_EVARG_LONG, li, long int); \
		break;							\
	case ' ':							\
	case ',':							\
	case '%':							\
		break;							\
	default:							\
		fatal("bad evarg spec");				\
	}

#include "close_code.h"
