/*	$Csoft: char.h,v 1.5 2002/02/01 02:03:34 vedge Exp $	*/

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

	struct	map *map;
	int	x, y;

	int	curoffs;	/* Current sprite or anim */
	int	curspeed;	/* Current speed */
	int	maxspeed;	/* Max. speed in ms */

	int	flags;		/* Current state */
#define CHAR_FOCUS	0x0001	/* Being controlled/viewed */
#define CHAR_ANIM	0x0002	/* Animation in progress */
#define CHAR_DASH	0x0010	/* Boost timer temporarily */
	
	SDL_TimerID timer;
};

struct character *char_create(char *, char *, int, int, int);

int	char_setanim(struct character *, int);
int	char_setsprite(struct character *, int);
int	char_add(struct character *, struct map *, int, int);
int	char_canmove(struct character *, int, int);
int	char_move(struct character *, int, int);
int	char_del(struct character *, struct map *, int, int);

void	char_destroy(struct object *);
int	char_link(void *);
void	char_event(struct object *, SDL_Event *);
void	char_setspeed(struct character *, Uint32);
#ifdef DEBUG
void	char_dump_char(void *, void *);
#endif

