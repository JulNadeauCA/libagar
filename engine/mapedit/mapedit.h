/*	$Csoft: mapedit.h,v 1.15 2002/02/18 01:29:40 vedge Exp $	*/

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

	struct	mapedit_margs margs;	/* Map creation arguments */
	struct	map *map;		/* Map being edited */
	int	x, y;			/* Cursor position */
	int	mmapx, mmapy;		/* Mouse coordinates */

	int	cursor_speed;		/* Cursor speed in ms */
	int	listw_speed;		/* List scrolling speed in ms */

	struct	eobjs_head eobjsh;	/* Editor object references */
	int	neobjs;

	struct	editobj *curobj;	/* Default object */
	int	curoffs;		/* Default reference index */
	int	curflags;		/* Default map entry flags */

	struct	mapdir cursor_dir;	/* Cursor direction */
	struct	gendir listw_dir;	/* Tile list window direction */
	struct	gendir olistw_dir;	/* Obj list window direction */
	
	struct	window *tilelist;	/* Tile list (right) */
	struct	window *tilestack;	/* Tile stack (left) */
	struct	window *objlist;	/* Object list (top) */

	SDL_TimerID timer;
};

/* Editor anims */
#define MAPEDIT_SELECT	0

/* Editor sprites */
#define MAPEDIT_ORIGIN	1
#define MAPEDIT_CIRQSEL	2
#define MAPEDIT_GRID	3
#define MAPEDIT_BLOCKED	4
#define MAPEDIT_WALK	5
#define MAPEDIT_CLIMB	6
#define MAPEDIT_SLIP	7
#define MAPEDIT_BIO	8
#define MAPEDIT_REGEN	9
#define MAPEDIT_SLOW	10
#define MAPEDIT_HASTE	11
#define MAPEDIT_ANIM	12
#define MAPEDIT_NVEL	13
#define MAPEDIT_SVEL	14
#define MAPEDIT_WVEL	15
#define MAPEDIT_EVEL	16

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
void		mapedit_move(struct mapedit *, int, int);
void		mapedit_predraw(struct map *, int, int, int);
void		mapedit_postdraw(struct map *, int, int, int);
void		mapedit_sticky(struct mapedit *);

extern struct mapedit *curmapedit;	/* Controlled map editor */

