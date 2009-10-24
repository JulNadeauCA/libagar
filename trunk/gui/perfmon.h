/*	Public domain	*/

#ifndef _AGAR_GUI_PERFMON_H_
#define _AGAR_GUI_PERFMON_H_
#include <agar/gui/begin.h>

__BEGIN_DECLS
extern int agEventAvg;			/* Number of events in last frame */
extern int agIdleAvg;			/* Measured AG_Delay() granularity */
extern AG_Window *agPerfWindow;

AG_Window *AG_PerfMonShow(void);
void       AG_PerfMonInit(void);
void       AG_PerfMonUpdate(int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_PERFMON_H_ */
