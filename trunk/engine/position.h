/*	$Csoft: position.h,v 1.2 2004/02/26 10:14:36 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_POSITION_H_
#define _AGAR_POSITION_H_
#include "begin_code.h"

struct object;
struct map;
struct input;

/* Unique object position on a map. */
struct position {
	struct map	*map;		/* Map (NULL if unresolved) */
	char		 map_name[OBJECT_PATH_MAX];
	struct map	*projmap;	/* Projection map (NULL if invisible) */
	char		 projmap_name[OBJECT_PATH_MAX];
	struct input	*input;		/* Controller (NULL if uncontrolled) */
	int		 x, y, z;	/* Map coordinates */
	Uint8		 dir, vel;	/* Velocity vector */
	Uint8		 flags;
#define POSITION_CENTER_VIEW  0x01	/* Center view around object/scroll. */
#define POSITION_SOFT_MOTION  0x02	/* Animate projection displacements. */
#define POSITION_PASS_THROUGH 0x04	/* Ignore node friction attribute. */
};

__BEGIN_DECLS
void	 position_init(struct position *);
int	 position_save(struct position *, struct netbuf *);
int	 position_load(struct object *, struct netbuf *);
int	 position_resolve(struct object *);

void	 position_set_input(void *, struct input *);
void	 position_set_projmap(void *, struct map *);
void	 position_set_velvec(void *, int, int);

int	 position_set(void *, struct map *, int, int, int);
void	 position_unset(void *);
void	 position_project(struct position *);
void	 position_unproject(struct position *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_POSITION_H */
