/*	$Csoft: physics.h,v 1.16 2002/06/09 15:04:29 vedge Exp $	    */
/*	Public domain	*/

struct noderef;
struct input;

enum {
	DIR_UP =	0x01,
	DIR_DOWN =	0x02,
	DIR_LEFT =	0x04,
	DIR_RIGHT =	0x08,
	DIR_ALL =	0xff
};

enum {
	DIR_ANIM_IDLEUP,
	DIR_ANIM_IDLEDOWN,
	DIR_ANIM_IDLELEFT,
	DIR_ANIM_IDLERIGHT,
	DIR_ANIM_MOVEUP,
	DIR_ANIM_MOVEDOWN,
	DIR_ANIM_MOVELEFT,
	DIR_ANIM_MOVERIGHT
};

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

	struct	object *ob;	/* Object back pointer */
	struct	map *map;	/* Map back pointer */

	Uint32	set;		/* Set direction mask (move) */
	Uint32	current;	/* Current direction mask (moving) */
	Uint32	clear;		/* Clear direction mask (stop move) */
	Uint32	moved;		/* Post direction mask (moved) */

	Uint32	flags;
#define DIR_SCROLLVIEW	0x01	/* Scroll the view if we reach boundaries. */
#define DIR_SOFTSCROLL	0x02	/* Animate move from node to node. */
#define DIR_STATIC	0x04	/* Don't change sprites with directions. */
#define DIR_PASSTHROUGH	0x08	/* Pass through map nodes. */
};

struct mappos {
	struct	map *map;	/* Map (or NULL) */
	Uint32	x, y;		/* Map coordinates */
	Uint32	speed;		/* Speed in ms */
	struct	noderef *nref;	/* Node reference */
	struct	mapdir dir;	/* Map direction (not saved) */
	struct	input *input;	/* Controller (or NULL) */
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
int	mapdir_move(struct mapdir *, Uint32 *, Uint32 *);
void	mapdir_postmove(struct mapdir *, Uint32 *, Uint32 *, Uint32);

