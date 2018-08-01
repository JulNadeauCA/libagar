/*	Public domain	*/

#include <agar/core/begin.h>

#ifndef AG_EVENT_ARGS_MAX
#define AG_EVENT_ARGS_MAX 8
#endif
#ifndef AG_EVENT_NAME_MAX
#define AG_EVENT_NAME_MAX 32
#endif

#ifdef AG_TYPE_SAFETY
# define AG_OBJECT(v,t)    ((v < event->argc && event->argv[v].type==AG_VARIABLE_POINTER \
                            && AG_OfClass(event->argv[v].data.p, (t))) ? \
			    event->argv[v].data.p : AG_ObjectMismatch())
# define AG_PTR(v)         ((v < event->argc && event->argv[v].type==AG_VARIABLE_POINTER) ? event->argv[v].data.p   : AG_PtrMismatch())
# define AG_STRING(v)      ((v < event->argc && event->argv[v].type==AG_VARIABLE_STRING)  ? event->argv[v].data.s   : AG_StringMismatch())
# define AG_INT(v)         ((v < event->argc && event->argv[v].type==AG_VARIABLE_INT)     ? event->argv[v].data.i   : AG_IntMismatch())
# define AG_UINT(v)        ((v < event->argc && event->argv[v].type==AG_VARIABLE_UINT)    ? event->argv[v].data.u   : (Uint)AG_IntMismatch())
# define AG_LONG(v)        ((v < event->argc && event->argv[v].type==AG_VARIABLE_LONG)    ? event->argv[v].data.li  : AG_LongMismatch())
# define AG_ULONG(v)       ((v < event->argc && event->argv[v].type==AG_VARIABLE_ULONG)   ? event->argv[v].data.uli : (Ulong)AG_LongMismatch())
# define AG_FLOAT(v)       ((v < event->argc && event->argv[v].type==AG_VARIABLE_FLOAT)   ? event->argv[v].data.flt : AG_FloatMismatch())
# define AG_DOUBLE(v)      ((v < event->argc && event->argv[v].type==AG_VARIABLE_DOUBLE)  ? event->argv[v].data.dbl : AG_DoubleMismatch())
# ifdef AG_HAVE_LONG_DOUBLE
# define AG_LONG_DOUBLE(v) ((v < event->argc && event->argv[v].type==AG_VARIABLE_LONG_DOUBLE) ? event->argv[v].data.ldbl : AG_LongDoubleMismatch())
# endif
#else
# define AG_PTR(v)         (event->argv[v].data.p)
# define AG_OBJECT(v, t)   (event->argv[v].data.p)
# define AG_STRING(v)      (event->argv[v].data.s)
# define AG_INT(v)         (event->argv[v].data.i)
# define AG_UINT(v)        (event->argv[v].data.u)
# define AG_LONG(v)        (event->argv[v].data.li)
# define AG_ULONG(v)       (event->argv[v].data.uli)
# define AG_FLOAT(v)       (event->argv[v].data.flt)
# define AG_DOUBLE(v)      (event->argv[v].data.dbl)
# ifdef AG_HAVE_LONG_DOUBLE
# define AG_LONG_DOUBLE(v) (event->argv[v].data.ldbl)
# endif
#endif /* AG_TYPE_SAFETY */

#define AG_SELF()	AG_PTR(0)
#define AG_SENDER()	AG_PTR(event->argc)

#define AG_PTR_NAMED(k)		AG_GetNamedPtr(event,(k))
#define AG_OBJECT_NAMED(k,cls)	AG_GetNamedObject(event,(k),(cls))
#define AG_STRING_NAMED(k)	AG_GetNamedString(event,(k))
#define AG_INT_NAMED(k)		AG_GetNamedInt(event,(k))
#define AG_UINT_NAMED(k)	AG_GetNamedUint(event,(k))
#define AG_LONG_NAMED(k)	AG_GetNamedLong(event,(k))
#define AG_ULONG_NAMED(k)	AG_GetNamedUlong(event,(k))
#define AG_FLOAT_NAMED(k)	AG_GetNamedFlt(event,(k))
#define AG_DOUBLE_NAMED(k)	AG_GetNamedDbl(event,(k))
#ifdef AG_HAVE_LONG_DOUBLE
#define AG_LONG_DOUBLE_NAMED(k) AG_GetNamedLongDbl(event,(k))
#endif

struct ag_timer;
struct ag_event_sink;

/* Event handler / virtual function */
typedef struct ag_event {
	char name[AG_EVENT_NAME_MAX];		/* String identifier */
	Uint flags;
#define	AG_EVENT_ASYNC     0x01			/* Service in separate thread */
#define AG_EVENT_PROPAGATE 0x02			/* Forward to child objs */
	union ag_function fn;			/* Callback function */
	int argc, argc0;			/* Argument count & offset */
	AG_Variable argv[AG_EVENT_ARGS_MAX];	/* Argument values */
	AG_TAILQ_ENTRY(ag_event) events;	/* Entry in Object */
} AG_Event, AG_Function;

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

typedef int (*AG_EventSinkFn)(struct ag_event_sink *, AG_Event *);

typedef struct ag_event_sink {
	enum ag_event_sink_type type;		/* Event filter type */
	int ident;				/* Identifier / fd */
	Uint flags, flagsMatched;
#define AG_FSEVENT_DELETE	0x0001		/* Referenced file deleted */
#define AG_FSEVENT_WRITE	0x0002		/* Write occured */
#define AG_FSEVENT_EXTEND	0x0004		/* File extended */
#define AG_FSEVENT_ATTRIB	0x0008		/* File attributes changed */
#define AG_FSEVENT_LINK		0x0010		/* Link count changed */
#define AG_FSEVENT_RENAME	0x0020		/* Referenced file renamed */
#define AG_FSEVENT_REVOKE	0x0040		/* Filesystem unmount / revoke() */
#define AG_PROCEVENT_EXIT	0x1000		/* Process exited */
#define AG_PROCEVENT_FORK	0x2000		/* Process forked */
#define AG_PROCEVENT_EXEC	0x4000		/* Process exec'd */
	AG_EventSinkFn fn;			/* Sink function */
	AG_Event fnArgs;			/* Sink function arguments */
	AG_TAILQ_ENTRY(ag_event_sink) sinks;    /* Epilogue "sinks" */
} AG_EventSink;

/* Low-level event source */
typedef struct ag_event_source {
	int  caps[AG_SINK_LAST];		/* Capabilities */
	Uint flags;
	int  breakReq;				/* Break from event loop */
	int  returnCode;			/* AG_EventLoop() return code */
	int  (*sinkFn)(void);
	int  (*addTimerFn)(struct ag_timer *, Uint32, int);
	void (*delTimerFn)(struct ag_timer *);
	int  (*resetTimerFn)(struct ag_timer *, Uint32, int);
	AG_TAILQ_HEAD_(ag_event_sink) prologues;   /* Event prologues */
	AG_TAILQ_HEAD_(ag_event_sink) epilogues;   /* Event sink epilogues */
	AG_TAILQ_HEAD_(ag_event_sink) spinners;	   /* Spinning sinks */
	AG_TAILQ_HEAD_(ag_event_sink) sinks;	   /* Normal event sinks */
} AG_EventSource;

/* Queue of events */
typedef struct ag_event_queue {
	Uint     nEvents;
	AG_Event *events;
} AG_EventQ;

typedef void (*AG_EventFn)(AG_Event *);

#if defined(AG_DEBUG) || defined(AG_TYPE_SAFETY)
# define AG_EVENT_PUSH_ARG_PRECOND(ev) \
	if ((ev)->argc >= AG_EVENT_ARGS_MAX-1) { AG_FatalError("AG_Event: Too many args"); }
# define AG_EVENT_POP_ARG_PRECOND(ev) \
	if ((ev)->argc < 1) { AG_FatalError("AG_Event: Pop without Push"); }
# define AG_EVENT_POP_ARG_POSTCOND(V, vtype) \
	if ((V)->type != (vtype)) { AG_FatalError("AG_Event: Illegal Pop type"); }
#else
# define AG_EVENT_PUSH_ARG_PRECOND(ev)
# define AG_EVENT_POP_ARG_PRECOND(ev)
# define AG_EVENT_POP_ARG_POSTCOND(v,vtype)
#endif

/*
 * Implementation of AG_EventPushTYPE() and AG_EventPopTYPE().
 */
#define AG_EVENT_PUSH_FN(ev, tname, aname, member, val) {		\
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
	(ev)->argv[(ev)->argc].fn.fnVoid = NULL;			\
	(ev)->argc++;							\
}
#define AG_EVENT_POP_FN(vtype, memb)		\
	AG_Variable *V;				\
	AG_EVENT_POP_ARG_PRECOND(ev)		\
	V = &ev->argv[ev->argc--];		\
	AG_EVENT_POP_ARG_POSTCOND(V, vtype)	\
	return (V->data.memb)

/*
 * Inline implementation of the varargs argument parser, AG_EVENT_GET_ARGS().
 * Used by AG_{Set,Add,Post}Event() and AG_EventArgs().
 */
#define AG_EVENT_INS_ARG(eev, ap, tname, member, t) {	\
	V = &(eev)->argv[(eev)->argc];			\
	AG_EVENT_PUSH_ARG_PRECOND(eev)			\
	V->type = (tname);				\
	V->mutex = NULL;				\
	V->data.member = va_arg(ap,t);			\
	V->fn.fnVoid = NULL;				\
	(eev)->argc++;					\
}
#ifdef AG_HAVE_LONG_DOUBLE
# define AG_EVENT_PUSH_ARG_CASE_LDBL(ev)				\
	  case 'd':							\
	    AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_LONG_DOUBLE, ldbl,	\
	        long double);						\
	    break;
#else
# define AG_EVENT_PUSH_ARG_CASE_LDBL(ev)
#endif
#define AG_EVENT_PUSH_ARG(ap,ev) {					\
	AG_Variable *V;							\
	switch (*c) {							\
	case 'p':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_POINTER, p, void *);	\
	  break;							\
	case 'i':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_INT, i, int);		\
	  break;							\
	case 'u':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_UINT, u, Uint);	\
	  break;							\
	case 'f':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_FLOAT, flt, double);	\
	  break;							\
	case 'd':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_DOUBLE, dbl, double);  \
	  break;							\
	case 's':							\
	  AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_STRING, s, char *);	\
	  break;							\
	case 'l':							\
	  switch (c[1]) {						\
	  case 'i':							\
	    AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_LONG, li, long);	\
	    break;							\
	  case 'u':							\
	    AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_ULONG, uli,		\
	        unsigned long);						\
	    break;							\
	  AG_EVENT_PUSH_ARG_CASE_LDBL(ev)				\
	  default:							\
	    AG_FatalError("AG_Event: Bad format (l[iud]?)");		\
	  }								\
	  c++;								\
	  break;							\
	case 'C':							\
	  switch (c[1]) {						\
	  case 's':							\
	    AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_CONST_STRING, Cs,	\
	        const char *);						\
	    break;							\
	  case 'p':							\
	    AG_EVENT_INS_ARG((ev), ap, AG_VARIABLE_CONST_POINTER, Cp,	\
	        const void *);						\
	    break;							\
	  default:							\
	    AG_FatalError("AG_Event: Bad format (C[sp]?)");		\
	  }								\
	  c++;								\
	  break;							\
	case ' ':							\
	case ',':							\
	case '%':							\
	  c++;								\
	  continue;							\
	default:							\
	  AG_FatalError("AG_Event: Bad format");			\
	}								\
	c++;								\
	if (*c == '(' && c[1] != '\0') {				\
		char *cEnd;						\
		AG_Strlcpy(V->name, &c[1], sizeof(V->name));		\
		for (cEnd = V->name; *cEnd != '\0'; cEnd++) {		\
			if (*cEnd == ')') {				\
				*cEnd = '\0';				\
				c+=2;					\
				break;					\
			}						\
			c++;						\
		}							\
	} else {							\
		V->name[0] = '\0';					\
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
int       AG_InitEventSubsystem(Uint);
void      AG_DestroyEventSubsystem(void);
void      AG_EventInit(AG_Event *);
void      AG_EventArgs(AG_Event *, const char *, ...);

AG_Event *AG_SetEvent(void *, const char *, AG_EventFn, const char *, ...);
AG_Event *AG_AddEvent(void *, const char *, AG_EventFn, const char *, ...);

AG_Function *AG_SetVoidFn(void *, const char *, AG_VoidFn, const char *, ...);
AG_Function *AG_SetIntFn(void *, const char *, AG_IntFn, const char *, ...);
AG_Function *AG_SetUintFn(void *, const char *, AG_UintFn, const char *, ...);
AG_Function *AG_SetLongFn(void *, const char *, AG_LongFn, const char *, ...);
AG_Function *AG_SetUlongFn(void *, const char *, AG_UlongFn, const char *, ...);
AG_Function *AG_SetUint8Fn(void *, const char *, AG_Uint8Fn, const char *, ...);
AG_Function *AG_SetSint8Fn(void *, const char *, AG_Sint8Fn, const char *, ...);
AG_Function *AG_SetUint16Fn(void *, const char *, AG_Uint16Fn, const char *, ...);
AG_Function *AG_SetSint16Fn(void *, const char *, AG_Sint16Fn, const char *, ...);
AG_Function *AG_SetUint32Fn(void *, const char *, AG_Uint32Fn, const char *, ...);
AG_Function *AG_SetSint32Fn(void *, const char *, AG_Sint32Fn, const char *, ...);
#ifdef AG_HAVE_64BIT
AG_Function *AG_SetUint64Fn(void *, const char *, AG_Uint64Fn, const char *, ...);
AG_Function *AG_SetSint64Fn(void *, const char *, AG_Sint64Fn, const char *, ...);
#endif
AG_Function *AG_SetFloatFn(void *, const char *, AG_FloatFn, const char *, ...);
AG_Function *AG_SetDoubleFn(void *, const char *, AG_DoubleFn, const char *, ...);
#ifdef AG_HAVE_LONG_DOUBLE
AG_Function *AG_SetLongDoubleFn(void *, const char *, AG_LongDoubleFn, const char *, ...);
#endif
AG_Function *AG_SetStringFn(void *, const char *, AG_StringFn, const char *, ...);
AG_Function *AG_SetPointerFn(void *, const char *, AG_PointerFn, const char *, ...);
AG_Function *AG_SetConstPointerFn(void *, const char *, AG_ConstPointerFn, const char *, ...);

void      AG_UnsetEvent(void *, const char *);
void      AG_PostEvent(void *, void *, const char *, const char *, ...);
void      AG_PostEventByPtr(void *, void *, AG_Event *, const char *, ...);
AG_Event *AG_FindEventHandler(void *, const char *);

void      AG_InitEventQ(AG_EventQ *);
void      AG_FreeEventQ(AG_EventQ *);
void      AG_QueueEvent(AG_EventQ *, const char *, const char *, ...);
int       AG_SchedEvent(void *, void *, Uint32, const char *, const char *, ...);
void      AG_ForwardEvent(void *, void *, AG_Event *);

AG_EventSource *AG_GetEventSource(void);
AG_EventSink   *AG_AddEventPrologue(AG_EventSinkFn, const char *, ...);
AG_EventSink   *AG_AddEventEpilogue(AG_EventSinkFn, const char *, ...);
AG_EventSink   *AG_AddEventSpinner(AG_EventSinkFn, const char *, ...);
AG_EventSink   *AG_AddEventSink(enum ag_event_sink_type, int, Uint,
                                AG_EventSinkFn, const char *, ...);

int  AG_EventLoop(void);
void AG_DelEventPrologue(AG_EventSink *);
void AG_DelEventEpilogue(AG_EventSink *);
void AG_DelEventSpinner(AG_EventSink *);
void AG_DelEventSink(AG_EventSink *);
void AG_DelEventSinksByIdent(enum ag_event_sink_type, int, Uint);
void AG_Terminate(int);
void AG_TerminateEv(AG_Event *);
int  AG_AddTimerKQUEUE(struct ag_timer *, Uint32, int);
void AG_DelTimerKQUEUE(struct ag_timer *);
int  AG_AddTimerTIMERFD(struct ag_timer *, Uint32, int);
void AG_DelTimerTIMERFD(struct ag_timer *);
int  AG_EventSinkKQUEUE(void);
int  AG_EventSinkTIMERFD(void);
int  AG_EventSinkTIMEDSELECT(void);
int  AG_EventSinkSELECT(void);
int  AG_EventSinkSPINNER(void);

static __inline__ void AG_EventPushPointer(AG_Event *ev, const char *name, void *val) { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_POINTER, name, p, val); }
static __inline__ void AG_EventPushString(AG_Event *ev, const char *name, char *val)  { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_STRING, name, s, val); }
static __inline__ void AG_EventPushInt(AG_Event *ev, const char *name, int val)       { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_INT, name, i, val); }
static __inline__ void AG_EventPushUint(AG_Event *ev, const char *name, Uint val)     { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_UINT, name, i, val); }
static __inline__ void AG_EventPushLong(AG_Event *ev, const char *name, long val)     { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_LONG, name, li, val); }
static __inline__ void AG_EventPushUlong(AG_Event *ev, const char *name, Ulong val)   { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_ULONG, name, uli, val); }
static __inline__ void AG_EventPushFloat(AG_Event *ev, const char *name, float val)   { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_FLOAT, name, flt, val); }
static __inline__ void AG_EventPushDouble(AG_Event *ev, const char *name, double val) { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_DOUBLE, name, dbl, val); }
#ifdef AG_HAVE_LONG_DOUBLE
static __inline__ void AG_EventPushLongDouble(AG_Event *ev, const char *name, long double val) { AG_EVENT_PUSH_FN(ev, AG_VARIABLE_LONG_DOUBLE, name, ldbl, val); }
#endif
static __inline__ void  *AG_EventPopPointer(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_POINTER, p); }
static __inline__ char  *AG_EventPopString(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_STRING, s); }
static __inline__ int    AG_EventPopInt(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_INT, i); }
static __inline__ Uint   AG_EventPopUint(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_UINT, u); }
static __inline__ long   AG_EventPopLong(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_LONG, li); }
static __inline__ Ulong  AG_EventPopUlong(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_ULONG, uli); }
static __inline__ float  AG_EventPopFloat(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_FLOAT, flt); }
static __inline__ double AG_EventPopDouble(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_DOUBLE, dbl); }
#ifdef AG_HAVE_LONG_DOUBLE
static __inline__ long double AG_EventPopLongDouble(AG_Event *ev) { AG_EVENT_POP_FN(AG_VARIABLE_LONG_DOUBLE, ldbl); }
#endif

#undef AG_EVENT_PUSH_FN
#undef AG_EVENT_POP_FN

/*
 * Extract Event argument by name (case-insensitive).
 */
static __inline__ AG_Variable *
AG_GetNamedEventArg(AG_Event *ev, const char *name)
{
	int i;

	for (i = 0; i < ev->argc; i++) {
		if (AG_Strcasecmp(ev->argv[i].name, name) == 0)
			return (&ev->argv[i]);
	}
	AG_SetError("Illegal AG_*_NAMED() access: No \"%s\"", name);
	AG_FatalError(NULL);
	return (NULL);
}
static __inline__ void *
AG_GetNamedPtr(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_POINTER)
		AG_FatalError("Illegal AG_PTR_NAMED() access");
#endif
	return (V->data.p);
}
static __inline__ char *
AG_GetNamedString(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_STRING)
		AG_FatalError("Illegal AG_STRING_NAMED() access");
#endif
	return (V->data.s);
}
static __inline__ int
AG_GetNamedInt(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_INT) { AG_FatalError("Illegal AG_INT_NAMED() access"); }
#endif
	return (V->data.i);
}
static __inline__ Uint
AG_GetNamedUint(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_UINT) { AG_FatalError("Illegal AG_UINT_NAMED() access"); }
#endif
	return (V->data.u);
}
static __inline__ long
AG_GetNamedLong(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_LONG) { AG_FatalError("Illegal AG_LONG_NAMED() access"); }
#endif
	return (V->data.li);
}
static __inline__ Ulong
AG_GetNamedUlong(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_ULONG) { AG_FatalError("Illegal AG_ULONG_NAMED() access"); }
#endif
	return (V->data.uli);
}

static __inline__ float
AG_GetNamedFlt(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_FLOAT) { AG_FatalError("Illegal AG_FLOAT_NAMED() access"); }
#endif
	return (V->data.flt);
}
static __inline__ double
AG_GetNamedDbl(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_DOUBLE) { AG_FatalError("Illegal AG_DOUBLE_NAMED() access"); }
#endif
	return (V->data.dbl);
}
#ifdef AG_HAVE_LONG_DOUBLE
static __inline__ long double
AG_GetNamedLongDbl(AG_Event *event, const char *name)
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
# ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_LONG_DOUBLE) { AG_FatalError("Illegal AG_LONG_DOUBLE_NAMED() access"); }
# endif
	return (V->data.ldbl);
}
#endif /* AG_HAVE_LONG_DOUBLE */
__END_DECLS

#include <agar/core/close.h>
