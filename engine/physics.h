/*	$Csoft	    */

enum {
	DIR_UP =	0x01,
	DIR_DOWN =	0x02,
	DIR_LEFT =	0x04,
	DIR_RIGHT =	0x08,
	DIR_ALL =	0xff
};

/* 2D movement of undefined constructs in an infinite area. */
struct gendir {
	Uint32	set;		/* Set direction mask (move) */
	Uint32	current;	/* Current direction mask (moving) */
	Uint32	clear;		/* Clear direction mask (stop move) */
	Uint32	moved;		/* Post direction mask (moved) */
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
};

void	gendir_init(struct gendir *);
Uint32	gendir_set(struct gendir *, Uint32, Uint32);
Uint32	gendir_move(struct gendir *);
void	gendir_postmove(struct gendir *, Uint32);

void	mapdir_init(struct mapdir *, struct object *, struct map *, Uint32,
	    Uint32);
void	mapdir_set(struct mapdir *, Uint32, Uint32);
int	mapdir_move(struct mapdir *, Uint32 *, Uint32 *);
void	mapdir_postmove(struct mapdir *, Uint32 *, Uint32 *, Uint32);

