/*	$Csoft: tool.h,v 1.4 2002/07/30 22:18:28 vedge Exp $	*/
/*	Public domain	*/

struct window;
struct mapview;

struct tool_ops {
	const struct object_ops	obops;

	struct window *(*tool_window)(void *);
	void	       (*tool_effect)(void *, struct mapview *, Uint32, Uint32);
	void	       (*tool_cursor)(void *, struct mapview *, Uint32, Uint32);
};

struct tool {
	struct object	 obj;
	char		*type;
	struct window	*win;		/* Tool settings window */
	struct button	*button;	/* Back pointer to button */
};

#define	TOOL(t)		((struct tool *)(t))
#define TOOL_OPS(t)	((struct tool_ops *)OBJECT((t))->ops)

#define TOOL_DIALOG_X	16
#define TOOL_DIALOG_Y	205	/* XXX */

void		tool_init(struct tool *, char *, const void *);
struct mapview *tool_mapview(void);

