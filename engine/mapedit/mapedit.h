/*	$Csoft: mapedit.h,v 1.2 2002/01/25 15:06:52 vedge Exp $	*/

struct editref {
	int	animi;		/* Index into the object's real anim list. */
	int	spritei;	/* Index into the object's real sprite list */
	void	*p;
	enum {
		EDITREF_SPRITE,	/* SDL_Surface */
		EDITREF_ANIM	/* struct anim */
	} type;
};

struct editobj {
	struct	object *pobj;		/* Original object structure */
	GSList	*refs;			/* Sprite/anim references */
	int	nrefs;
	int	nsprites;
	int	nanims;
};

struct mapedit {
	struct	object obj;

	struct	window *tilelist;	/* Tile list (right) */
	struct	window *tilestack;	/* Tile stack (left) */
	struct	window *objlist;	/* Object list (top) */

	GSList	*eobjs;			/* Object references */
	int	neobjs;
	int	curoffs;		/* Default reference index */
	int	curflags;		/* Default map entry flags */
	struct	editobj *curobj;	/* Default object */
	int	flags;	
#define MAPEDIT_TILELIST	0x01	/* Display tile list window */
#define MAPEDIT_TILESTACK	0x02	/* Display tile stack window */
#define MAPEDIT_OBJLIST		0x04	/* Display object list window */

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
};

/* Move mapedit to a new position. */
#define MAPEDIT_MOVE(medp, nx, ny)					\
	do {				    				\
		MAP_DELREF((medp)->map, (medp)->x, (medp)->y,		\
		    (medp), MAPEDIT_SELECT); 				\
		MAP_ADDANIM((medp)->map, nx, ny,			\
		    (medp), MAPEDIT_SELECT);				\
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
		    &(pma)->obj, MAPEDIT_SELECT);		\
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
#define MAPEDIT_ANIM	12

struct mapedit *mapedit_create(char *, char *);
void		mapedit_tilelist(struct mapedit *);
void		mapedit_tilestack(struct mapedit *);
void		mapedit_objlist(struct mapedit *);

extern struct mapedit *curmapedit;

