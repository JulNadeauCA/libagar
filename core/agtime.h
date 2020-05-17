/*	Public domain	*/

#ifndef _AGAR_CORE_OBJECT_H_
# error "Must be included by object.h"
#endif

#ifndef AG_TIMER_NAME_MAX
# if AG_MODEL == AG_SMALL
#  define AG_TIMER_NAME_MAX 8
# else
#  define AG_TIMER_NAME_MAX 16
# endif
#endif

struct ag_timer;

typedef Uint32 (*AG_TimerFn)(struct ag_timer *_Nonnull, AG_Event *_Nonnull);

typedef struct ag_timer_pvt {
	AG_TAILQ_ENTRY(ag_timer) timers;
	AG_TAILQ_ENTRY(ag_timer) change;
} AG_TimerPvt;

typedef struct ag_timer {
	AG_Event fnEvent;		/* Callback arguments */
	AG_TimerFn fn;			/* Function callback */
	AG_TimerPvt pvt;		/* Private data */
	void *_Nullable obj;		/* Parent object */
	Uint flags;
#define AG_TIMER_SURVIVE_DETACH	0x01	/* Don't cancel on ObjectDetach() */
#define AG_TIMER_AUTO_FREE	0x02	/* Free the timer structure on expire */
#define AG_TIMER_EXECD		0x04	/* Callback was invoked manually */
#define AG_TIMER_RESTART	0x08	/* Queue timer for restart (driver-specific) */
	int id;				/* Unique identifier */
	Uint32 tSched;			/* Scheduled expiration time (ticks) */
	Uint32 ival;			/* Timer interval in ticks */
	char name[AG_TIMER_NAME_MAX];	/* Name string (optional) */
} AG_Timer;

typedef struct ag_time_ops {
	const char *_Nonnull name;

	void   (*_Nullable init) (void);
	void   (*_Nullable destroy) (void);
	Uint32 (*_Nonnull getTicks) (void);
	void   (*_Nonnull delay) (Uint32);
} AG_TimeOps;

#if defined(AG_TIMERS) && defined(AG_THREADS)
# define AG_LockTiming()	AG_MutexLock(&agTimerLock)
# define AG_UnlockTiming()	AG_MutexUnlock(&agTimerLock)
#else
# define AG_LockTiming()
# define AG_UnlockTiming()
#endif
#define AG_GetTicks()		agTimeOps->getTicks()
#define AG_Delay(t)		agTimeOps->delay(t)

__BEGIN_DECLS
/* Time backends for basic GetTicks() and Delay() operations */
extern const AG_TimeOps *_Nullable agTimeOps;
extern const AG_TimeOps agTimeOps_dummy;
extern const AG_TimeOps agTimeOps_gettimeofday;
extern const AG_TimeOps agTimeOps_posix;
extern const AG_TimeOps agTimeOps_win32;
extern const AG_TimeOps agTimeOps_renderer;

void AG_SetTimeOps(const AG_TimeOps *_Nullable);

#ifdef AG_TIMERS
/*
 * Managed timers which can be owned by objects and mapped to either
 * kernel/hardware timers, or entries in a software timing wheel.
 */
extern struct ag_objectq       agTimerObjQ;
extern Uint                    agTimerCount;
extern struct ag_object        agTimerMgr;
extern _Nonnull_Mutex AG_Mutex agTimerLock;

void AG_InitTimers(void);
void AG_DestroyTimers(void);
void AG_InitTimer(AG_Timer *_Nonnull, const char *_Nonnull, Uint);

AG_Timer *_Nullable AG_AddTimerAuto(void *_Nullable, Uint32, _Nonnull AG_TimerFn,
				    const char *_Nullable, ...);

int  AG_AddTimer(void *_Nullable, AG_Timer *_Nonnull, Uint32,
                 _Nonnull AG_TimerFn,
                 const char *_Nullable, ...);
void AG_DelTimer(void *_Nullable, AG_Timer *_Nonnull);
void AG_DelTimers(void *_Nonnull);
int  AG_ResetTimer(void *_Nullable, AG_Timer *_Nonnull, Uint32);
int  AG_TimerIsRunning(void *_Nullable, AG_Timer *_Nonnull)
                      _Pure_Attribute;
Uint32 AG_ExecTimer(AG_Timer *_Nonnull);
int  AG_TimerWait(void *_Nullable, AG_Timer *_Nonnull, Uint32);
void AG_ProcessTimeouts(Uint32);

# ifdef AG_LEGACY
#  define AG_Timeout AG_Timer
#  define AG_TIMEOUTS_QUEUED() 1
# endif
#endif /* AG_TIMERS */
__END_DECLS
