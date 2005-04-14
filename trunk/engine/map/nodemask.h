/*	$Csoft: nodemask.h,v 1.4 2005/02/06 07:06:21 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_NODEMASK_H_
#define _AGAR_NODEMASK_H_
#include "begin_code.h"

enum nodemask_type {
	NODEMASK_BITMAP,		/* Bitmap (pixel-perfect at 1:1) */
	NODEMASK_POLYGON,		/* Simple polygon */
	NODEMASK_RECTANGLE		/* Rectangular region */
};

TAILQ_HEAD(nodemaskq, nodemask);

struct nodemask {
	enum nodemask_type type;
	int scale;
	union {
		struct {
			struct object *obj;
			Uint32 offs;
		} bitmap;
		struct {
			Uint32 *vertices;
			Uint32 nvertices;
		} poly;
	} params;
	TAILQ_ENTRY(nodemask) masks;
#define nm_bitmap	params.bitmap
#define nm_poly		params.poly
};

__BEGIN_DECLS
struct nodemask	*nodemask_new(enum nodemask_type);

void	 nodemask_init(struct nodemask *, enum nodemask_type);
int	 nodemask_load(struct map *, struct netbuf *, struct nodemask *);
void	 nodemask_save(struct map *, struct netbuf *, const struct nodemask *);
void	 nodemask_destroy(struct map *, struct nodemask *);
void	 nodemask_copy(const struct nodemask *, struct map *,
	               struct nodemask *);

void	 nodemask_bitmap(struct map *, struct nodemask *, void *, Uint32);
void	 nodemask_vertex(struct nodemask *, Uint32, Uint32);
int	 nodemask_intersect(const struct nodemask *, const struct nodemask *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_NODEMASK_H_ */
