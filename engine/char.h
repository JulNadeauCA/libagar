/*	$Csoft: char.h,v 1.3 2002/01/30 14:29:32 vedge Exp $	*/

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

	int	direction;	/* Motion */
#define CHAR_UP		0x01
#define CHAR_DOWN	0x02
#define CHAR_LEFT	0x04
#define CHAR_RIGHT	0x08

	int	flags;		/* Current state */
#define CHAR_FOCUS	0x0001	/* Being controlled/viewed */
#define CHAR_ANIM	0x0002	/* Animation in progress */
#define CHAR_DASH	0x0010	/* Boost timer temporarily */
	
	SDL_TimerID timer;
};

/* Set the active sprite. */
#define CHAR_SETSPRITE(chp, soffs)			\
	do {						\
		(chp)->flags &= ~(CHAR_ANIM);		\
		(chp)->curoffs = (soffs);		\
	} while (0)

/* Set the active anim. */
#define CHAR_SETANIM(chp, aoffs)			\
	do {						\
		(chp)->flags |= CHAR_ANIM;		\
		(chp)->curoffs = (aoffs);		\
	} while (0)

/* Position character obp at m:x,y. */
#define CHAR_PLOT(chp, pma, mx, my)				\
	do {							\
		(chp)->map = (pma);				\
		(chp)->x = (mx);				\
		(chp)->y = (my);				\
		MAP_ADDSPRITE((pma), (mx), (my),		\
		    (struct object *)(chp), (chp)->curoffs);	\
	} while (0)

/* Move character to a new position. */
#define CHAR_MOVE(chp, nx, ny)						\
	do {				    				\
		MAP_DELREF((chp)->map, (chp)->x, (chp)->y,(chp), -1);	\
		if ((chp)->flags & CHAR_ANIM) {				\
			MAP_ADDANIM((chp)->map, nx, ny,			\
			    (struct object *)(chp), (chp)->curoffs);	\
		} else {						\
			MAP_ADDSPRITE((chp)->map, nx, ny,		\
			    (struct object *)(chp), (chp)->curoffs); 	\
		}							\
		(chp)->x = nx;						\
		(chp)->y = ny;						\
	} while (0)

struct character *char_create(char *, char *, int, int, int);

void	 char_destroy(struct object *);
int	 char_link(void *);
void	 char_event(struct object *, SDL_Event *);
void	 char_setspeed(struct character *, Uint32);
#ifdef DEBUG
void	 char_dump_char(void *, void *);
#endif

