/*	$Csoft: sprite_browser.h,v 1.1.1.1 2002/09/01 09:03:06 vedge Exp $	*/
/*	Public domain	*/

struct sprite_browser {
	struct	 monitor_tool tool;
	int	 flags;
};

struct sprite_browser	*sprite_browser_new(struct monitor *, int);
void			 sprite_browser_init(struct sprite_browser *,
			     struct monitor *, int);
struct window		*sprite_browser_window(void *);

