/*	$Csoft: physics.h,v 1.20 2003/04/12 01:30:03 vedge Exp $	    */
/*	Public domain	*/

#ifndef _AGAR_PHYSICS_H_
#define _AGAR_PHYSICS_H_
#include "begin_code.h"

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

__BEGIN_DECLS
extern DECLSPEC void	gendir_init(struct gendir *);
extern DECLSPEC void	gendir_set(struct gendir *, int);
extern DECLSPEC void	gendir_unset(struct gendir *, int);
extern DECLSPEC int	gendir_move(struct gendir *);
extern DECLSPEC void	gendir_postmove(struct gendir *, int);

extern DECLSPEC void	mapdir_init(struct mapdir *, struct object *,
			            struct map *, int, int);
extern DECLSPEC void	mapdir_set(struct mapdir *, int);
extern DECLSPEC void	mapdir_unset(struct mapdir *, int);
extern DECLSPEC int	mapdir_move(struct mapdir *);
extern DECLSPEC void	mapdir_postmove(struct mapdir *, int *, int *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_PHYSICS_H_ */
