/*	$Csoft: monitor.h,v 1.3 2002/09/09 00:42:54 vedge Exp $	*/
/*	Public domain	*/

struct window;
struct mapview;

struct monitor {
	struct	 object obj;

	int	 flags;

	struct {
		struct	window *toolbar;
		struct	window *object_browser;
		struct	window *sprite_browser;
	} wins;
};

struct monitor_tool_ops {
	const	 struct object_ops obops;

	struct	 window	*(*tool_window)(void *);
};

struct monitor_tool {
	struct	 object obj;
	char	*type;
	Uint32	 flags;
	struct	 monitor *mon;		/* Map editor */
	struct	 window *win;		/* Tool settings window */
	struct	 button *button;	/* Back pointer to button */
};

enum {
	MONITOR_SPRITE_BROWSER,
	MONITOR_OBJECT_BROWSER
};

#define	MONITOR_TOOL(t)		((struct monitor_tool *)(t))
#define MONITOR_TOOL_OPS(t)	((struct monitor_tool_ops *)OBJECT((t))->ops)

extern struct monitor monitor;	/* engine.c */

void		 monitor_init(struct monitor *, char *name);
void		 monitor_destroy(void *);
void		 monitor_tool_init(struct monitor_tool *, char *,
		     struct monitor *, const void *);
struct mapview	*monitor_tool_mapview(void);

