/*	$Csoft: transform.h,v 1.2 2005/04/16 05:58:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_TRANSFORM_H_
#define _AGAR_TRANSFORM_H_
#include "begin_code.h"

#define AG_TRANSFORM_MAX_ARGS	64

enum ag_transform_type {
	AG_TRANSFORM_MIRROR,
	AG_TRANSFORM_FLIP,
	AG_TRANSFORM_ROTATE,
	AG_TRANSFORM_RGB_INVERT
};

TAILQ_HEAD(ag_transformq, ag_transform);

typedef struct ag_transform {
	enum ag_transform_type	  type;
	SDL_Surface		*(*func)(SDL_Surface *, int, Uint32 *);
	Uint32			 *args;
	int			  nargs;
	TAILQ_ENTRY(ag_transform) transforms;
} AG_Transform;

struct ag_transform_ent {
	char			 *name;
	enum ag_transform_type	  type;
	SDL_Surface		*(*func)(SDL_Surface *, int, Uint32 *);
};

struct ag_nitem;

__BEGIN_DECLS
AG_Transform   *AG_TransformNew(enum ag_transform_type, int, Uint32 *);
int		AG_TransformInit(AG_Transform *, enum ag_transform_type, int,
		                   Uint32 *);
int		AG_TransformLoad(AG_Netbuf *, AG_Transform *);
void		AG_TransformSave(AG_Netbuf *, const AG_Transform *);
void		AG_TransformDestroy(AG_Transform *);
__inline__ int	AG_TransformCompare(const AG_Transform *,
		                      const AG_Transform *);
void		AG_TransformPrint(const struct ag_transformq *, char *, size_t)
		                  BOUNDED_ATTRIBUTE(__string__, 2, 3);
AG_Transform   *AG_TransformRotate(struct ag_nitem *, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_TRANSFORM_H_ */
