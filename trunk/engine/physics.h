/*	$Csoft: physics.h,v 1.19 2003/03/10 02:13:40 vedge Exp $	    */
/*	Public domain	*/

#ifndef _AGAR_PHYSICS_H_
#define _AGAR_PHYSICS_H_

#define DIR_UP		0x01
#define DIR_DOWN	0x02
#define DIR_LEFT	0x04
#define DIR_RIGHT	0x08
#define DIR_ALL		0xff

struct object;
struct map;

/* 2D movement of undefined constructs in an infinite area. */
struct gendir {
	int	set;		/* Set direction mask (move) */
	int	current;	/* Current direction mask (moving) */
	int	clear;		/* Clear direction mask (stop move) */
	int	moved;		/* Post direction mask (moved) */
	int	offs;		/* Timing for repeat */
	int	noffs;		/* Timing for delay */
};

/* 2D movement of map references between adjacent map nodes. */
struct mapdir {
	int	speed;		/* Soft-scroll increments */

	struct object	*ob;	/* Object back pointer */
	struct map	*map;	/* Map back pointer */

	int	set;		/* Set direction mask (move) */
	int	current;	/* Current direction mask (moving) */
	int	clear;		/* Clear direction mask (stop move) */
	int	moved;		/* Post direction mask (moved) */

	int	flags;
#define DIR_SCROLLVIEW	0x01	/* Perform centering/soft-scrolling. */
#define DIR_SOFTSCROLL	0x02	/* Animate all node->node displacements. */
#define DIR_PASSTHROUGH	0x08	/* Pass through all nodes. */
};

void	gendir_init(struct gendir *);
void	gendir_set(struct gendir *, int);
void	gendir_unset(struct gendir *, int);
int	gendir_move(struct gendir *);
void	gendir_postmove(struct gendir *, int);

void	mapdir_init(struct mapdir *, struct object *, struct map *, int, int);
void	mapdir_set(struct mapdir *, int);
void	mapdir_unset(struct mapdir *, int);
int	mapdir_move(struct mapdir *);
void	mapdir_postmove(struct mapdir *, int *, int *, int);

#endif /* _AGAR_PHYSICS_H_ */
