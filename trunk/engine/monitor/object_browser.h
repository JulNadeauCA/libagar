/*	$Csoft: object_browser.h,v 1.2 2002/11/14 05:59:02 vedge Exp $	*/
/*	Public domain	*/

struct object_browser {
	struct monitor_tool	 tool;
	struct tlist		*objlist;

	int	flags;
};

struct object_browser	*object_browser_new(struct monitor *, int);
void			 object_browser_init(struct object_browser *,
			     struct monitor *, int);
struct window		*object_browser_window(void *);
void			 object_browser_attached(int, union evarg *);
void			 object_browser_detached(int, union evarg *);
