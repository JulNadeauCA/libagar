/*	$Csoft: char.h,v 1.18 2002/04/24 05:17:15 vedge Exp $	*/

struct character {
	struct	object obj;

	Uint32	level;		/* Current level */
	Uint32	exp;		/* Experience */
	Uint32	age;		/* Age */
	Uint32	seed;		/* Random seed */
	
	Uint32	flags;
#define CHAR_FOCUS	0x0001	/* Focus on character */
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

	pthread_mutex_t	lock;	/* Lock on whole structure */
};

/* Sprites */
enum {
	CHAR_ICON,
	CHAR_IDLE,
	CHAR_UP,
	CHAR_DOWN
};

#define CHARACTER(ob)	((struct character *)(ob))

void		  char_init(struct character *, char *, char *);
int		  char_load(void *, int);
int		  char_save(void *, int);
int		  char_link(void *);
int		  char_unlink(void *);

