/*	$Csoft: mapedit.h,v 1.10 2002/02/14 06:32:07 vedge Exp $	*/

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

struct mapedit {
	struct	object obj;

	struct	window *tilelist;	/* Tile list (right) */
	struct	window *tilestack;	/* Tile stack (left) */
	struct	window *objlist;	/* Object list (top) */

	struct	eobjs_head eobjsh;	/* Editor object references */
	int	neobjs;

	int	curoffs;		/* Default reference index */
	int	curflags;		/* Default map entry flags */
	struct	editobj *curobj;	/* Default object */
	int	flags;	
#define MAPEDIT_TILELIST	0x01	/* Display tile list window */
#define MAPEDIT_TILESTACK	0x02	/* Display tile stack window */
#define MAPEDIT_OBJLIST		0x04	/* Display object list window */
#define MAPEDIT_DRAWGRID	0x08	/* Draw a grid on the map */
#define MAPEDIT_DRAWPROPS	0x10	/* Draw a grid on the map */

	struct	map *map;		/* Map being edited */
	int	x, y;			/* Cursor position */
	int	mmapx, mmapy;		/* Mouse coordinates */

	struct	mapdir cursor_dir;	/* Cursor direction */
	struct	gendir listw_dir;	/* Tile list window direction */
	struct	gendir olistw_dir;	/* Obj list window direction */

	SDL_TimerID timer;

	void	 (*event_hook)(struct mapedit *, SDL_Event *);
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

struct mapedit *mapedit_create(char *, char *, int, int);
void		mapedit_tilelist(struct mapedit *);
void		mapedit_tilestack(struct mapedit *);
void		mapedit_objlist(struct mapedit *);
void		mapedit_move(struct mapedit *, int, int);

extern struct mapedit *curmapedit;	/* Controlled map editor */

