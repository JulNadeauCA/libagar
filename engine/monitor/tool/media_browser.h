/*	$Csoft: object_browser.h,v 1.1.1.1 2002/09/01 09:03:06 vedge Exp $	*/
/*	Public domain	*/

struct media_browser {
	struct	 monitor_tool tool;
	int	 flags;
};

struct media_browser	*media_browser_new(struct monitor *, int);
void			 media_browser_init(struct media_browser *,
			     struct monitor *, int);
struct window		*media_browser_window(void *);

