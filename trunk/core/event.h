/*	Public domain	*/

#include "begin_code.h"

#define AG_EVENT_ARGS_MAX 16
#define AG_EVENT_NAME_MAX 32

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

/* Argument to event handler */
typedef union evarg {
	void	*p;
	char	*s;
	int	 i;
	long int li;
	double	 f;
} AG_EvArg;

#ifdef DEBUG
#define AG_PTR(v) (event->argt[v]==AG_EVARG_POINTER?event->argv[v].p:\
    AG_PtrMismatch())
#define AG_STRING(v) (event->argt[v]==AG_EVARG_STRING?event->argv[v].s:\
    (char *)AG_PtrMismatch())
#define AG_CHAR(v) (event->argt[v]==AG_EVARG_CHAR?(char)event->argv[v].i:\
    (char)AG_IntMismatch())
#define AG_UCHAR(v) (event->argt[v]==AG_EVARG_UCHAR?(char)event->argv[v].i:\
    (unsigned char)AG_IntMismatch())
#define AG_INT(v) (event->argt[v]==AG_EVARG_INT?event->argv[v].i:\
    AG_IntMismatch())
#define AG_UINT(v) (event->argt[v]==AG_EVARG_UINT?(unsigned)event->argv[v].i:\
    (unsigned)AG_IntMismatch())
#define AG_LONG(v) (event->argt[v]==AG_EVARG_LONG?event->argv[v].li:\
    (long)AG_IntMismatch())
#define AG_ULONG(v) (event->argt[v]==AG_EVARG_ULONG?\
    (unsigned long)event->argv[v].li:(unsigned long)AG_IntMismatch())
#define AG_FLOAT(v) (event->argt[v]==AG_EVARG_FLOAT?(float)event->argv[v].f:\
    (unsigned long)AG_FloatMismatch())
#define AG_DOUBLE(v) (event->argt[v]==AG_EVARG_DOUBLE?event->argv[v].f:\
    AG_FloatMismatch())
 
#define AG_OBJECT(v,t) \
 (AG_OfClass(event->argv[v].p,(t)))?event->argv[v].p:\
  AG_ObjectMismatch(OBJECT(event->argv[v].p)->cls->hier,(t))

#else /* !DEBUG */

#define AG_PTR(v) (event->argv[v].p)
#define AG_STRING(v) (event->argv[v].s)
#define AG_CHAR(v) ((char)event->argv[v].i)
#define AG_UCHAR(v) ((unsigned char)event->argv[v].i)
#define AG_INT(v) (event->argv[v].i)
#define AG_UINT(v) ((unsigned)event->argv[v].i)
#define AG_LONG(v) (event->argv[v].li)
#define AG_ULONG(v) ((unsigned long)event->argv[v].li)
#define AG_FLOAT(v) ((float)event->argv[v].f)
#define AG_DOUBLE(v) (event->argv[v].f)
#define AG_OBJECT(v,t) (event->argv[v].p)

#endif /* DEBUG */

#define AG_SELF() AG_PTR(0)
#define AG_SENDER() AG_PTR(event->argc)
#define AG_SINT8(v) ((Sint8)AG_INT(v))
#define AG_UINT8(v) ((Uint8)AG_UINT(v))
#define AG_SINT16(v) ((Sint16)AG_INT(v))
#define AG_UINT16(v) ((Uint16)AG_UINT(v))
#define AG_SINT32(v) ((Sint32)AG_LONG(v))
#define AG_UINT32(v) ((Uint32)AG_ULONG(v))
#define AG_SDLKEY(v) ((SDLKey)AG_INT(v))
#define AG_SDLMOD(v) ((SDLMod)AG_INT(v))

/* Event / event handler structure */
typedef struct ag_event {
	char name[AG_EVENT_NAME_MAX];		/* String identifier */
	
	Uint flags;
#define	AG_EVENT_ASYNC     0x01			/* Service in separate thread */
#define AG_EVENT_PROPAGATE 0x02			/* Forward to child objs */
#define AG_EVENT_SCHEDULED 0x04			/* Timing-dependent (RO) */

	void (*handler)(struct ag_event *);

	int argc;				/* Total argument count */
	int argc0;				/* Argument count (omitting
						   PostEvent() arguments) */
	AG_EvArg argv[AG_EVENT_ARGS_MAX];	/* Argument values */
	int 	 argt[AG_EVENT_ARGS_MAX];	/* Argument types */
	const char *argn[AG_EVENT_ARGS_MAX];	/* Argument names */

	AG_Timeout timeout;			/* Execution timeout */
	AG_TAILQ_ENTRY(ag_event) events;	/* For Object */
} AG_Event;

typedef void (*AG_EventFn)(AG_Event *);

__BEGIN_DECLS
extern const char *agEvArgTypeNames[];

void      AG_EventInit(AG_Event *);
void      AG_EventArgs(AG_Event *, const char *, ...);
AG_Event *AG_SetEvent(void *, const char *, AG_EventFn, const char *, ...);
AG_Event *AG_AddEvent(void *, const char *, AG_EventFn, const char *, ...);
void      AG_UnsetEvent(void *, const char *);
void      AG_PostEvent(void *, void *, const char *, const char *, ...);
int       AG_ProcessEvent(SDL_Event *);
AG_Event *AG_FindEventHandler(void *, const char *);

int       AG_SchedEvent(void *, void *, Uint32, const char *,
                        const char *, ...);
int       AG_ReschedEvent(void *, const char *, Uint32);
int       AG_CancelEvent(void *, const char *);
void      AG_ForwardEvent(void *, void *, AG_Event *);
void      AG_BindGlobalKey(SDLKey, SDLMod, void (*)(void));
void      AG_BindGlobalKeyEv(SDLKey, SDLMod, void (*)(AG_Event *));

/* Execute an event handler routine without processing any arguments. */
static __inline__ void
AG_ExecEventFn(void *obj, AG_Event *ev)
{
	if (ev->handler != NULL)
		AG_PostEvent(NULL, obj, ev->name, NULL);
}
__END_DECLS

#ifdef DEBUG
#define AG_EVENT_DEFAULT_CASE() default: AG_FatalError("Bad event arg spec");
#define AG_EVENT_BOUNDARY_CHECK(ev) \
  if ((ev)->argc >= AG_EVENT_ARGS_MAX-1) AG_FatalError("Excess event args");
#else
#define AG_EVENT_DEFAULT_CASE() default: break;
#define AG_EVENT_BOUNDARY_CHECK(ev)
#endif

#define AG_EVENT_INS_VAL(eev,tname,aname,member,val) {		\
	AG_EVENT_BOUNDARY_CHECK(eev)				\
	(eev)->argv[(eev)->argc].member = (val);		\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argn[(eev)->argc] = (aname);			\
	(eev)->argc++;						\
}
#define AG_EVENT_INS_ARG(eev,ap,tname,member,t) { 		\
	AG_EVENT_BOUNDARY_CHECK(eev)				\
	(eev)->argv[(eev)->argc].member = va_arg(ap, t);	\
	(eev)->argt[(eev)->argc] = (tname);			\
	(eev)->argn[(eev)->argc] = "";				\
	(eev)->argc++;						\
}

#define AG_EVENT_PUSH_ARG(ap,fp,ev) {					\
	switch (*(fp)) {						\
	case 'p':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_POINTER,p,void *);	\
		break;							\
	case 'i':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_INT,i,int);		\
		break;							\
	case 'u':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_UINT,i,int);		\
		break;							\
	case 'f':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_FLOAT,f,double);	\
		break;							\
	case 'd':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_DOUBLE,f,double);	\
		break;							\
	case 's':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_STRING,s,char *);	\
		break;							\
	case 'c':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_UCHAR,i,int);	\
		break;							\
	case 'C':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_CHAR,i,int);	\
		break;							\
	case 'U':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_ULONG,li,long int);	\
		break;							\
	case 'D':							\
		AG_EVENT_INS_ARG((ev),ap,AG_EVARG_LONG,li,long int);	\
		break;							\
	case ' ':							\
	case ',':							\
	case '%':							\
		break;							\
	AG_EVENT_DEFAULT_CASE()						\
	}								\
	(fp)++;								\
}

#define AG_EVENT_GET_ARGS(ev, fmtp)					\
	if (fmtp != NULL) {						\
		const char *e_fc = fmtp;				\
		va_list ap;						\
									\
		va_start(ap, fmtp);					\
		while (*e_fc != '\0') AG_EVENT_PUSH_ARG(ap,(e_fc),(ev)); \
		va_end(ap);						\
	}

__BEGIN_DECLS
/*
 * Push arguments onto an Event structure.
 */
static __inline__ void
AG_EventPushPointer(AG_Event *ev, const char *key, void *val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_POINTER, key, p, val);
}
static __inline__ void
AG_EventPushString(AG_Event *ev, const char *key, char *val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_STRING, key, s, val);
}
static __inline__ void
AG_EventPushChar(AG_Event *ev, const char *key, char val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_CHAR, key, i, (int)val);
}
static __inline__ void
AG_EventPushUChar(AG_Event *ev, const char *key, Uchar val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_UCHAR, key, i, (int)val);
}
static __inline__ void
AG_EventPushInt(AG_Event *ev, const char *key, int val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_INT, key, i, val);
}
static __inline__ void
AG_EventPushUInt(AG_Event *ev, const char *key, Uint val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_UINT, key, i, (int)val);
}
static __inline__ void
AG_EventPushLong(AG_Event *ev, const char *key, long val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_LONG, key, li, val);
}
static __inline__ void
AG_EventPushULong(AG_Event *ev, const char *key, Ulong val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_ULONG, key, li, val);
}
static __inline__ void
AG_EventPushFloat(AG_Event *ev, const char *key, float val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_FLOAT, key, f, (double)val);
}
static __inline__ void
AG_EventPushDouble(AG_Event *ev, const char *key, double val)
{
	AG_EVENT_INS_VAL(ev, AG_EVARG_DOUBLE, key, f, val);
}
static __inline__ void
AG_EventPopArgument(AG_Event *ev)
{
	ev->argc--;
}
__END_DECLS

#include "close_code.h"
