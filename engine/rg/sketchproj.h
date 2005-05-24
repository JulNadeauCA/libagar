/*	$Csoft: sketchproj.h,v 1.1 2005/05/18 09:07:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_SKETCHPROJ_H_
#define _AGAR_RG_SKETCHPROJ_H_

#include <engine/rg/feature.h>

#include "begin_code.h"

struct sketchproj {
	struct feature ft;
	char sketch[TILE_ELEMENT_NAME_MAX];
	Uint8 alpha;				/* Overall alpha */
	Uint32 color;				/* Line color */
};

__BEGIN_DECLS
void		 sketchproj_init(void *, struct tileset *, int);
int		 sketchproj_load(void *, struct netbuf *);
void		 sketchproj_save(void *, struct netbuf *);
void		 sketchproj_apply(void *, struct tile *, int, int);
struct window	*sketchproj_edit(void *, struct tileview *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_SKETCHPROJ_H_ */
