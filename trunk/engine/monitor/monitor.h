/*	$Csoft: monitor.h,v 1.1.1.1 2002/09/01 09:03:06 vedge Exp $	*/
/*	Public domain	*/

struct monitor {
	struct	 object obj;

	int	 flags;

	struct {
		struct	window *toolbar;
		struct	window *object_browser;
		struct	window *media_browser;
	} wins;
};

enum {
	MONITOR_OBJECT_BROWSER,
	MONITOR_MEDIA_BROWSER
};

extern struct monitor monitor;	/* engine.c */

void	monitor_init(struct monitor *, char *name);
void	monitor_destroy(void *);

