/*	$Csoft: transform.h,v 1.2 2005/04/16 05:58:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_TRANSFORM_H_
#define _AGAR_MAP_TRANSFORM_H_
#include "begin_code.h"

#define MAP_TRANSFORM_MAX_ARGS	64

enum map_transform_type {
	MAP_TRANSFORM_MIRROR,
	MAP_TRANSFORM_FLIP,
	MAP_TRANSFORM_ROTATE,
	MAP_TRANSFORM_RGB_INVERT
};

TAILQ_HEAD(map_transformq, map_transform);

typedef struct map_transform {
	enum map_transform_type type;
	SDL_Surface *(*func)(SDL_Surface *, int, Uint32 *);
	Uint32 *args;
	int nargs;
	TAILQ_ENTRY(map_transform) transforms;
} MAP_Transform;

struct map_transform_ent {
	char *name;
	enum map_transform_type type;
	SDL_Surface *(*func)(SDL_Surface *, int, Uint32 *);
};

struct map_item;

__BEGIN_DECLS
MAP_Transform *MAP_TransformNew(enum map_transform_type, int, Uint32 *);
int	       MAP_TransformInit(MAP_Transform *, enum map_transform_type, int,
		                  Uint32 *);
int	       MAP_TransformLoad(AG_Netbuf *, MAP_Transform *);
void	       MAP_TransformSave(AG_Netbuf *, const MAP_Transform *);
void	       MAP_TransformDestroy(MAP_Transform *);
__inline__ int MAP_TransformCompare(const MAP_Transform *,
		                    const MAP_Transform *);
void	       MAP_TransformPrint(const struct map_transformq *, char *, size_t)
		                  BOUNDED_ATTRIBUTE(__string__, 2, 3);
MAP_Transform *MAP_TransformRotate(struct map_item *, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MAP_TRANSFORM_H_ */
