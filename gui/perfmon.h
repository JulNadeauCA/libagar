/*	Public domain	*/

#include <agar/gui/begin.h>

__BEGIN_DECLS
extern int agEventAvg;			/* Number of events in last frame */
extern int agIdleAvg;			/* Measured SDL_Delay() granularity */
extern AG_Window *agPerfWindow;

AG_Window *AG_PerfMonShow(void);
void       AG_PerfMonInit(void);
void       AG_PerfMonUpdate(void);
__END_DECLS

#include <agar/gui/close.h>
