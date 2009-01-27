/*	Public domain	*/

#include <agar/core/begin.h>

#define AG_EVENT_ARGS_MAX 16
#define AG_EVENT_NAME_MAX 32

#ifdef AG_DEBUG
#define AG_PTR(v) (event->argv[v].type==AG_VARIABLE_POINTER ? event->argv[v].data.p : AG_PtrMismatch())
#define AG_STRING(v) (event->argv[v].type==AG_VARIABLE_STRING ? event->argv[v].data.s : (char *)AG_PtrMismatch())
#define AG_CHAR(v) (event->argv[v].type==AG_VARIABLE_CHAR ? (char)event->argv[v].data.i : (char)AG_IntMismatch())
#define AG_UCHAR(v) (event->argv[v].type==AG_VARIABLE_UCHAR ? (char)event->argv[v].data.i : (unsigned char)AG_IntMismatch())
#define AG_UINT(v) (event->argv[v].type==AG_VARIABLE_UINT ? (unsigned)event->argv[v].data.i : (unsigned)AG_IntMismatch())
#define AG_INT(v) (event->argv[v].type==AG_VARIABLE_INT ? event->argv[v].data.i : AG_IntMismatch())
#define AG_UINT8(v) (event->argv[v].type==AG_VARIABLE_UINT8 ? event->argv[v].data.u8 : (Uint8)AG_IntMismatch())
#define AG_SINT8(v) (event->argv[v].type==AG_VARIABLE_SINT8 ? event->argv[v].data.s8 : (Sint8)AG_IntMismatch())
#define AG_UINT16(v) (event->argv[v].type==AG_VARIABLE_UINT16 ? event->argv[v].data.u16 : (Uint16)AG_IntMismatch())
#define AG_SINT16(v) (event->argv[v].type==AG_VARIABLE_SINT16 ? event->argv[v].data.s16 : (Sint16)AG_IntMismatch())
#define AG_UINT32(v) (event->argv[v].type==AG_VARIABLE_UINT32 ? event->argv[v].data.u32 : (Uint32)AG_IntMismatch())
#define AG_SINT32(v) (event->argv[v].type==AG_VARIABLE_SINT32 ? event->argv[v].data.s32 : (Sint32)AG_IntMismatch())
#define AG_FLOAT(v) (event->argv[v].type==AG_VARIABLE_FLOAT ? (float)event->argv[v].data.flt : (unsigned long)AG_FloatMismatch())
#define AG_DOUBLE(v) (event->argv[v].type==AG_VARIABLE_DOUBLE ? event->argv[v].data.dbl : AG_FloatMismatch())
#define AG_OBJECT(v,t) (AG_OfClass(event->argv[v].data.p,(t))) ? event->argv[v].data.p : AG_ObjectMismatch(AGOBJECT(event->argv[v].data.p)->cls->hier,(t))

#else /* !AG_DEBUG */

#define AG_PTR(v)	(event->argv[v].data.p)
#define AG_STRING(v)	(event->argv[v].data.s)
#define AG_CHAR(v)	((char)event->argv[v].data.i)
#define AG_UCHAR(v)	((unsigned char)event->argv[v].data.i)
#define AG_UINT(v)	((unsigned)event->argv[v].data.i)
#define AG_INT(v)	(event->argv[v].data.i)
#define AG_UINT8(v)	(event->argv[v].data.u8)
#define AG_SINT8(v)	(event->argv[v].data.s8)
#define AG_UINT16(v)	(event->argv[v].data.u16)
#define AG_SINT16(v)	(event->argv[v].data.s16)
#define AG_UINT32(v)	(event->argv[v].data.u32)
#define AG_SINT32(v)	(event->argv[v].data.s32)
#define AG_FLOAT(v)	((float)event->argv[v].data.flt)
#define AG_DOUBLE(v)	(event->argv[v].data.dbl)
#define AG_OBJECT(v,t)	(event->argv[v].data.p)

#endif /* AG_DEBUG */

#define AG_SELF()	AG_PTR(0)
#define AG_SENDER()	AG_PTR(event->argc)
#define AG_LONG(v)	((long)AG_SINT32(v))
#define AG_ULONG(v)	((long)AG_UINT32(v))

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
	AG_TAILQ_ENTRY(ag_event) events;	/* For Object */
} AG_Event;

typedef void (*AG_EventFn)(AG_Event *);

#ifdef AG_DEBUG
#define AG_EVENT_DEFAULT_CASE() \
	default: AG_FatalError("Bad AG_Event(3) argument string");
#define AG_EVENT_BOUNDARY_CHECK(ev) \
	if ((ev)->argc >= AG_EVENT_ARGS_MAX-1) \
		AG_FatalError("Too many AG_Event(3) arguments");
#else
#define AG_EVENT_DEFAULT_CASE() default: break;
#define AG_EVENT_BOUNDARY_CHECK(ev)
#endif

#define AG_EVENT_INS_VAL(eev,tname,aname,member,val) {		\
	AG_EVENT_BOUNDARY_CHECK(eev)				\
	(eev)->argv[(eev)->argc].type = (tname);		\
	if ((aname) != NULL) {					\
		AG_Strlcpy((eev)->argv[(eev)->argc].name, (aname), \
		        AG_VARIABLE_NAME_MAX);			\
	} else {						\
		(eev)->argv[(eev)->argc].name[0] = '\0';	\
	}							\
	(eev)->argv[(eev)->argc].mutex = NULL;			\
	(eev)->argv[(eev)->argc].data.member = (val);		\
	(eev)->argv[(eev)->argc].fn.fnVoid = NULL;		\
	(eev)->argc++;						\
}
#define AG_EVENT_INS_ARG(eev,ap,tname,member,t) { 		\
	AG_EVENT_BOUNDARY_CHECK(eev)				\
	(eev)->argv[(eev)->argc].type = (tname);		\
	(eev)->argv[(eev)->argc].name[0] = '\0';		\
	(eev)->argv[(eev)->argc].mutex = NULL;			\
	(eev)->argv[(eev)->argc].data.member = va_arg(ap,t);	\
	(eev)->argv[(eev)->argc].fn.fnVoid = NULL;		\
	(eev)->argc++;						\
}

#define AG_EVENT_PUSH_ARG(ap,fp,ev) {					\
	switch (*(fp)) {						\
	case 'p':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_POINTER,p,void *);	\
		break;							\
	case 'i':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_INT,i,int);		\
		break;							\
	case 'u':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_UINT,i,int);		\
		break;							\
	case 'f':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_FLOAT,flt,double);	\
		break;							\
	case 'd':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_DOUBLE,dbl,double);	\
		break;							\
	case 's':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_STRING,s,char *);	\
		break;							\
	case 'c':							\
		AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_UINT8,u8,int);	\
		break;							\
	case 'C':							\
		switch (*(fp+1)) {					\
		case 's':						\
			AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_CONST_STRING,	\
			    Cs,const char *);				\
			break;						\
		case 'p':						\
			AG_EVENT_INS_ARG((ev),ap,AG_VARIABLE_CONST_POINTER,\
			    Cp,const void *);				\
			break;						\
		}							\
		(fp)++;							\
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
		const char *e_fc = (const char *)fmtp;			\
		va_list ap;						\
									\
		va_start(ap, fmtp);					\
		while (*e_fc != '\0') {					\
			AG_EVENT_PUSH_ARG(ap,(e_fc),(ev));		\
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
AG_EventPushFloat(AG_Event *ev, const char *key, float val)
{
	AG_EVENT_INS_VAL(ev, AG_VARIABLE_FLOAT, key, flt, (double)val);
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
__END_DECLS

/* Legacy */
#define AG_SDLKEY(v) ((SDLKey)AG_INT(v))
#define AG_SDLMOD(v) ((SDLMod)AG_INT(v))
#define AG_EventPushChar AG_EventPushSint8
#define AG_EventPushUChar AG_EventPushUint8
#define AG_EventPushLong AG_EventPushSint32
#define AG_EventPushULong AG_EventPushUint32

#include <agar/core/close.h>
