/*	$Csoft: object.h,v 1.101 2004/01/22 09:58:42 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_POSITION_H_
#define _AGAR_POSITION_H_
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
struct map;
struct input;

/* Object direction on a map */
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

/* Unique object position on a map. */
struct position {
	struct map	*map;		/* Map (NULL if unresolved) */
	char		*map_name;	/* Map reference to resolve */
	int		 x, y, z;	/* Map coordinates */
	struct map	*projmap;	/* Current projection map */
	char		*projmap_name;	/* Submap reference to resolve */
	struct input	*input;		/* Controller (or NULL) */
	struct mapdir	 dir;		/* Direction */
	int		 vel;		/* Velocity factor */
};

__BEGIN_DECLS
void	 position_init(struct position *);
void	 position_destroy(struct position *);
int	 position_save(struct position *, struct netbuf *);
int	 position_load(struct object *, struct position *, struct netbuf *);

int	 position_set_input(void *, const char *);
int	 position_set_projmap(void *, const char *);
int	 position_set_direction(void *, int, int, int);

int	 position_set(void *, struct map *, int, int, int, struct map *);
void	 position_unset(void *);
void	 position_project(struct position *);
void	 position_unproject(struct position *);

void	mapdir_init(struct mapdir *, struct object *, int, int);
void	mapdir_destroy(struct mapdir *);
void	mapdir_set(struct mapdir *, int);
void	mapdir_unset(struct mapdir *, int);
int	mapdir_move(struct mapdir *);
void	mapdir_postmove(struct mapdir *, int *, int *, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_POSITION_H */
