/*	$Csoft: monitor.h,v 1.2 2002/09/04 03:21:00 vedge Exp $	*/
/*	Public domain	*/

struct monitor {
	struct	 object obj;

	int	 flags;

	struct {
		struct	window *toolbar;
		struct	window *object_browser;
		struct	window *sprite_browser;
	} wins;
};

enum {
	MONITOR_SPRITE_BROWSER,
	MONITOR_OBJECT_BROWSER
};

extern struct monitor monitor;	/* engine.c */

void	monitor_init(struct monitor *, char *name);
void	monitor_destroy(void *);

