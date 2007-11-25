/*	Public domain	*/

#ifndef _AGAR_NODEMASK_H_
#define _AGAR_NODEMASK_H_
#include "begin_code.h"

struct map;

#if 0
	AG_NODEMASK_BITMAP,		/* Bitmap (pixel-perfect at 1:1) */
#endif
enum map_nodemask_type {
	AG_NODEMASK_POLYGON,		/* Simple polygon */
	AG_NODEMASK_RECTANGLE		/* Rectangular region */
};

typedef AG_TAILQ_HEAD(map_nodemaskq, map_nodemask) MAP_NodeMaskQ;

typedef struct map_nodemask {
	enum map_nodemask_type type;
	int scale;
	union {
		struct {
			AG_Object *obj;
			Uint32 offs;
		} bitmap;
		struct {
			Uint32 *vertices;
			Uint32 nvertices;
		} poly;
	} params;
#define nm_bitmap	params.bitmap
#define nm_poly		params.poly
	AG_TAILQ_ENTRY(map_nodemask) masks;
} MAP_NodeMask;

__BEGIN_DECLS
MAP_NodeMask	*MAP_NodeMaskNew(enum map_nodemask_type);

void	 MAP_NodeMaskInit(MAP_NodeMask *, enum map_nodemask_type);
int	 MAP_NodeMaskLoad(struct map *, AG_DataSource *, MAP_NodeMask *);
void	 MAP_NodeMaskSave(struct map *, AG_DataSource *, const MAP_NodeMask *);
void	 MAP_NodeMaskDestroy(struct map *, MAP_NodeMask *);
void	 MAP_NodeMaskCopy(const MAP_NodeMask *, struct map *,
	               MAP_NodeMask *);

#if 0
void	 MAP_NodeMaskBitmap(struct map *, MAP_NodeMask *, void *, Uint32);
#endif
void	 MAP_NodeMaskVertex(MAP_NodeMask *, Uint32, Uint32);
int	 MAP_NodeMaskIntersect(const MAP_NodeMask *, const MAP_NodeMask *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_NODEMASK_H_ */
