/*	$Csoft: monitor.h,v 1.15 2003/03/20 01:17:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MONITOR_MONITOR_H_
#define _AGAR_MONITOR_MONITOR_H_
#include "begin_code.h"

struct object;
struct window;

struct monitor {
	struct object	 obj;
	struct window	*toolbar;
};

/* Icons */
enum {
	MONITOR_OBJECT_BROWSER,
	MONITOR_MEDIA_BROWSER,
	MONITOR_LEVEL_BROWSER,
	MONITOR_WIDGET_BROWSER,
	MONITOR_VIEW_PARAMS,
	MONITOR_FPS_COUNTER,
	MONITOR_SCREENSHOT,
	MONITOR_PROPS_BASE
};

extern struct monitor	monitor;

__BEGIN_DECLS
extern DECLSPEC void		 monitor_init(struct monitor *, char *);
extern DECLSPEC void		 monitor_destroy(void *);

extern DECLSPEC struct window	*object_browser_window(void);
extern DECLSPEC struct window	*level_browser_window(void);
extern DECLSPEC struct window	*widget_browser_window(void);
extern DECLSPEC struct window	*view_params_window(void);
extern DECLSPEC struct window	*screenshot_window(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MONITOR_MONITOR_H_ */
