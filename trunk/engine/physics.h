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
	int	set;		/* Set direction mask (move) */
	int	current;	/* Current direction mask (moving) */
	int	clear;		/* Clear direction mask (stop move) */
	int	moved;		/* Post direction mask (moved) */
};

/* 2D movement of map references between adjacent map nodes. */
struct mapdir {
	int	speed;		/* Soft-scroll increments */

	struct	object *ob;	/* Object back pointer */
	struct	map *map;	/* Map back pointer */

	int	set;		/* Set direction mask (move) */
	int	current;	/* Current direction mask (moving) */
	int	clear;		/* Clear direction mask (stop move) */
	int	moved;		/* Post direction mask (moved) */

	int	flags;
#define DIR_SCROLLVIEW	0x01	/* Scroll the view if we reach boundaries. */
#define DIR_SOFTSCROLL	0x02	/* Animate move from node to node. */
};

int	gendir_init(struct gendir *);
int	gendir_set(struct gendir *, int, int);
int	gendir_move(struct gendir *);
void	gendir_postmove(struct gendir *, int);

int	mapdir_init(struct mapdir *, struct object *, struct map *,
	    int, int);
void	mapdir_set(struct mapdir *, int, int);
int	mapdir_move(struct mapdir *, int *, int *);
void	mapdir_postmove(struct mapdir *, int *, int *, int);

