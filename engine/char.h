/*	$Csoft: char.h,v 1.8 2002/02/14 05:23:52 vedge Exp $	*/

enum {
	CHAR_VERMAJ =	1,
	CHAR_VERMIN =	0
};

struct character {
	struct	object obj;	/* Generic object */

	Uint32	level;		/* Current level */
	Uint32	exp;		/* Experience */
	Uint32	age;		/* Age */
	Uint64	seed;		/* Random seed */
	
	Uint32	flags;
#define CHAR_FOCUS	0x0001	/* Being controlled/viewed */
#define CHAR_ONMAP	0x0002	/* Assume position on a map */
#define CHAR_ANIM	0x0004	/* Animation in progress */
#define CHAR_DASH	0x0010	/* Boost timer temporarily */
#define CHAR_REGEN	0x0020	/* HP regeneration */
#define CHAR_POISON	0x0040	/* HP decay */
#define CHAR_DOOM	0x0080	/* Doom */
#define CHAR_DONTSAVE	(CHAR_FOCUS|CHAR_ANIM|CHAR_DASH)
	
	Uint32	maxhp, maxmp;	/* Maximum HP/MP */
	Uint32	hp, mp;		/* Effective HP/MP */

	struct	map *map;	/* Map */
	struct	mapdir dir;	/* Direction in map */
	Uint32	x, y;		/* Coordinates in map */

	Uint32	curoffs;	/* Current sprite or anim */
	Uint32	curspeed;	/* Current speed */
	Uint32	maxspeed;	/* Max. speed in ms */

	SDL_TimerID timer;
	SLIST_ENTRY(character) wchars;	/* Active characters */
};

extern struct character *curchar;	/* Controlled character */

/* Sprites */
enum {
	CHAR_ICON,
	CHAR_IDLE,
	CHAR_UP,
	CHAR_DOWN
};

/* Animations */
enum {
	CHAR_WALKUP,
	CHAR_WALKDOWN,
	CHAR_WALKLEFT,
	CHAR_WALKRIGHT
};

struct character *char_create(char *, char *, int, int, int);
int		  char_destroy(void *);
void		  char_event(void *, SDL_Event *);
int		  char_load(void *, int);
int		  char_save(void *, int);
int		  char_link(void *);
int		  char_unlink(void *);
#ifdef DEBUG
void		  char_dump(struct character *);
#endif

int		  char_focus(struct character *);
int		  char_unfocus(struct character *);
int		  char_setanim(struct character *, int);
int		  char_setsprite(struct character *, int);
int		  char_add(struct character *, struct map *, int, int);
int		  char_canmove(struct character *, int, int);
int		  char_move(struct character *, int, int);
int		  char_del(struct character *, struct map *, int, int);
void		  char_setspeed(struct character *, Uint32);

