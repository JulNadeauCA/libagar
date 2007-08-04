/*	Public domain	*/

#include "begin_code.h"

#define AG_EVENT_ARGS_MAX 16
#define AG_EVENT_NAME_MAX 31

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
 (AG_ObjectIsClass(event->argv[v].p,(t)))?event->argv[v].p:\
  AG_ObjectMismatch(AGOBJECT(event->argv[v].p)->ops->type,(t))

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

typedef struct ag_event {
	char	name[AG_EVENT_NAME_MAX];
	Uint	flags;
#define	AG_EVENT_ASYNC		0x01	/* Event handler runs in own thread */
#define AG_EVENT_PROPAGATE	0x02	/* Relay event to object descendents */
#define AG_EVENT_SCHEDULED	0x04	/* Timing-dependent (read-only flag) */
	void (*handler)(struct ag_event *);
	int	 argc, argc0;
	AG_EvArg argv[AG_EVENT_ARGS_MAX];
	int 	 argt[AG_EVENT_ARGS_MAX];
	char	*argn[AG_EVENT_ARGS_MAX];
	AG_Timeout timeout;
	TAILQ_ENTRY(ag_event) events;
} AG_Event;

typedef void (*AG_EventFn)(AG_Event *);

extern const char *agEvArgTypeNames[];

__BEGIN_DECLS
void	AG_EventLoop_FixedFPS(void);
#define AG_EventLoop() AG_EventLoop_FixedFPS()

AG_Event *AG_SetEvent(void *, const char *, AG_EventFn, const char *, ...);
AG_Event *AG_AddEvent(void *, const char *, AG_EventFn, const char *, ...);
void	  AG_UnsetEvent(void *, const char *);
void	  AG_PostEvent(void *, void *, const char *, const char *, ...);
void	  AG_ProcessEvent(SDL_Event *);
AG_Event *AG_FindEventHandler(void *, const char *);

int	  	 AG_SchedEvent(void *, void *, Uint32, const char *,
		               const char *, ...);
int		 AG_ReschedEvent(void *, const char *, Uint32);
int		 AG_CancelEvent(void *, const char *);
__inline__ void	 AG_ExecEvent(void *, const char *);
void		 AG_ForwardEvent(void *, void *, AG_Event *);
void		 AG_BindGlobalKey(SDLKey, SDLMod, void (*)(void));
struct ag_window *AG_EventShowPerfGraph(void);
__inline__ Uint8 AG_MouseGetState(int *, int *);
__END_DECLS

#ifdef DEBUG
#define AG_EVENT_DEFAULT_CASE() default: fatal("bad event arg spec");
#define AG_EVENT_BOUNDARY_CHECK(ev) \
  if ((ev)->argc >= AG_EVENT_ARGS_MAX-1) fatal("excess event args");
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
#define AG_EVENT_INS_ARG(eev,ap,tname,member,type) { 		\
	AG_EVENT_BOUNDARY_CHECK(eev)				\
	(eev)->argv[(eev)->argc].member = va_arg(ap, type);	\
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

#include "close_code.h"
