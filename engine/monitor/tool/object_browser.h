/*	$Csoft: object_browser.h,v 1.1 2002/09/04 03:21:01 vedge Exp $	*/
/*	Public domain	*/

struct object_browser {
	struct	 monitor_tool tool;
	int	 flags;
};

struct object_browser	*object_browser_new(struct monitor *, int);
void			 object_browser_init(struct object_browser *,
			     struct monitor *, int);
struct window		*object_browser_window(void *);

