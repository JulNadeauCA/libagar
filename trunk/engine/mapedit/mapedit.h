/*	$Csoft: mapedit.h,v 1.5 2002/02/05 05:59:18 vedge Exp $	*/

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

	/* Directions */
	int	cursdir;		/* Cursor */
	int	listwdir;		/* Tile list (vert) */
	int	listsdir;		/* Tile stack (vert) */
	int	listodir;		/* Object list (horiz) */
#define MAPEDIT_UP		0x01
#define MAPEDIT_DOWN		0x02
#define MAPEDIT_LEFT		0x04
#define MAPEDIT_RIGHT		0x08
#define	MAPEDIT_PAGEUP		0x10
#define	MAPEDIT_PAGEDOWN	0x20
#define MAPEDIT_CTRLLEFT	0x40
#define MAPEDIT_CTRLRIGHT	0x80

	SDL_TimerID timer;

	void	 (*event_hook)(struct mapedit *, SDL_Event *);
};

/* Move mapedit to a new position. */
#define MAPEDIT_MOVE(medp, nx, ny)					\
	do {				    				\
		MAP_DELREF((medp)->map, (medp)->x, (medp)->y,		\
		    (struct object *)(medp), MAPEDIT_SELECT);		\
		MAP_ADDANIM((medp)->map, nx, ny,			\
		    (struct object *)(medp), MAPEDIT_SELECT);		\
		(medp)->x = nx;						\
		(medp)->y = ny;						\
	} while (0)

/* Position mapedit at m:x,y. */
#define MAPEDIT_PLOT(med, pma, mx, my)				\
	do {							\
		(med)->map = (pma);				\
		(med)->x = (mx);				\
		(med)->y = (my);				\
		MAP_ADDANIM((pma), (mx), (my),			\
		    (struct object *)(med), MAPEDIT_SELECT);	\
	} while (0)

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

struct mapedit *mapedit_create(char *, char *);
void		mapedit_tilelist(struct mapedit *);
void		mapedit_tilestack(struct mapedit *);
void		mapedit_objlist(struct mapedit *);

extern struct mapedit *curmapedit;	/* Controlled map editor */

