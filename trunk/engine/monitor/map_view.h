/*	$Csoft: sprite_browser.h,v 1.1 2002/09/16 16:44:12 vedge Exp $	*/
/*	Public domain	*/

struct map_view {
	struct	 monitor_tool tool;
	int	 flags;
};

struct map_view		*map_view_new(struct monitor *, int);
void			 map_view_init(struct map_view *,
			     struct monitor *, int);
struct window		*map_view_window(void *);
void			 map_view_show(int, union evarg *);

