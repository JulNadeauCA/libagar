/*	$Csoft: char.h,v 1.21 2002/06/03 18:33:04 vedge Exp $	*/

#include <engine/ailment.h>

struct character {
	struct	object obj;

	Uint32	level;		/* Current level */
	Uint32	exp;		/* Experience */
	Uint32	age;		/* Age */
	Uint32	seed;		/* Random seed */
	
	Uint32	flags;
#define CHAR_FOCUS	0x0001	/* Focus on character */
#define CHAR_DASH	0x0010	/* Boost timer temporarily */
#define CHAR_DONTSAVE	(CHAR_DASH)

	struct	ailmentq ailments;	/* Status ailments */

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

struct character *char_new(char *, char *);
void		  char_init(struct character *, char *, char *);
int		  char_load(void *, int);
int		  char_save(void *, int);
void		  char_onattach(int, union evarg *);
void		  char_ondetach(int, union evarg *);

