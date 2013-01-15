/*	Public domain	*/

#include <agar/core/begin.h>

#define AG_TIMER_NAME_MAX 16

typedef struct ag_timer {
	int id;				/* Unique identifier */
	void *obj;			/* Parent object */
	Uint flags;
#define AG_TIMER_SURVIVE_DETACH	0x01	/* Don't cancel on ObjectDetach() */
#define AG_TIMER_AUTO_FREE	0x02	/* Free the timer structure on expire */
#define AG_TIMER_EXECD		0x04	/* Callback was invoked manually */
#define AG_TIMER_RESTART	0x08	/* Queue timer for restart (driver-specific) */
	Uint32 tSched;			/* Scheduled expiration time (ticks) */
	Uint32 ival;			/* Timer interval in ticks */
	Uint32 (*fn)(struct ag_timer *, AG_Event *);
	AG_Event fnEvent;
	AG_TAILQ_ENTRY(ag_timer) timers;
	AG_TAILQ_ENTRY(ag_timer) change;
#ifdef AG_LEGACY
	Uint32 (*fnLegacy)(void *p, Uint32 ival, void *arg);
	void   *argLegacy;
#endif
	char name[AG_TIMER_NAME_MAX];	/* Name string (optional) */
} AG_Timer;

typedef Uint32 (*AG_TimerFn)(AG_Timer *, AG_Event *);

typedef struct ag_time_ops {
	const char *name;
	void   (*init)(void);
	void   (*destroy)(void);
	Uint32 (*getTicks)(void);
	void   (*delay)(Uint32);
} AG_TimeOps;

#define AG_LockTiming()		AG_MutexLock(&agTimerLock)
#define AG_UnlockTiming()	AG_MutexUnlock(&agTimerLock)
#define AG_GetTicks		agTimeOps->getTicks
#define AG_Delay(t)		agTimeOps->delay(t)

#ifdef AG_LEGACY
typedef AG_Timer AG_Timeout;
#endif

__BEGIN_DECLS
extern struct ag_objectq agTimerObjQ;
extern Uint              agTimerCount;
extern struct ag_object  agTimerMgr;
extern AG_Mutex          agTimerLock;

extern const AG_TimeOps *agTimeOps;
extern const AG_TimeOps  agTimeOps_dummy;
extern const AG_TimeOps  agTimeOps_gettimeofday;
extern const AG_TimeOps  agTimeOps_posix;
extern const AG_TimeOps  agTimeOps_win32;
extern const AG_TimeOps  agTimeOps_renderer;

void    AG_SetTimeOps(const AG_TimeOps *);
void	AG_InitTimers(void);
void	AG_DestroyTimers(void);

void      AG_InitTimer(AG_Timer *, const char *, Uint);
int       AG_AddTimer(void *, AG_Timer *, Uint32, AG_TimerFn, const char *, ...);
AG_Timer *AG_AddTimerAuto(void *, Uint32, AG_TimerFn, const char *, ...);
void	  AG_DelTimer(void *, AG_Timer *);
int	  AG_ResetTimer(void *, AG_Timer *, Uint32);
int	  AG_TimerIsRunning(void *, AG_Timer *);
int       AG_TimerWait(void *, AG_Timer *, Uint32);

void    AG_ProcessTimeouts(Uint32);

/* Execute a timer's associated callback routine. */
static __inline__ Uint32
AG_ExecTimerFn(AG_Timer *to)
{
	Uint32 rv;

	to->flags |= AG_TIMER_EXECD;
	rv = to->fn(to, &to->fnEvent);
	to->flags &= ~(AG_TIMER_EXECD);
	return (rv);
}

#ifdef AG_LEGACY
void AG_SetTimeout(AG_Timeout *, Uint32 (*)(void *, Uint32, void *), void *, Uint) DEPRECATED_ATTRIBUTE;
void AG_ScheduleTimeout(void *, AG_Timeout *, Uint32) DEPRECATED_ATTRIBUTE;
# define AG_TIMEOUT_INITIALIZER { -1, NULL, 0, 0, 0, NULL }
# define AG_TIMEOUTS_QUEUED() (!AG_TAILQ_EMPTY(&agTimerObjQ))
# define AG_CANCEL_ONDETACH	0x10	/* Don't cancel on ObjectDetach() */
# define AG_CANCEL_ONLOAD	0x20	/* In queue for execution */
# define AG_TimeoutWait(obj,to,d) AG_TimerWait((obj),(to),(d))
# define AG_TimeoutIsScheduled(obj,to) AG_TimerIsRunning((obj),(to))
# define AG_DelTimeout(obj,to) AG_DelTimer((obj),(to))
# define AG_ProcessTimeout(x) AG_ProcessTimeouts(x)
# define AG_AddTimeout(p,to,dt) AG_ScheduleTimeout((p),(to),(dt))
# define AG_ReplaceTimeout(p,to,dt) AG_ScheduleTimeout((p),(to),(dt))
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/core/close.h>
