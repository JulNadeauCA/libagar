/*	$Csoft: mapedit.h,v 1.62 2003/01/20 12:06:57 vedge Exp $	*/
/*	Public domain	*/

struct mapdir;
struct gendir;
struct tool;

struct mapedit {
	struct object	obj;

	struct window	*toolbar_win;
	struct window	*objlist_win;
	struct window	*new_map_win;
	struct window	*load_map_win;
	struct {
		struct tool	*stamp;
		struct tool	*eraser;
		struct tool	*magnifier;
		struct tool	*resize;
		struct tool	*propedit;
		struct tool	*select;
	} tools;
	struct tool	*curtool;		/* Selected tool */
	struct node	*src_node;		/* Selected source node */
};

enum {
	/* Map editor */
	MAPEDIT_ICON,
	MAPEDIT_ANIM,
	MAPEDIT_OVERLAP,
	MAPEDIT_NVEL,
	MAPEDIT_SVEL,
	MAPEDIT_WVEL,
	MAPEDIT_EVEL,
	MAPEDIT_ANIM_TXT,
	MAPEDIT_ANIM_INDEPENDENT_TXT,
	MAPEDIT_ANIM_DELTA_TXT,
	/* Props */
	MAPEDIT_FRAME_0,
	MAPEDIT_FRAME_1,
	MAPEDIT_FRAME_2,
	MAPEDIT_FRAME_3,
	MAPEDIT_FRAME_4,
	MAPEDIT_FRAME_5,
	MAPEDIT_FRAME_6,
	MAPEDIT_BLOCK,
	MAPEDIT_ORIGIN,
	MAPEDIT_WALK,
	MAPEDIT_CLIMB,
	MAPEDIT_SLIPPERY,
	MAPEDIT_BIO,
	MAPEDIT_REGEN,
	MAPEDIT_SLOW,
	MAPEDIT_HASTE,
	/* Toolbar */
	MAPEDIT_TOOL_MAP,
	MAPEDIT_TOOL_NEW_MAP,
	MAPEDIT_TOOL_LOAD_MAP,
	MAPEDIT_TOOL_SAVE_MAP,
	MAPEDIT_TOOL_SAVE_MAP_TO,
	MAPEDIT_TOOL_TILESTACK,
	MAPEDIT_TOOL_OBJLIST,
	MAPEDIT_TOOL_STAMP,
	MAPEDIT_TOOL_ERASER,
	MAPEDIT_TOOL_MAPOPS,
	MAPEDIT_TOOL_CLEAR_MAP,
	MAPEDIT_TOOL_NEW_VIEW,
	MAPEDIT_TOOL_MAGNIFIER,
	MAPEDIT_TOOL_RESIZE,
	MAPEDIT_TOOL_GRID,
	MAPEDIT_TOOL_PROPS,
	MAPEDIT_TOOL_PROPEDIT,
	MAPEDIT_TOOL_SHOW_CURSOR,
	MAPEDIT_TOOL_EDIT,
	MAPEDIT_TOOL_RIGHT,
	MAPEDIT_TOOL_LEFT,
	MAPEDIT_TOOL_UP,
	MAPEDIT_TOOL_DOWN,
	MAPEDIT_TOOL_NODEEDIT,
	MAPEDIT_TOOL_SELECT
};

void	 mapedit_init(struct mapedit *, char *);
void	 mapedit_attached(int, union evarg *);
void	 mapedit_detached(int, union evarg *);

extern struct mapedit *mapedit;

