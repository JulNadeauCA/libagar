/*	$Csoft: tool.h,v 1.2 2002/07/07 06:32:44 vedge Exp $	*/
/*	Public domain	*/

struct window;
struct mapview;

struct tool_ops {
	const	 struct object_ops obops;

	struct	 window	*(*tool_window)(void *);
	void	 	 (*tool_effect)(void *, struct mapview *,
			     Uint32, Uint32);
};

struct tool {
	struct	 object obj;
	char	*type;

	Uint32	 flags;

	struct	 mapedit *med;		/* Map editor */
	struct	 window *win;		/* Tool settings window */
};

#define	TOOL(t)		((struct tool *)(t))
#define TOOL_OPS(t)	((struct tool_ops *)OBJECT((t))->ops)

#define TOOL_DIALOG_X	16
#define TOOL_DIALOG_Y	176	/* XXX */

void		tool_init(struct tool *, char *, struct mapedit *,
		     const void *);
struct mapview *tool_mapview(void);

