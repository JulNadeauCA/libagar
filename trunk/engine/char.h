/*	$Csoft: char.h,v 1.7 2002/02/05 05:49:03 vedge Exp $	*/

/*
 * Data shared by all character types.
 */
struct character {
	struct	object obj;	/* Generic object */
	int	level;		/* Current level */
	double	exp;		/* Experience */
	double	age;		/* Age (years) */
	int	maxhp, maxmp;	/* Maximum HP/MP */
	int	hp, mp;		/* Effective HP/MP */
	long	seed;		/* Random seed */
	int	effect;		/* Effects */
#define EFFECT_REGEN		0x0001
#define EFFECT_POISON		0x0002
#define EFFECT_DOOM		0x0004

	struct	map *map;	/* Map */
	struct	mapdir dir;	/* Direction in map */
	int	x, y;		/* Coordinates in map */

	int	curoffs;	/* Current sprite or anim */
	int	curspeed;	/* Current speed */
	int	maxspeed;	/* Max. speed in ms */

	int	flags;		/* Current state */
#define CHAR_FOCUS	0x0001	/* Being controlled/viewed */
#define CHAR_ONMAP	0x0002
#define CHAR_ANIM	0x0004	/* Animation in progress */
#define CHAR_DASH	0x0010	/* Boost timer temporarily */
	
	SDL_TimerID timer;
	SLIST_ENTRY(character) wchars;	/* Active characters */

	void	 (*event_hook)(struct character *, SDL_Event *);
};

extern struct character *curchar;	/* Controlled character */

struct character *char_create(char *, char *, int, int, int);

/* Sprites */
#define CHAR_ICON	0
#define CHAR_IDLE	1
#define CHAR_UP		2
#define CHAR_DOWN	3
#define CHAR_LEFT	4
#define CHAR_RIGHT	5

/* Animations */
#define CHAR_WALKUP	0
#define CHAR_WALKDOWN	1
#define CHAR_WALKLEFT	2
#define CHAR_WALKRIGHT	3


int	char_setanim(struct character *, int);
int	char_setsprite(struct character *, int);
int	char_add(struct character *, struct map *, int, int);
int	char_canmove(struct character *, int, int);
int	char_move(struct character *, int, int);
int	char_del(struct character *, struct map *, int, int);

void	char_destroy(struct object *);
int	char_link(void *);
void	char_setspeed(struct character *, Uint32);

int	char_focus(struct character *);
int	char_unfocus(struct character *);

#ifdef DEBUG
void	char_dump(struct character *);
#endif

