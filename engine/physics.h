/*	$Csoft: physics.h,v 1.22 2003/06/18 00:46:58 vedge Exp $	    */
/*	Public domain	*/

#ifndef _AGAR_PHYSICS_H_
#define _AGAR_PHYSICS_H_
#include "begin_code.h"

#define DIR_N	0x01
#define DIR_S	0x02
#define DIR_W	0x04
#define DIR_E	0x08
#define DIR_NW	(DIR_N|DIR_W)
#define DIR_NE	(DIR_N|DIR_E)
#define DIR_SW	(DIR_S|DIR_W)
#define DIR_SE	(DIR_S|DIR_E)
#define DIR_ALL	0xff

struct object;

struct mapdir {
	struct object	*ob;	/* Object back pointer */

	int	speed;		/* Increment for object shift and scrolling */
	int	set;		/* Set direction mask (move) */
	int	current;	/* Current direction mask (moving) */
	int	clear;		/* Clear direction mask (stop move) */
	int	moved;		/* Post direction mask (moved) */

	int	flags;
#define DIR_CENTER_VIEW  0x01	/* Center view around object and scroll. */
#define DIR_SOFT_MOTION  0x02	/* Animate node->node displacements. */
#define DIR_PASS_THROUGH 0x04	/* Ignore node surface settings. */
};

__BEGIN_DECLS
void	mapdir_init(struct mapdir *, struct object *, int, int);
void	mapdir_set(struct mapdir *, int);
void	mapdir_unset(struct mapdir *, int);
int	mapdir_move(struct mapdir *);
void	mapdir_postmove(struct mapdir *, int *, int *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_PHYSICS_H_ */
