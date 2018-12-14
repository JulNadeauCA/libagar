/*	Public domain	*/

#ifndef _AGAR_CORE_OBJECT_H_
# error "Must be included by object.h"
#endif

#define AG_TIMER_NAME_MAX 16

typedef struct ag_timer_pvt {
	AG_TAILQ_ENTRY(ag_timer) timers;
	AG_TAILQ_ENTRY(ag_timer) change;
#ifdef AG_LEGACY
	Uint32 (*_Nullable fnLegacy)(void *_Nonnull p, Uint32 ival,
	                             void *_Nullable arg);
	void    *_Nullable argLegacy;
#endif
} AG_TimerPvt;

typedef struct ag_timer {
	int id;				/* Unique identifier */
	void *_Nullable obj;		/* Parent object */
	Uint flags;
#define AG_TIMER_SURVIVE_DETACH	0x01	/* Don't cancel on ObjectDetach() */
#define AG_TIMER_AUTO_FREE	0x02	/* Free the timer structure on expire */
#define AG_TIMER_EXECD		0x04	/* Callback was invoked manually */
#define AG_TIMER_RESTART	0x08	/* Queue timer for restart (driver-specific) */
	Uint32 tSched;			/* Scheduled expiration time (ticks) */
	Uint32 ival;			/* Timer interval in ticks */
	Uint32 (*_Nullable fn) (struct ag_timer *_Nonnull _Restrict,
	                        AG_Event        *_Nonnull _Restrict);
	AG_Event fnEvent;
	char name[AG_TIMER_NAME_MAX];	/* Name string (optional) */
	AG_TimerPvt pvt;		/* Private data */
} AG_Timer;

typedef Uint32 (*AG_TimerFn)(AG_Timer *_Nonnull, AG_Event *_Nonnull);

typedef struct ag_time_ops {
	const char *_Nonnull name;

	void   (*_Nullable init) (void);
	void   (*_Nullable destroy) (void);
	Uint32 (*_Nonnull getTicks) (void);
	void   (*_Nonnull delay) (Uint32);
} AG_TimeOps;

#define AG_LockTiming()		AG_MutexLock(&agTimerLock)
#define AG_UnlockTiming()	AG_MutexUnlock(&agTimerLock)
#define AG_GetTicks		agTimeOps->getTicks
#define AG_Delay(t)		agTimeOps->delay(t)

#ifdef AG_LEGACY
typedef AG_Timer AG_Timeout;
#endif

__BEGIN_DECLS
extern struct ag_objectq       agTimerObjQ;
extern Uint                    agTimerCount;
extern struct ag_object        agTimerMgr;
extern _Nonnull_Mutex AG_Mutex agTimerLock;

extern const AG_TimeOps *_Nullable agTimeOps;
extern const AG_TimeOps  agTimeOps_dummy;
extern const AG_TimeOps  agTimeOps_gettimeofday;
extern const AG_TimeOps  agTimeOps_posix;
extern const AG_TimeOps  agTimeOps_win32;
extern const AG_TimeOps  agTimeOps_renderer;

void AG_SetTimeOps(const AG_TimeOps *_Nullable);
void AG_InitTimers(void);
void AG_DestroyTimers(void);

void AG_InitTimer(AG_Timer *_Nonnull, const char *_Nonnull, Uint);

int  AG_AddTimer(void *_Nullable, AG_Timer *_Nonnull, Uint32,
                 _Nonnull AG_TimerFn,
		 const char *_Nullable, ...);

AG_Timer *_Nullable AG_AddTimerAuto(void *_Nullable, Uint32,
                                    _Nonnull AG_TimerFn,
				    const char *_Nullable, ...);

void AG_DelTimer(void *_Nullable, AG_Timer *_Nonnull);
void AG_DelTimers(void *_Nonnull);
int  AG_ResetTimer(void *_Nullable, AG_Timer *_Nonnull, Uint32);

int  AG_TimerIsRunning(void *_Nullable, AG_Timer *_Nonnull)
                      _Pure_Attribute;

int  AG_TimerWait(void *_Nullable, AG_Timer *_Nonnull, Uint32);
void AG_ProcessTimeouts(Uint32);
__END_DECLS
