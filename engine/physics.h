/*	$Csoft: physics.h,v 1.18 2002/12/13 12:17:45 vedge Exp $	    */
/*	Public domain	*/

struct noderef;
struct input;

#define DIR_UP		0x01
#define DIR_DOWN	0x02
#define DIR_LEFT	0x04
#define DIR_RIGHT	0x08
#define DIR_ALL		0xff

/* 2D movement of undefined constructs in an infinite area. */
struct gendir {
	Uint32	set;		/* Set direction mask (move) */
	Uint32	current;	/* Current direction mask (moving) */
	Uint32	clear;		/* Clear direction mask (stop move) */
	Uint32	moved;		/* Post direction mask (moved) */
	Uint32	offs;		/* Timing for repeat */
	Uint32	noffs;		/* Timing for delay */
};

/* 2D movement of map references between adjacent map nodes. */
struct mapdir {
	Uint32	speed;		/* Soft-scroll increments */

	struct object	*ob;	/* Object back pointer */
	struct map	*map;	/* Map back pointer */

	Uint32	set;		/* Set direction mask (move) */
	Uint32	current;	/* Current direction mask (moving) */
	Uint32	clear;		/* Clear direction mask (stop move) */
	Uint32	moved;		/* Post direction mask (moved) */

	Uint32	flags;
#define DIR_SCROLLVIEW	0x01	/* Perform centering/soft-scrolling. */
#define DIR_SOFTSCROLL	0x02	/* Animate all node->node displacements. */
#define DIR_PASSTHROUGH	0x08	/* Pass through all nodes. */
};

/* Position on a map. */
struct mappos {
	struct map	*map;	/* Map (or NULL) */
	int		 x, y;	/* Map coordinates */
	struct noderef	*nref;	/* Node reference */
	struct mapdir	 dir;	/* Map direction (not saved) */
	struct input	*input;	/* Controller (or NULL) */
};

void	gendir_init(struct gendir *);
void	gendir_set(struct gendir *, Uint32);
void	gendir_unset(struct gendir *, Uint32);
Uint32	gendir_move(struct gendir *);
void	gendir_postmove(struct gendir *, Uint32);

void	mapdir_init(struct mapdir *, struct object *, struct map *, Uint32,
	    Uint32);
void	mapdir_set(struct mapdir *, Uint32);
void	mapdir_unset(struct mapdir *, Uint32);
int	mapdir_move(struct mapdir *, int *, int *);
void	mapdir_postmove(struct mapdir *, int *, int *, Uint32);

