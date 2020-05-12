/*	Public domain	*/

#ifndef _AGAR_CORE_OBJECT_H_
# error "Must be included by object.h"
#endif

#include <string.h>
#include <stdarg.h>

#ifndef AG_EVENT_ARGS_MAX
# if AG_MODEL == AG_SMALL
#  define AG_EVENT_ARGS_MAX 4
# else
#  define AG_EVENT_ARGS_MAX 7
# endif
#endif

#ifndef AG_EVENT_NAME_MAX
# if AG_MODEL == AG_SMALL
#  define AG_EVENT_NAME_MAX 14
# else
#  define AG_EVENT_NAME_MAX 32
# endif
#endif

/*
 * Argument accessor macros
 */
#ifdef AG_TYPE_SAFETY

# define AG_PTR(v) ((v <= event->argc && \
                    event->argv[v].type == AG_VARIABLE_POINTER && \
                    !(event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY)) ? \
                    event->argv[v].data.p : AG_PtrMismatch())

# define AG_CONST_PTR(v) ((v <= event->argc && \
                          event->argv[v].type == AG_VARIABLE_POINTER && \
                          (event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY)) ? \
                          (const void *)event->argv[v].data.p : \
			  (const void *)AG_PtrMismatch())

# define AG_STRING(v) ((v < event->argc && event->argv[v].type==AG_VARIABLE_STRING) ? event->argv[v].data.s : AG_StringMismatch())
# define AG_INT(v)    ((v < event->argc && event->argv[v].type==AG_VARIABLE_INT)    ? event->argv[v].data.i : AG_IntMismatch())
# define AG_UINT(v)   ((v < event->argc && event->argv[v].type==AG_VARIABLE_UINT)   ? event->argv[v].data.u : (Uint)AG_IntMismatch())
# define AG_LONG(v)   ((v < event->argc && event->argv[v].type==AG_VARIABLE_LONG)   ? event->argv[v].data.li : AG_LongMismatch())
# define AG_ULONG(v)  ((v < event->argc && event->argv[v].type==AG_VARIABLE_ULONG)  ? event->argv[v].data.uli : (Ulong)AG_LongMismatch())
# define AG_FLOAT(v)  ((v < event->argc && event->argv[v].type==AG_VARIABLE_FLOAT)  ? event->argv[v].data.flt : AG_FloatMismatch())
# define AG_DOUBLE(v) ((v < event->argc && event->argv[v].type==AG_VARIABLE_DOUBLE) ? event->argv[v].data.dbl : AG_DoubleMismatch())
#else /* !AG_TYPE_SAFETY */
# define AG_PTR(v)       (event->argv[v].data.p)
# define AG_CONST_PTR(v) (event->argv[v].data.p)
# define AG_STRING(v)    (event->argv[v].data.s)
# define AG_INT(v)       (event->argv[v].data.i)
# define AG_UINT(v)      (event->argv[v].data.u)
# define AG_LONG(v)      (event->argv[v].data.li)
# define AG_ULONG(v)     (event->argv[v].data.uli)
# define AG_FLOAT(v)     (event->argv[v].data.flt)
# define AG_DOUBLE(v)    (event->argv[v].data.dbl)
#endif /* !AG_TYPE_SAFETY */
#ifdef AG_UNICODE
# define AG_CHAR(v)      ((AG_Char)AG_ULONG(v))
#else
# define AG_CHAR(v)      ((AG_Char)AG_UINT(v))
#endif
#define AG_SELF()        AG_PTR(0)

#define AG_PTR_NAMED(k)       AG_GetNamedPtr(event,(k))
#define AG_CONST_PTR_NAMED(k) AG_GetNamedConstPtr(event,(k))
#define AG_STRING_NAMED(k)    AG_GetNamedString(event,(k))
#define AG_INT_NAMED(k)       AG_GetNamedInt(event,(k))
#define AG_UINT_NAMED(k)      AG_GetNamedUint(event,(k))
#define AG_LONG_NAMED(k)      AG_GetNamedLong(event,(k))
#define AG_ULONG_NAMED(k)     AG_GetNamedUlong(event,(k))
#define AG_FLOAT_NAMED(k)     AG_GetNamedFlt(event,(k))
#define AG_DOUBLE_NAMED(k)    AG_GetNamedDbl(event,(k))

struct ag_timer;
struct ag_event_sink;

/* Event handler / virtual function */
typedef struct ag_event {
	char name[AG_EVENT_NAME_MAX];		/* String identifier */
	AG_VoidFn fn;				/* Callback function */
#if AG_MODEL == AG_SMALL
	Uint8 argc, argc0;			/* Argument count & offset */
#else
	int   argc, argc0;			/* Argument count & offset */
#endif
	AG_Variable argv[AG_EVENT_ARGS_MAX];	/* Argument values */
	AG_TAILQ_ENTRY(ag_event) events;	/* Entry in Object */
} AG_Event, AG_Function;

#define AGEVENT(ev)    ((struct ag_event *)(ev))
#define AGFUNCTION(ev) ((struct ag_event *)(ev))

/* Low-level event sink */
enum ag_event_sink_type {
	AG_SINK_NONE,
	AG_SINK_PROLOGUE,		/* Special event loop prologue */
	AG_SINK_EPILOGUE,		/* Special event sink epilogue */
	AG_SINK_SPINNER,		/* Special non-blocking sink */
	AG_SINK_TERMINATOR,		/* Quit request */
	AG_SINK_TIMER,			/* Timer expiration */
	AG_SINK_READ,			/* Data available on fd */
	AG_SINK_WRITE,			/* Write buffer available on fd */
	AG_SINK_FSEVENT,		/* Filesystem event */
	AG_SINK_PROCEVENT,		/* Process event */
	AG_SINK_LAST
};

/* Event sink callback */
typedef int (*AG_EventSinkFn)(struct ag_event_sink *_Nonnull, AG_Event *_Nonnull);

/* Active low-level event sink */
typedef struct ag_event_sink {
	enum ag_event_sink_type type;		/* Event filter type */
	int ident;				/* Identifier / fd */
	Uint flags, flagsMatched;
#define AG_FSEVENT_DELETE	0x0001		/* Referenced file deleted */
#define AG_FSEVENT_WRITE	0x0002		/* A write has occurred */
#define AG_FSEVENT_EXTEND	0x0004		/* File extended */
#define AG_FSEVENT_ATTRIB	0x0008		/* File attributes changed */
#define AG_FSEVENT_LINK		0x0010		/* Link count changed */
#define AG_FSEVENT_RENAME	0x0020		/* Referenced file renamed */
#define AG_FSEVENT_REVOKE	0x0040		/* Filesystem unmount / revoke() */
#define AG_PROCEVENT_EXIT	0x1000		/* Process exited */
#define AG_PROCEVENT_FORK	0x2000		/* Process forked */
#define AG_PROCEVENT_EXEC	0x4000		/* Process exec'd */
	_Nonnull AG_EventSinkFn fn;		/* Sink function */
	AG_Event fnArgs;			/* Sink function arguments */
	AG_TAILQ_ENTRY(ag_event_sink) sinks;    /* Epilogue "sinks" */
} AG_EventSink;

/* Low-level event source */
typedef struct ag_event_source {
	Uint flags;
	int  breakReq;				/* Break from event loop */
	int  (*_Nonnull sinkFn)(void);
#ifdef AG_TIMERS
	int  (*_Nullable addTimerFn)(struct ag_timer *_Nonnull, Uint32, int);
	void (*_Nullable delTimerFn)(struct ag_timer *_Nonnull);
#endif
	AG_TAILQ_HEAD_(ag_event_sink) prologues;   /* Event prologues */
	AG_TAILQ_HEAD_(ag_event_sink) epilogues;   /* Event sink epilogues */
	AG_TAILQ_HEAD_(ag_event_sink) spinners;	   /* Spinning sinks */
	AG_TAILQ_HEAD_(ag_event_sink) sinks;	   /* Normal event sinks */

	int  returnCode;			/* AG_EventLoop() return code */
	Uint8 caps[AG_SINK_LAST];		/* Capabilities */
	Uint16 _pad;
} AG_EventSource;

typedef void (*AG_EventFn)(AG_Event *_Nonnull);

#if defined(AG_DEBUG) || defined(AG_TYPE_SAFETY)
# define AG_EVENT_PUSH_ARG_PRECOND(ev) \
	if ((ev)->argc >= AG_EVENT_ARGS_MAX) { AG_FatalError("AG_Event: Too many args"); }
# define AG_EVENT_POP_ARG_PRECOND(ev) \
	if ((ev)->argc < 1) { AG_FatalError("AG_Event: Pop without Push"); }
# define AG_EVENT_POP_ARG_POSTCOND(v, vtype) \
	if ((v)->type != (vtype)) { AG_FatalError("AG_Event: Illegal Pop type"); }
#else
# define AG_EVENT_PUSH_ARG_PRECOND(ev)
# define AG_EVENT_POP_ARG_PRECOND(ev)
# define AG_EVENT_POP_ARG_POSTCOND(v,vtype)
#endif

/*
 * Implementation of AG_EventPushTYPE() and AG_EventPopTYPE().
 */
#ifdef AG_THREADS
# define AG_EVENT_PUSH_FN(ev, tname, aname, member, val) {		\
	AG_EVENT_PUSH_ARG_PRECOND(ev)					\
	(ev)->argv[(ev)->argc].type = (tname);				\
	if ((aname) != NULL) {						\
		AG_Strlcpy((ev)->argv[(ev)->argc].name, (aname),	\
		        AG_VARIABLE_NAME_MAX);				\
	} else {							\
		(ev)->argv[(ev)->argc].name[0] = '\0';			\
	}								\
	(ev)->argv[(ev)->argc].mutex = NULL;				\
	(ev)->argv[(ev)->argc].data.member = (val);			\
	(ev)->argc++;							\
}
#else /* !AG_THREADS */
# define AG_EVENT_PUSH_FN(ev, tname, aname, member, val) {		\
	AG_EVENT_PUSH_ARG_PRECOND(ev)					\
	(ev)->argv[(ev)->argc].type = (tname);				\
	if ((aname) != NULL) {						\
		AG_Strlcpy((ev)->argv[(ev)->argc].name, (aname),	\
		        AG_VARIABLE_NAME_MAX);				\
	} else {							\
		(ev)->argv[(ev)->argc].name[0] = '\0';			\
	}								\
	(ev)->argv[(ev)->argc].data.member = (val);			\
	(ev)->argc++;							\
}
#endif /* !AG_THREADS */

#define AG_EVENT_POP_FN(vtype, memb)		\
	AG_Variable *V;				\
						\
	AG_EVENT_POP_ARG_PRECOND(ev)		\
	V = &ev->argv[ev->argc--];		\
	AG_EVENT_POP_ARG_POSTCOND(V, vtype)	\
	return (V->data.memb)

/*
 * Inline implementation of the varargs argument parser, AG_EVENT_GET_ARGS().
 * Used by AG_{Set,Add,Post}Event() and AG_EventArgs().
 */
#ifdef AG_THREADS
# define AG_EVENT_INS_ARG(eev, ap, tname, member, t)    \
	V = &(eev)->argv[(eev)->argc];			\
	AG_EVENT_PUSH_ARG_PRECOND(eev)			\
	V->type = (tname);				\
	V->mutex = NULL;				\
	V->data.member = va_arg(ap,t);			\
	(eev)->argc++
#else /* !AG_THREADS */
# define AG_EVENT_INS_ARG(eev, ap, tname, member, t)	\
	V = &(eev)->argv[(eev)->argc];			\
	AG_EVENT_PUSH_ARG_PRECOND(eev)			\
	V->type = (tname);				\
	V->data.member = va_arg(ap,t);			\
	(eev)->argc++
#endif /* !AG_THREADS */

#if AG_MODEL != AG_SMALL
# define AG_EVENT_PUSH_ARG_CASE_LONG(ev)					\
	  case 'i':								\
	    AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_LONG, li, long);		\
	    break;								\
	  case 'u':								\
	    AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_ULONG, uli, unsigned long);	\
	    break;
#else
# define AG_EVENT_PUSH_ARG_CASE_LONG(ev)
#endif

#ifdef AG_HAVE_FLOAT
# define AG_EVENT_PUSH_ARG_CASE_FLT(ev)					\
	case 'f':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_FLOAT, flt, double);	\
	  break;							\
	case 'd':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_DOUBLE, dbl, double);  \
	  break;
#else
# define AG_EVENT_PUSH_ARG_CASE_FLT(ev)
#endif

#ifdef AG_NAMED_ARGS
# define AG_EVENT_GET_NAMED_ARG()					\
	if (*c == '(' && c[1] != '\0') {				\
		char *cEnd;						\
		AG_Strlcpy(V->name, &c[1], sizeof(V->name));		\
		for (cEnd = V->name; *cEnd != '\0'; cEnd++) {		\
			if (*cEnd == ')') {				\
				*cEnd = '\0';				\
				c += 2;					\
				break;					\
			}						\
			c++;						\
		}							\
	} else {							\
		V->name[0] = '\0';					\
	}
#else
# define AG_EVENT_GET_NAMED_ARG() 					\
	{								\
		V->name[0] = '\0';					\
	}
#endif

#ifdef AG_VERBOSITY
# ifdef AG_DEBUG
#  define AG_EVENT_PARSER_ERROR AG_FatalErrorF("Bad AG_Event argument " \
                                               "(near `%%%s')", c)
# else
#  define AG_EVENT_PARSER_ERROR AG_FatalError("Bad AG_Event argument")
# endif
#else
# define AG_EVENT_PARSER_ERROR  AG_FatalError("E3")
#endif

#define AG_EVENT_PUSH_ARG(ap,ev) {					\
	AG_Variable *V = NULL;						\
									\
	switch (*c) {							\
	case 'p':							\
	  AG_EVENT_INS_ARG(ev, ap, AG_VARIABLE_POINTER, p, void *);	\
	  V->info.pFlags = 0;						\
	  break;							\
	case 'C':							\
	  if (c[1] == 'p') {						\
	  	AG_EVENT_INS_ARG(ev, ap, AG_VARIABLE_POINTER, p, void *); \
	  	V->info.pFlags = AG_VARIABLE_P_READONLY;		 \
		c++;							\
	  }								\
	  break;							\
	case 'i':							\
	  AG_EVENT_INS_ARG(ev, ap, AG_VARIABLE_INT, i, int);		\
	  break;							\
	case 'u':							\
	  AG_EVENT_INS_ARG(ev, ap, AG_VARIABLE_UINT, u, Uint);	\
	  break;							\
	AG_EVENT_PUSH_ARG_CASE_FLT(ev)					\
	case 's':							\
	  AG_EVENT_INS_ARG(ev, ap, AG_VARIABLE_STRING, s, char *);	\
	  break;							\
	case 'l':							\
	  switch (c[1]) {						\
	  AG_EVENT_PUSH_ARG_CASE_LONG(ev)				\
	  default:							\
	    AG_EVENT_PARSER_ERROR;					\
	  }								\
	  c++;								\
	  break;							\
	case ' ':							\
	case ',':							\
	case '%':							\
	  c++;								\
	  continue;							\
	default:							\
	  AG_EVENT_PARSER_ERROR;					\
	}								\
	c++;								\
	if (V != NULL) {						\
		AG_EVENT_GET_NAMED_ARG();				\
	}								\
}
#define AG_EVENT_GET_ARGS(ev, fmtp)					\
	if (fmtp != NULL) {						\
		const char *c = (const char *)fmtp;			\
		va_list ap;						\
									\
		va_start(ap, fmtp);					\
		while (*c != '\0') {					\
			AG_EVENT_PUSH_ARG(ap, (ev));			\
		}							\
		va_end(ap);						\
	}

__BEGIN_DECLS
void               AG_EventInit(AG_Event *_Nonnull);
void               AG_EventArgs(AG_Event *_Nonnull, const char *_Nullable , ...);
AG_Event *_Nonnull AG_EventNew(AG_EventFn, void *_Nonnull, const char *_Nullable,
                               ...);
#if AG_MODEL != AG_SMALL
AG_Event *_Nonnull AG_EventDup(const AG_Event *_Nonnull);
#endif
void               AG_EventCopy(AG_Event *_Nonnull, const AG_Event *_Nonnull);

AG_Event *_Nonnull AG_SetEvent(void *_Nonnull, const char *_Nullable ,
                               _Nullable AG_EventFn, const char *_Nullable, ...);
AG_Event *_Nonnull AG_AddEvent(void *_Nonnull, const char *_Nullable,
			       _Nullable AG_EventFn, const char *_Nullable, ...);
void               AG_UnsetEvent(void *_Nonnull, const char *_Nonnull);
void               AG_UnsetEventByPtr(void *_Nonnull, AG_Event *_Nonnull);
/*
 * TODO AG_AppendEvent(), AG_ReplaceEvent().
 */

void AG_PostEventByPtr(void *_Nonnull, AG_Event *_Nonnull,
                       const char *_Nullable, ...);
void AG_PostEvent(void *_Nonnull, const char *_Nonnull,
                  const char *_Nullable, ...);
void AG_EventGetArgs(AG_Event *_Nonnull, const char *_Nullable, va_list);
void AG_ForwardEvent(void *_Nonnull, const AG_Event *_Nonnull);

AG_Event *_Nullable AG_FindEventHandler(void *_Nonnull, const char *_Nonnull);

#ifdef AG_TIMERS
int AG_SchedEvent(void *_Nonnull, Uint32, const char *_Nullable,
                  const char *_Nullable, ...);
#endif

#ifdef AG_EVENT_LOOP
int                      AG_InitEventSubsystem(Uint);
void                     AG_DestroyEventSubsystem(void);
AG_EventSource *_Nonnull AG_GetEventSource(void);
AG_EventSink *_Nullable  AG_AddEventPrologue(_Nonnull AG_EventSinkFn, const char *_Nullable, ...);
AG_EventSink *_Nullable  AG_AddEventEpilogue(_Nonnull AG_EventSinkFn, const char *_Nullable, ...);
AG_EventSink *_Nullable  AG_AddEventSpinner(_Nonnull AG_EventSinkFn, const char *_Nullable, ...);
void                     AG_DelEventPrologue(AG_EventSink *_Nonnull);
void                     AG_DelEventEpilogue(AG_EventSink *_Nonnull);
void                     AG_DelEventSpinner(AG_EventSink *_Nonnull);
AG_EventSink *_Nullable  AG_AddEventSink(enum ag_event_sink_type, int, Uint,
                                         _Nonnull AG_EventSinkFn,
			 		 const char *_Nullable, ...);
void                     AG_DelEventSink(AG_EventSink *_Nonnull);
void                     AG_DelEventSinksByIdent(enum ag_event_sink_type, int, Uint);
int                      AG_EventLoop(void);
void                     AG_Terminate(int);
void                     AG_TerminateEv(AG_Event *_Nonnull);
# ifdef AG_TIMERS
int                      AG_AddTimerKQUEUE(struct ag_timer *_Nonnull, Uint32, int);
void                     AG_DelTimerKQUEUE(struct ag_timer *_Nonnull);
int                      AG_AddTimerTIMERFD(struct ag_timer *_Nonnull, Uint32, int);
void                     AG_DelTimerTIMERFD(struct ag_timer *_Nonnull);
# endif
int                      AG_EventSinkKQUEUE(void);
int                      AG_EventSinkTIMERFD(void);
int                      AG_EventSinkTIMEDSELECT(void);
int                      AG_EventSinkSELECT(void);
int                      AG_EventSinkSPINNER(void);
#endif /* AG_EVENT_LOOP */

/*
 * Inlinables
 */
#if AG_MODEL != AG_SMALL
void                  ag_event_push_pointer(AG_Event *_Nonnull, const char *_Nullable, void *_Nullable);
void                  ag_event_push_const_pointer(AG_Event *_Nonnull, const char *_Nullable, const void *_Nullable);
void                  ag_event_push_string(AG_Event *_Nonnull, const char *_Nullable, char *_Nonnull);
void                  ag_event_push_int(AG_Event *_Nonnull, const char *_Nullable, int);
void                  ag_event_push_uint(AG_Event *_Nonnull, const char *_Nullable, Uint);
void                  ag_event_push_long(AG_Event *_Nonnull, const char *_Nullable, long);
void                  ag_event_push_ulong(AG_Event *_Nonnull, const char *_Nullable, Ulong);
# ifdef AG_HAVE_FLOAT
void                  ag_event_push_float(AG_Event *_Nonnull, const char *_Nullable, float);
void                  ag_event_push_double(AG_Event *_Nonnull, const char *_Nullable, double);
# endif
void *_Nullable       ag_event_pop_pointer(AG_Event *_Nonnull);
const void *_Nullable ag_event_pop_const_pointer(AG_Event *_Nonnull);
char *_Nonnull        ag_event_pop_string(AG_Event *_Nonnull);
int                   ag_event_pop_int(AG_Event *_Nonnull);
Uint                  ag_event_pop_uint(AG_Event *_Nonnull);
long                  ag_event_pop_long(AG_Event *_Nonnull);
Ulong                 ag_event_pop_ulong(AG_Event *_Nonnull);
# ifdef AG_HAVE_FLOAT
float                 ag_event_pop_float(AG_Event *_Nonnull);
double                ag_event_pop_double(AG_Event *_Nonnull);
# endif
#endif /* !AG_SMALL */

#ifdef AG_NAMED_ARGS
AG_Variable *_Nonnull ag_get_named_event_arg(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
void *_Nullable       ag_get_named_ptr(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
const void *_Nullable ag_get_named_const_ptr(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
char *_Nonnull        ag_get_named_string(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
int                   ag_get_named_int(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
Uint                  ag_get_named_uint(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
# if AG_MODEL != AG_SMALL
long                  ag_get_named_long(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
Ulong                 ag_get_named_ulong(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
# endif
# ifdef AG_HAVE_FLOAT
float                 ag_get_named_flt(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
double                ag_get_named_dbl(AG_Event *_Nonnull, const char *_Nonnull) _Pure_Attribute;
# endif
#endif /* AG_NAMED_ARGS */

#ifdef AG_INLINE_EVENT
# define AG_INLINE_HEADER
# include <agar/core/inline_event.h>
#else
# define AG_EventPushPointer(e,n,v)      ag_event_push_pointer((e),(n),(v))
# define AG_EventPushConstPointer(e,n,v) ag_event_push_const_pointer((e),(n),(v))
# define AG_EventPushString(e,n,v)       ag_event_push_string((e),(n),(v))
# define AG_EventPushInt(e,n,v)          ag_event_push_int((e),(n),(v))
# define AG_EventPushUint(e,n,v)         ag_event_push_uint((e),(n),(v))
# define AG_EventPushLong(e,n,v)         ag_event_push_long((e),(n),(v))
# define AG_EventPushUlong(e,n,v)        ag_event_push_ulong((e),(n),(v))
# define AG_EventPushFloat(e,n,v)        ag_event_push_float((e),(n),(v))
# define AG_EventPushDouble(e,n,v)       ag_event_push_double((e),(n),(v))
# define AG_EventPopPointer(e)           ag_event_pop_pointer(e)
# define AG_EventPopConstPointer(e)      ag_event_pop_const_pointer(e)
# define AG_EventPopString(e)            ag_event_pop_string(e)
# define AG_EventPopInt(e)               ag_event_pop_int(e)
# define AG_EventPopUint(e)              ag_event_pop_uint(e)
# define AG_EventPopLong(e)              ag_event_pop_long(e)
# define AG_EventPopUlong(e)             ag_event_pop_ulong(e)
# define AG_EventPopFloat(e)             ag_event_pop_float(e)
# define AG_EventPopDouble(e)            ag_event_pop_double(e)
# define AG_GetNamedEventArg(e,n)        ag_get_named_event_arg((e),(n))
# define AG_GetNamedPtr(e,n)             ag_get_named_ptr((e),(n))
# define AG_GetNamedConstPtr(e,n)        ag_get_named_const_ptr((e),(n))
# define AG_GetNamedString(e,n)          ag_get_named_string((e),(n))
# define AG_GetNamedInt(e,n)             ag_get_named_int((e),(n))
# define AG_GetNamedUint(e,n)            ag_get_named_uint((e),(n))
# define AG_GetNamedLong(e,n)            ag_get_named_long((e),(n))
# define AG_GetNamedUlong(e,n)           ag_get_named_ulong((e),(n))
# define AG_GetNamedFlt(e,n)             ag_get_named_flt((e),(n))
# define AG_GetNamedDbl(e,n)             ag_get_named_dbl((e),(n))
#endif /* AG_INLINE_EVENT */
__END_DECLS
