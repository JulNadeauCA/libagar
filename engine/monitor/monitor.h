/*	$Csoft: monitor.h,v 1.28 2005/06/11 11:10:38 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MONITOR_MONITOR_H_
#define _AGAR_MONITOR_MONITOR_H_
#include "begin_code.h"

struct ag_menu_item;

__BEGIN_DECLS
void AG_MonitorMenu(struct ag_menu_item *);
void AG_MonitorDestroy(void);

AG_Window *AG_DebugWidgetBrowser(void);
AG_Window *AG_DebugViewSettings(void);
AG_Window *AG_DebugScreenshot(void);
AG_Window *AG_DebugUnicodeBrowser(void);
AG_Window *AG_DebugLeakDetector(void);
AG_Window *AG_DebugTimeoutList(void);
AG_Window *AG_DebugServerWindow(void);
int	   AG_DebugServerStart(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MONITOR_MONITOR_H_ */
