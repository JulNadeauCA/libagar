/*	$Csoft: mapedit.h,v 1.37 2002/06/12 20:40:08 vedge Exp $	*/
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
	pthread_mutex_t	lock;		/* Lock on object list */
};

TAILQ_HEAD(eobjs_head, editobj);

struct mapedit_margs {
	char	*name;
	char	*desc;
	int	 mapw;
	int	 maph;
};

struct mapdir;
struct gendir;

struct mapedit {
	struct	object obj;
	
	Uint32	flags;	
#define MAPEDIT_DRAWGRID	0x01	/* Draw a grid on the map (slow) */
#define MAPEDIT_DRAWPROPS	0x02	/* Draw tile properties (slow) */
#define MAPEDIT_INSERT		0x04	/* Insert mode (else replace) */

	struct	mapedit_margs margs;	/* Map creation arguments */
	struct	map *map;		/* Map being edited */
	Uint32	x, y;			/* Cursor position */
	int	mmapx, mmapy;		/* Map view coordinates */
	int	mtmapx, mtmapy;		/* Lists coordinates */
	int	redraw;			/* Redraw lists and map */

	struct	eobjs_head eobjsh;	/* Editor object references */
	int	neobjs;

	struct	editobj *curobj;	/* Default object */
	int	curoffs;		/* Default reference index */
	int	curflags;		/* Default map entry flags */

	struct	mapdir cursor_dir;	/* Cursor direction */
	Uint32	cursor_speed;		/* Cursor speed in ms */
	Uint32	listw_speed;		/* List scrolling speed in ms */
	
	/* Tile stack (left) */
	SDL_Rect tilestack;		/* Region */

	/* Tile list (right) */
	SDL_Rect tilelist;		/* Region */
	struct	 gendir listw_dir;	/* Scrolling direction */
	int	 tilelist_offs;

	/* Obj list (top) */
	SDL_Rect objlist;		/* Region */
	struct	 gendir olistw_dir;	/* Scrolling direction */
	int	 objlist_offs;

	struct	 window *settings_win;
	struct	 window *coords_win;
	struct	 label *coords_label;

	/*
	 * Map must be locked before this mutex is acquired, otherwise
	 * deadlock occurs.
	 */
	pthread_mutex_t	lock;		/* Lock on whole structure */
};

/* Editor anims */
enum {
	MAPEDIT_SELECT
};

/* Editor sprites */
enum {
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
};

#define MAPEDIT_PREDRAW(m, node, vx, vy) do {				\
		if (curmapedit != NULL) {				\
			mapedit_predraw((m), (node)->flags, (vx), (vy));\
		}							\
	} while (0)

#define MAPEDIT_POSTDRAW(m, node, vx, vy) do {				\
		if (curmapedit != NULL) {				\
			mapedit_postdraw((m), (node)->flags, (vx), (vy));\
		}							\
	} while (0)

void	mapedit_init(struct mapedit *, char *);
int	mapedit_load(void *, int);
int	mapedit_save(void *, int);
void	mapedit_event(void *, SDL_Event *);
void	mapedit_move(struct mapedit *, Uint32, Uint32);
void	mapedit_predraw(struct map *, Uint32, Uint32, Uint32);
void	mapedit_postdraw(struct map *, Uint32, Uint32, Uint32);

void	mapedit_sticky(struct mapedit *);

extern struct mapedit *curmapedit;	/* Controlled map editor */

