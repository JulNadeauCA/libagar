/*	$Csoft: mapedit.h,v 1.1.1.1 2002/01/25 09:50:02 vedge Exp $	*/

/*
 * The map edition code references sprites and animations in a
 * contiguous list, and interpose a structure for browsing purposes.
 *
 * The engine does not interpose structures, but uses map entries
 * to distinguish sprites from animations. Offsets have different
 * meaning depending on map entry flags.
 */

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

	int	mmapx, mmapy;
	int	listwdir;
	int	listsdir;
	int	listodir;
	int	cursdir;
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

struct mapedit *mapedit_create(struct map *, int, int);
void		mapedit_tilelist(struct mapedit *);
void		mapedit_tilestack(struct mapedit *);
void		mapedit_objlist(struct mapedit *);

extern struct mapedit *curmapedit;

