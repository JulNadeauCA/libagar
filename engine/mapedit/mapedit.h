/*	$Csoft: mapedit.h,v 1.57 2002/09/04 04:41:26 vedge Exp $	*/
/*	Public domain	*/

struct editref {
	int	animi;		/* Index into the object's real anim list. */
	int	spritei;	/* Index into the object's real sprite list */
	void	*p;
	enum {
		EDITREF_SPRITE,	/* SDL_Surface */
		EDITREF_ANIM	/* struct anim */
	} type;
	
	SIMPLEQ_ENTRY(editref) erefs;	/* Reference list */
};

SIMPLEQ_HEAD(erefq, editref);

struct editobj {
	struct object	*pobj;		/* Original object structure */
	struct erefq	 erefs;		/* Reference list */

	int	nrefs;
	int	nsprites;
	int	nanims;

	TAILQ_ENTRY(editobj) eobjs;	/* Editable object list */
};

TAILQ_HEAD(eobjq, editobj);

struct mapdir;
struct gendir;
struct tool;

struct mapedit {
	struct object	obj;
	
	struct window	*toolbar_win;
	struct window	*objlist_win;
	struct window	*new_map_win;
	struct window	*load_map_win;
	struct tool	*curtool;
	struct {
		struct tool	*stamp;
		struct tool	*eraser;
		struct tool	*magnifier;
		struct tool	*resize;
		struct tool	*propedit;
	} tools;

	struct eobjq	 eobjsh;	/* Shadow object tree */
	int		 neobjs;
	struct editobj	*curobj;
	struct {
		struct object	*obj;
		Uint32		 offs;
		Uint32		 flags;
	} ref;
	struct {
		Uint32	flags;
	} node;
};

/* Editor sprites */
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
	MAPEDIT_TOOL_EDIT
};

void	 mapedit_init(struct mapedit *, char *);
void	 mapedit_attached(int, union evarg *);
void	 mapedit_detached(int, union evarg *);

extern struct mapedit *curmapedit;	/* Controlled map editor */

