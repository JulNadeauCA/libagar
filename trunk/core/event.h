/*	Public domain	*/

#include <agar/core/begin.h>

#define AG_EVENT_ARGS_MAX 16
#define AG_EVENT_NAME_MAX 32

#ifdef AG_DEBUG
# define AG_PTR(v)	(event->argv[v].type==AG_VARIABLE_POINTER ? event->argv[v].data.p : AG_PtrMismatch())
# define AG_OBJECT(v,t)	(AG_OfClass(event->argv[v].data.p,(t))) ? event->argv[v].data.p : AG_ObjectMismatch(AGOBJECT(event->argv[v].data.p)->cls->hier,(t))
# define AG_STRING(v)	(event->argv[v].type==AG_VARIABLE_STRING ? event->argv[v].data.s : (char *)AG_PtrMismatch())
# define AG_INT(v)	(event->argv[v].type==AG_VARIABLE_INT ? event->argv[v].data.i : AG_IntMismatch())
# define AG_UINT(v)	(event->argv[v].type==AG_VARIABLE_UINT ? event->argv[v].data.u : (Uint)AG_IntMismatch())
# define AG_LONG(v)	(event->argv[v].type==AG_VARIABLE_SINT32 ? event->argv[v].data.s32 : (Sint32)AG_IntMismatch())
# define AG_ULONG(v)	(event->argv[v].type==AG_VARIABLE_UINT32 ? event->argv[v].data.u32 : (Uint32)AG_IntMismatch())
# define AG_FLOAT(v)	(event->argv[v].type==AG_VARIABLE_FLOAT ? event->argv[v].data.flt : AG_FloatMismatch())
# define AG_DOUBLE(v)	(event->argv[v].type==AG_VARIABLE_DOUBLE ? event->argv[v].data.dbl : (double)AG_FloatMismatch())
#else /* !AG_DEBUG */
# define AG_PTR(v)	(event->argv[v].data.p)
# define AG_OBJECT(v,t)	(event->argv[v].data.p)
# define AG_STRING(v)	(event->argv[v].data.s)
# define AG_INT(v)	(event->argv[v].data.i)
# define AG_UINT(v)	(event->argv[v].data.u)
# define AG_LONG(v)	((long)event->argv[v].data.s32)
# define AG_ULONG(v)	((Ulong)event->argv[v].data.u32)
# define AG_FLOAT(v)	((float)event->argv[v].data.flt)
# define AG_DOUBLE(v)	(event->argv[v].data.dbl)
#endif /* AG_DEBUG */

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
	AG_Variable argv[AG_EVENT_ARGS_MAX];	/* Argument values */

	AG_Timeout timeout;			/* Execution timeout */
	AG_TAILQ_ENTRY(ag_event) events;	/* Entry in Object */
} AG_Event;

/* Queue of events */
typedef struct ag_event_queue {
	Uint     nEvents;
	AG_Event *events;
} AG_EventQ;

typedef void (*AG_EventFn)(AG_Event *);

#ifdef AG_DEBUG
#define AG_EVENT_BOUNDARY_CHECK(ev) \
	if ((ev)->argc >= AG_EVENT_ARGS_MAX-1) \
		AG_FatalError("Too many AG_Event(3) arguments");
#else
#define AG_EVENT_BOUNDARY_CHECK(ev)
#endif

#define AG_EVENT_INS_VAL(eev,tname,aname,member,val) {			\
	AG_EVENT_BOUNDARY_CHECK(eev)					\
	(eev)->argv[(eev)->argc].type = (tname);			\
	if ((aname) != NULL) {						\
		AG_Strlcpy((eev)->argv[(eev)->argc].name, (aname),	\
		        AG_VARIABLE_NAME_MAX);				\
	} else {							\
		(eev)->argv[(eev)->argc].name[0] = '\0';		\
	}								\
	(eev)->argv[(eev)->argc].mutex = NULL;				\
	(eev)->argv[(eev)->argc].data.member = (val);			\
	(eev)->argv[(eev)->argc].fn.fnVoid = NULL;			\
	(eev)->argc++;							\
}
#define AG_EVENT_INS_ARG(eev,ap,tname,member,t) { 			\
	V = &(eev)->argv[(eev)->argc];					\
	AG_EVENT_BOUNDARY_CHECK(eev)					\
	V->type = (tname);						\
	V->mutex = NULL;						\
	V->data.member = va_arg(ap,t);					\
	V->fn.fnVoid = NULL;						\
	(eev)->argc++;							\
}
#define AG_EVENT_PUSH_ARG(ap,ev) {					\
	AG_Variable *V;							\
									\
	switch (*c) {							\
	case 'p':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_POINTER,p,void *);	\
		break;							\
	case 'i':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_INT,i,int);	\
		break;							\
	case 'u':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_UINT,i,int);	\
		break;							\
	case 'f':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_FLOAT,flt,double);	\
		break;							\
	case 'd':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_DOUBLE,dbl,double);\
		break;							\
	case 's':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_STRING,s,char *);	\
		break;							\
	case 'l':							\
		switch (c[1]) {						\
		case 'i':						\
			AG_EVENT_INS_ARG((ev),ap,			\
			    AG_VARIABLE_SINT32,				\
			    s32,long);					\
			break;						\
		case 'u':						\
			AG_EVENT_INS_ARG((ev),ap,			\
			    AG_VARIABLE_UINT32,				\
			    u32,unsigned long);				\
			break;						\
		default:						\
			AG_FatalError("Bad AG_Event(3) arguments");	\
			continue;					\
		}							\
		c++;							\
		break;							\
	case 'C':							\
		switch (c[1]) {						\
		case 's':						\
			AG_EVENT_INS_ARG((ev),ap,			\
			    AG_VARIABLE_CONST_STRING,			\
			    Cs,const char *);				\
			break;						\
		case 'p':						\
			AG_EVENT_INS_ARG((ev),ap,			\
			    AG_VARIABLE_CONST_POINTER,			\
			    Cp,const void *);				\
			break;						\
		default:						\
			AG_FatalError("Bad AG_Event(3) arguments");	\
			continue;					\
		}							\
		c++;							\
		break;							\
	case ' ':							\
	case ',':							\
	case '%':							\
		c++;							\
		continue;						\
	default:							\
		AG_FatalError("Bad AG_Event(3) argument: `%c'", *c);	\
		c++;							\
		continue;						\
	}								\
	c++;								\
	if (*c == '(' && c[1] != '\0') {				\
		char *cEnd;						\
		Strlcpy(V->name, &c[1], sizeof(V->name));		\
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
			AG_EVENT_PUSH_ARG(ap,(ev));			\
		}							\
		va_end(ap);						\
	}

__BEGIN_DECLS
void      AG_EventInit(AG_Event *);
void      AG_EventArgs(AG_Event *, const char *, ...);
AG_Event *AG_SetEvent(void *, const char *, AG_EventFn, const char *, ...);
AG_Event *AG_AddEvent(void *, const char *, AG_EventFn, const char *, ...);
void      AG_UnsetEvent(void *, const char *);
void      AG_PostEvent(void *, void *, const char *, const char *, ...);
AG_Event *AG_FindEventHandler(void *, const char *);

void      AG_InitEventQ(AG_EventQ *);
void      AG_FreeEventQ(AG_EventQ *);
void      AG_QueueEvent(AG_EventQ *, const char *, const char *, ...);

int       AG_SchedEvent(void *, void *, Uint32, const char *,
                        const char *, ...);
int       AG_ReschedEvent(void *, const char *, Uint32);
int       AG_CancelEvent(void *, const char *);
void      AG_ForwardEvent(void *, void *, AG_Event *);

/* Execute an event handler routine without processing any arguments. */
static __inline__ void
AG_ExecEventFn(void *obj, AG_Event *ev)
{
	if (ev->handler != NULL)
		AG_PostEvent(NULL, obj, ev->name, NULL);
}

/* Push arguments onto an Event structure. */
static __inline__ void
AG_EventPushPointer(AG_Event *ev, const char *key, void *val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_POINTER, key, p, val);
}
static __inline__ void
AG_EventPushString(AG_Event *ev, const char *key, char *val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_STRING, key, s, val);
}
static __inline__ void
AG_EventPushInt(AG_Event *ev, const char *key, int val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_INT, key, i, val);
}
static __inline__ void
AG_EventPushUint(AG_Event *ev, const char *key, Uint val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_UINT, key, i, (int)val);
}
static __inline__ void
AG_EventPushLong(AG_Event *ev, const char *key, long val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_SINT32, key, s32, (Sint32)val);
}
static __inline__ void
AG_EventPushUlong(AG_Event *ev, const char *key, Ulong val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_UINT32, key, u32, (Uint32)val);
}
static __inline__ void
AG_EventPushFloat(AG_Event *ev, const char *key, float val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_FLOAT, key, flt, val);
}
static __inline__ void
AG_EventPushDouble(AG_Event *ev, const char *key, double val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_DOUBLE, key, dbl, val);
}
static __inline__ void
AG_EventPopArgument(AG_Event *ev)
{
	ev->argc--;
}

/*
 * Accessor functions for AG_FOO_NAMED() macros.
 */
static __inline__ AG_Variable *
AG_GetNamedEventArg(AG_Event *ev, const char *key)
{
	int i;

	for (i = 0; i < ev->argc; i++) {
		if (strcmp(ev->argv[i].name, key) == 0)
			return (&ev->argv[i]);
	}
	AG_FatalError("No such AG_Event argument: \"%s\"", key);
	return (NULL);
}
static __inline__ void *
AG_GetNamedPtr(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return (V->data.p);
}
static __inline__ void *
AG_GetNamedObject(AG_Event *event, const char *key, const char *classSpec)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);

	if (!AG_OfClass((AG_Object *)V->data.p, classSpec)) {
		AG_FatalError("Argument %s is not a %s", key, classSpec);
	}
	return (V->data.p);
}
static __inline__ char *
AG_GetNamedString(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return (V->data.s);
}
static __inline__ int
AG_GetNamedInt(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return (V->data.i);
}
static __inline__ Uint
AG_GetNamedUint(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return (V->data.u);
}
static __inline__ long
AG_GetNamedLong(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return ((long)V->data.s32);
}
static __inline__ Ulong
AG_GetNamedUlong(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return ((Ulong)V->data.u32);
}
static __inline__ float
AG_GetNamedFlt(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return (V->data.flt);
}
static __inline__ double
AG_GetNamedDbl(AG_Event *event, const char *key)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);
	return (V->data.dbl);
}

#ifdef AG_LEGACY
# define AG_CHAR(v) ((char)event->argv[v].data.i)
# define AG_UCHAR(v) ((Uchar)event->argv[v].data.u)
# define AG_EventPushChar(ev,key,val) AG_EventPushInt((ev),(key),(int)(val))
# define AG_EventPushUchar(ev,key,val) AG_EventPushUint((ev),(key),(Uint)(val))
#endif
__END_DECLS

#include <agar/core/close.h>
