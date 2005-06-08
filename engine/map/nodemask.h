/*	$Csoft: nodemask.h,v 1.1 2005/04/14 06:19:41 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_NODEMASK_H_
#define _AGAR_NODEMASK_H_
#include "begin_code.h"

struct ag_map;

enum ag_nodemask_type {
	AG_NODEMASK_BITMAP,		/* Bitmap (pixel-perfect at 1:1) */
	AG_NODEMASK_POLYGON,		/* Simple polygon */
	AG_NODEMASK_RECTANGLE		/* Rectangular region */
};

TAILQ_HEAD(ag_nodemaskq, ag_nodemask);

typedef struct ag_nodemask {
	enum ag_nodemask_type type;
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
	TAILQ_ENTRY(ag_nodemask) masks;
} AG_NodeMask;

__BEGIN_DECLS
AG_NodeMask	*AG_NodeMaskNew(enum ag_nodemask_type);

void	 AG_NodeMaskInit(AG_NodeMask *, enum ag_nodemask_type);
int	 AG_NodeMaskLoad(struct ag_map *, AG_Netbuf *, AG_NodeMask *);
void	 AG_NodeMaskSave(struct ag_map *, AG_Netbuf *, const AG_NodeMask *);
void	 AG_NodeMaskDestroy(struct ag_map *, AG_NodeMask *);
void	 AG_NodeMaskCopy(const AG_NodeMask *, struct ag_map *,
	               AG_NodeMask *);

void	 AG_NodeMaskBitmap(struct ag_map *, AG_NodeMask *, void *, Uint32);
void	 AG_NodeMaskVertex(AG_NodeMask *, Uint32, Uint32);
int	 AG_NodeMaskIntersect(const AG_NodeMask *, const AG_NodeMask *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_NODEMASK_H_ */
