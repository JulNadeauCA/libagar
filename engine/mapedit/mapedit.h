/*	$Csoft: mapedit.h,v 1.22 2002/03/03 06:26:54 vedge Exp $	*/

#include <engine/physics.h>

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
	int	 tilew;
	int	 tileh;
};

struct mapedit {
	struct	object obj;
	
	int	flags;	
#define MAPEDIT_DRAWGRID	0x01	/* Draw a grid on the map */
#define MAPEDIT_DRAWPROPS	0x02	/* Draw tile properties (slow) */
#define MAPEDIT_INSERT		0x04	/* Insert mode (else replace) */

	struct	mapedit_margs margs;	/* Map creation arguments */
	struct	map *map;		/* Map being edited */
	Uint32	x, y;			/* Cursor position */
	Uint32	mmapx, mmapy;		/* Mouse coordinates */

	Uint32	cursor_speed;		/* Cursor speed in ms */
	Uint32	listw_speed;		/* List scrolling speed in ms */

	struct	eobjs_head eobjsh;	/* Editor object references */
	int	neobjs;

	struct	editobj *curobj;	/* Default object */
	int	curoffs;		/* Default reference index */
	int	curflags;		/* Default map entry flags */

	struct	mapdir cursor_dir;	/* Cursor direction */
	struct	gendir listw_dir;	/* Tile list window direction */
	struct	gendir olistw_dir;	/* Obj list window direction */
	
	SDL_Rect	tilelist;	/* Tile list (right) */
	SDL_Rect	tilestack;	/* Tile stack (left) */
	SDL_Rect	objlist;	/* Object list (top) */

	SDL_TimerID timer;
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
	MAPEDIT_NVEL,
	MAPEDIT_SVEL,
	MAPEDIT_WVEL,
	MAPEDIT_EVEL,
	MAPEDIT_STATE,
	MAPEDIT_INSERT_TXT,
	MAPEDIT_REPLACE_TXT,
	MAPEDIT_PROPS_TXT,
	MAPEDIT_GRID_TXT
};

struct mapedit *mapedit_create(char *);
int		mapedit_link(void *);
int		mapedit_unlink(void *);
int		mapedit_destroy(void *);
int		mapedit_load(void *, int);
int		mapedit_save(void *, int);
void		mapedit_event(void *, SDL_Event *);
void		mapedit_tilelist(struct mapedit *);
void		mapedit_tilestack(struct mapedit *);
void		mapedit_objlist(struct mapedit *);
void		mapedit_move(struct mapedit *, Uint32, Uint32);
void		mapedit_predraw(struct map *, Uint32, Uint32, Uint32);
void		mapedit_postdraw(struct map *, Uint32, Uint32, Uint32);

void		mapedit_setcaption(struct mapedit *, char *);
void		mapedit_sticky(struct mapedit *);

extern struct mapedit *curmapedit;	/* Controlled map editor */

