/*	$Csoft: tool.h,v 1.7 2003/02/04 02:35:39 vedge Exp $	*/
/*	Public domain	*/

struct window;
struct mapview;

struct tool_ops {
	const struct object_ops	obops;

	struct window	*(*window)(void *);
	int		 (*cursor)(void *, struct mapview *, SDL_Rect *);
	void		 (*effect)(void *, struct mapview *, struct node *);
	void		 (*mouse)(void *, struct mapview *, Sint16, Sint16,
			          Uint8);
};

struct tool {
	struct object	 obj;
	int		 flags;
#define TOOL_NO_EDIT	0x01		/* Effect when edition is disabled */
	char		*type;
	struct window	*win;		/* Tool settings window */
	struct button	*button;	/* Back pointer to button */
};

#define	TOOL(t)		((struct tool *)(t))
#define TOOL_OPS(t)	((struct tool_ops *)OBJECT((t))->ops)

#define TOOL_DIALOG_X	16
#define TOOL_DIALOG_Y	205

void		tool_init(struct tool *, char *, const void *);
struct mapview *tool_mapview(void);

