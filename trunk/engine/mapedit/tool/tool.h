/*	$Csoft$	*/
/*	Public domain	*/

struct window;

struct tool_ops {
	const	 struct object_ops obops;

	struct	 window	*(*tool_window)(void *);
	void	 	 (*tool_effect)(void *, struct map *, Uint32, Uint32);
};

struct tool {
	struct	 object obj;

	Uint32	 flags;

	struct	 mapedit *med;		/* Map editor */
	struct	 window *win;		/* Tool settings window */
};

#define	TOOL(t)		((struct tool *)(t))
#define TOOL_OPS(t)	((struct tool_ops *)OBJECT((t))->ops)

void	tool_init(struct tool *, char *, struct mapedit *, const void *);

