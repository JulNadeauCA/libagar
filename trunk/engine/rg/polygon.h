/*	$Csoft: polygon.h,v 1.3 2005/05/24 03:00:29 vedge Exp $	*/
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
	Uint8 alpha;				/* Overall alpha */
	union {
		struct {
			Uint32 c;		/* Fill color */
		} solid;
		char texture[TEXTURE_NAME_MAX];
	} args;
#define p_solid args.solid
#define p_texture args.texture
};

__BEGIN_DECLS
void		 polygon_init(void *, struct tileset *, int);
int		 polygon_load(void *, struct netbuf *);
void		 polygon_save(void *, struct netbuf *);
void		 polygon_render(void *, struct tile *, int, int);
void		 polygon_render_simple(void *, struct tile *, int, int);
struct window	*polygon_edit(void *, struct tileview *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_POLYGON_H_ */
