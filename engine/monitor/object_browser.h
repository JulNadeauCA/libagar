/*	$Csoft: object_browser.h,v 1.3 2002/11/14 07:59:28 vedge Exp $	*/
/*	Public domain	*/

struct object_browser {
	struct monitor_tool	 tool;
	struct tlist		*objlist;
	int			 flags;
};

struct object_browser	*object_browser_new(struct monitor *, int);
void			 object_browser_init(struct object_browser *,
			     struct monitor *, int);
struct window		*object_browser_window(void *);
void			 object_browser_attached_object(int, union evarg *);
void			 object_browser_detached_object(int, union evarg *);
