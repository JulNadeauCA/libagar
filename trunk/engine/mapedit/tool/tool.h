/*	$Csoft: tool.h,v 1.9 2003/03/16 03:57:57 vedge Exp $	*/
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

struct tool_binding {
	const char	 *name;
	SDLMod		  mod;
	SDLKey		  key;
	int		  edit;			/* Require MAPVIEW_EDIT? */
	void		(*func)(void *, struct mapview *);
	SLIST_ENTRY(tool_binding) bindings;
};

struct tool {
	struct object	 obj;
	char		*type;
	struct window	*win;		/* Tool settings window */
	struct button	*button;	/* Trigger */
	SLIST_HEAD(,tool_binding) bindings;
};

#define	TOOL(t)		((struct tool *)(t))
#define TOOL_OPS(t)	((struct tool_ops *)OBJECT((t))->ops)

#define TOOL_DIALOG_X	16
#define TOOL_DIALOG_Y	205

void		tool_init(struct tool *, char *, const void *);
void		tool_destroy(void *);
void		tool_bind_key(void *, SDLMod, SDLKey,
		    void (*)(void *, struct mapview *), int);
struct mapview *tool_mapview(void);

