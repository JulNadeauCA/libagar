/*	$Csoft: monitor.h,v 1.10 2002/11/26 11:01:00 vedge Exp $	*/
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
	MONITOR_SPRITE_BROWSER,
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

