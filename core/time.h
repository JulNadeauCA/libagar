/*	Public domain	*/

#include <agar/core/begin.h>

typedef struct ag_time_ops {
	const char *name;
	void   (*Init)(void);
	void   (*Destroy)(void);
	Uint32 (*GetTicks)(void);
	void   (*Delay)(Uint32);
} AG_TimeOps;

__BEGIN_DECLS
extern const AG_TimeOps *agTimeOps;
extern const AG_TimeOps  agTimeOps_dummy;
extern const AG_TimeOps  agTimeOps_gettimeofday;
extern const AG_TimeOps  agTimeOps_win32;
extern const AG_TimeOps  agTimeOps_condwait;

void AG_SetTimeOps(const AG_TimeOps *);
#define AG_GetTicks  agTimeOps->GetTicks
#define AG_Delay(ms) agTimeOps->Delay(ms)
__END_DECLS

#include <agar/core/close.h>
