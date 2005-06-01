/*	$Csoft: polygon.h,v 1.6 2005/05/31 03:59:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_POLYGON_H_
#define _AGAR_RG_POLYGON_H_

#include <engine/rg/feature.h>

#include "begin_code.h"

struct polygon {
	struct feature ft;
	char sketch[TILE_ELEMENT_NAME_MAX];
	enum polygon_type {
		POLYGON_SOLID,			/* Solid fill */
		POLYGON_TEXTURED		/* Texture fill */
	} type;
	Uint32 cSolid;				/* Solid Fill color */
	char texture[TEXTURE_NAME_MAX];		/* Texture name */
	int texture_alpha;			/* Texture blending alpha */
	
	int *ints;				/* Used for scan conversion */
	u_int nints;
};

__BEGIN_DECLS
void		 polygon_init(void *, struct tileset *, int);
void		 polygon_destroy(void *);
int		 polygon_load(void *, struct netbuf *);
void		 polygon_save(void *, struct netbuf *);
void		 polygon_render(void *, struct tile *, int, int);
#if 0
void		 polygon_render_simple(void *, struct tile *, int, int);
#endif
struct window	*polygon_edit(void *, struct tileview *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_POLYGON_H_ */
