/*	$Csoft$	*/

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

	int	curspeed;	/* Current speed */
	int	maxspeed;	/* Max. speed in ms */

	int	direction;	/* Motion */
#define CHAR_UP		0x01
#define CHAR_DOWN	0x02
#define CHAR_LEFT	0x04
#define CHAR_RIGHT	0x08

	int	flags;		/* Current state */
#define CHAR_FOCUS	0x0001	/* Being controlled/viewed */
#define CHAR_ZOMBIE	0x0002	/* Zombie character */
#define CHAR_ACTIVE	0x0004	/* Active character */
#define CHAR_SMOTION	0x0008	/* Sticky motion */
#define CHAR_DASH	0x0010	/* Boost timer */
	
	SDL_TimerID timer;
};

#define WFLAG_MOVEDUP		0x01
#define WFLAG_MOVEDDOWN		0x02
#define WFLAG_MOVEDLEFT		0x04
#define WFLAG_MOVEDRIGHT	0x08

struct character *char_create(char *, char *, int, int, struct map *, int);

void	 char_destroy(struct object *);
int	 char_link(void *);
void	 char_event(struct object *, SDL_Event *);
void	 char_setspeed(struct character *, Uint32);
#ifdef DEBUG
void	 char_dump_char(void *, void *);
#endif

