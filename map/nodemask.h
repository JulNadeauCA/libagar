/*	Public domain	*/

#ifndef _AGAR_MAP_NODEMASK_H_
#define _AGAR_MAP_NODEMASK_H_
#include <agar/map/begin.h>

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
			AG_Object *_Nullable obj;
			Uint32 offs;
			Uint32 _pad;
		} bitmap;
		struct {
			Uint32 *_Nullable vertices;
			Uint32           nvertices;
			Uint32 _pad;
		} poly;
	} params;
#define nm_bitmap params.bitmap
#define nm_poly   params.poly
	AG_TAILQ_ENTRY(map_nodemask) masks;
} MAP_NodeMask;

__BEGIN_DECLS
MAP_NodeMask *_Nonnull MAP_NodeMaskNew(enum map_nodemask_type);

void MAP_NodeMaskInit(MAP_NodeMask *_Nonnull, enum map_nodemask_type);
int  MAP_NodeMaskLoad(struct map *_Nonnull, AG_DataSource *_Nonnull,
                      MAP_NodeMask *_Nonnull);
void MAP_NodeMaskSave(struct map *_Nonnull, AG_DataSource *_Nonnull,
                      const MAP_NodeMask *_Nonnull);
void MAP_NodeMaskDestroy(struct map *_Nonnull, MAP_NodeMask *_Nonnull);
void MAP_NodeMaskCopy(const MAP_NodeMask *_Nonnull,
                      struct map *_Nonnull, MAP_NodeMask *_Nonnull);

#if 0
void MAP_NodeMaskBitmap(struct map *, MAP_NodeMask *, void *, Uint32);
#endif
void MAP_NodeMaskVertex(MAP_NodeMask *_Nonnull, Uint32, Uint32);
int  MAP_NodeMaskIntersect(const MAP_NodeMask *_Nonnull, const MAP_NodeMask *_Nonnull);
__END_DECLS

#include <agar/map/close.h>
#endif /* _AGAR_MAP_NODEMASK_H_ */
