/*	$Csoft: monitor.h,v 1.24 2004/03/18 21:27:48 vedge Exp $	*/
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
	MONITOR_GFX_DEBUG,
	MONITOR_WIDGET_BROWSER,
	MONITOR_VIEW_PARAMS,
	MONITOR_FPS_COUNTER,
	MONITOR_SCREENSHOT,
	MONITOR_PROPS_BASE
};

extern struct monitor	monitor;

__BEGIN_DECLS
void		 monitor_init(struct monitor *, const char *);
void		 monitor_destroy(void *);
struct window	*widget_debug_window(void);
struct window	*view_params_window(void);
struct window	*screenshot_window(void);
struct window	*uniconv_window(void);
struct window	*leak_window(void);
struct window	*timeouts_window(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MONITOR_MONITOR_H_ */
