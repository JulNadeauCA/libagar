/*	$Csoft: monitor.h,v 1.25 2004/05/12 05:34:37 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MONITOR_MONITOR_H_
#define _AGAR_MONITOR_MONITOR_H_
#include "begin_code.h"

__BEGIN_DECLS
void		 monitor_init(void);
void		 monitor_destroy(void);

struct window	*widget_debug_window(void);
struct window	*view_params_window(void);
struct window	*screenshot_window(void);
struct window	*uniconv_window(void);
struct window	*leak_window(void);
struct window	*timeouts_window(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MONITOR_MONITOR_H_ */
