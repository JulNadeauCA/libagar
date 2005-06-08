/*	$Csoft: sketchproj.h,v 1.2 2005/05/24 08:39:16 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_SKETCHPROJ_H_
#define _AGAR_RG_SKETCHPROJ_H_

#include <engine/rg/feature.h>

#include "begin_code.h"

struct rg_sketchproj {
	RG_Feature ft;

	char sketch[RG_TILE_ELEMENT_NAME_MAX];
	Uint8 alpha;				/* Overall alpha */
	Uint32 color;				/* Line color */
};

__BEGIN_DECLS
void		 RG_SketchProjInit(void *, RG_Tileset *, int);
int		 RG_SketchProjLoad(void *, AG_Netbuf *);
void		 RG_SketchProjSave(void *, AG_Netbuf *);
void		 RG_SketchProjApply(void *, RG_Tile *, int, int);
AG_Window	*RG_SketchProjEdit(void *, RG_Tileview *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_SKETCHPROJ_H_ */
