/*	$Csoft: char.h,v 1.14 2002/03/17 09:13:48 vedge Exp $	*/

struct character {
	struct	object obj;

	Uint32	level;		/* Current level */
	Uint32	exp;		/* Experience */
	Uint32	age;		/* Age */
	Uint32	seed;		/* Random seed */
	
	Uint32	flags;
#define CHAR_DASH	0x0010	/* Boost timer temporarily */
#define CHAR_REGEN	0x0020	/* HP regeneration */
#define CHAR_POISON	0x0040	/* HP decay */
#define CHAR_DOOM	0x0080	/* Doom */
#define CHAR_DONTSAVE	(CHAR_DASH)
	
	Uint32	maxhp, maxmp;	/* Maximum HP/MP */
	Uint32	hp, mp;		/* Effective HP/MP */
	Uint32	maxspeed;	/* Max. speed in ms (saved) */

	Uint32	nzuars;		/* Zuars */

	SDL_TimerID timer;
	SLIST_ENTRY(character) wchars;	/* Active characters */
};

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

struct character *char_create(char *, char *, Uint32, Uint32, Uint32);
int		  char_destroy(void *);
int		  char_load(void *, int);
int		  char_save(void *, int);
int		  char_link(void *);
int		  char_unlink(void *);

