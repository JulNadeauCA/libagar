/*	$Csoft: monitor_tool.h,v 1.1.1.1 2002/09/01 09:00:48 vedge Exp $	*/
/*	Public domain	*/

struct window;
struct mapview;

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

#define	MONITOR_TOOL(t)		((struct monitor_tool *)(t))
#define MONITOR_TOOL_OPS(t)	((struct monitor_tool_ops *)OBJECT((t))->ops)

void		 monitor_tool_init(struct monitor_tool *, char *,
		     struct monitor *, const void *);
struct mapview	*monitor_tool_mapview(void);

