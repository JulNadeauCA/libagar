/*	$Csoft: monitor.h,v 1.7 2002/11/15 04:18:33 vedge Exp $	*/
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
	MONITOR_LEVEL_BROWSER
};

extern struct monitor	monitor;

void		 monitor_init(struct monitor *, char *name);
void		 monitor_destroy(void *);

struct window	*object_browser_window(void);
struct window	*sprite_browser_window(void);
struct window	*level_browser_window(void);
