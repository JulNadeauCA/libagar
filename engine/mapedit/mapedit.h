/*	$Csoft: mapedit.h,v 1.46 2002/07/08 03:15:53 vedge Exp $	*/
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

SIMPLEQ_HEAD(erefs_head, editref);

struct editobj {
	struct	object *pobj;		/* Original object structure */
	struct	erefs_head erefsh;	/* Reference list */
	int	nrefs;
	int	nsprites;
	int	nanims;

	TAILQ_ENTRY(editobj) eobjs;	/* Editable object list */
};

TAILQ_HEAD(eobjs_head, editobj);

struct mapdir;
struct gendir;
struct tool;

struct mapedit {
	struct	 object obj;

	Uint32	 flags;

	struct	 window *toolbar_win;
	struct	 window *objlist_win;
	struct	 window *tileq_win;
	struct	 window *new_map_win;
	struct	 window *load_map_win;
	struct	 window *coords_win;
	struct	 label *coords_label;

	struct	 tool *curtool;
	struct {
		struct	tool *stamp;
		struct	tool *eraser;
	} tools;

	struct	 eobjs_head eobjsh;	/* Shadow object tree */
	int	 neobjs;

	struct	 editobj *curobj;
	int	 curoffs;
	int	 curflags;
};

/* Editor sprites */
enum {
	/* Map editor */
	MAPEDIT_ICON,
	MAPEDIT_ORIGIN,
	MAPEDIT_CIRQSEL,
	MAPEDIT_GRID,
	MAPEDIT_BLOCKED,
	MAPEDIT_WALK,
	MAPEDIT_CLIMB,
	MAPEDIT_SLIP,
	MAPEDIT_BIO,
	MAPEDIT_REGEN,
	MAPEDIT_SLOW,
	MAPEDIT_HASTE,
	MAPEDIT_ANIM,
	MAPEDIT_OVERLAP,
	MAPEDIT_NVEL,
	MAPEDIT_SVEL,
	MAPEDIT_WVEL,
	MAPEDIT_EVEL,
	MAPEDIT_STATE,
	MAPEDIT_INSERT_TXT,
	MAPEDIT_REPLACE_TXT,
	MAPEDIT_PROPS_TXT,
	MAPEDIT_GRID_TXT,
	MAPEDIT_ANIM_TXT,
	MAPEDIT_ANIM_INDEPENDENT_TXT,
	MAPEDIT_ANIM_DELTA_TXT,
	/* Toolbar */
	MAPEDIT_TOOL_MAP,
	MAPEDIT_TOOL_NEW_MAP,
	MAPEDIT_TOOL_LOAD_MAP,
	MAPEDIT_TOOL_SAVE_MAP,
	MAPEDIT_TOOL_SAVE_MAP_TO,
	MAPEDIT_TOOL_TILEQ,
	MAPEDIT_TOOL_TILESTACK,
	MAPEDIT_TOOL_OBJLIST,
	MAPEDIT_TOOL_STAMP,
	MAPEDIT_TOOL_ERASER,
	MAPEDIT_TOOL_MAPOPS,
	MAPEDIT_TOOL_CLEAR_MAP
};

void	mapedit_init(struct mapedit *, char *);
int	mapedit_load(void *, int);
int	mapedit_save(void *, int);

void	mapedit_key(struct mapedit *, SDL_Event *);
void	mapedit_move(struct mapedit *, Uint32, Uint32);

void	mapedit_sticky(struct mapedit *);

extern struct mapedit *curmapedit;	/* Controlled map editor */

