/*	$Csoft: monitor.h,v 1.11 2002/12/01 10:46:42 vedge Exp $	*/
/*	Public domain	*/

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
	MONITOR_VIEW_PARAMS
};

extern struct monitor	monitor;

void	 monitor_init(struct monitor *, char *name);
void	 monitor_destroy(void *);

struct window	*object_browser_window(void);
struct window	*level_browser_window(void);
struct window	*widget_browser_window(void);
struct window	*view_params_window(void);

