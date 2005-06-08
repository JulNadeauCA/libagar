/*	$Csoft: texsel.h,v 1.1 2005/05/28 08:39:35 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_TEXSEL_H_
#define _AGAR_RG_TEXSEL_H_

#include <engine/widget/widget.h>
#include <engine/widget/tlist.h>

#include <engine/rg/tileset.h>

#include "begin_code.h"

typedef struct rg_texture_selector {
	AG_Tlist tl;		/* Superclass */
	RG_Tileset *tset;		/* Attached tileset */
	char texname[RG_TEXTURE_NAME_MAX];	/* Default texture name binding */
	int flags;
} RG_TextureSelector;

__BEGIN_DECLS
RG_TextureSelector *RG_TextureSelectorNew(void *, RG_Tileset *, int);
void 		    RG_TextureSelectorInit(RG_TextureSelector *,
		                           RG_Tileset *, int);
void 		    RG_TextureSelectorScale(void *, int, int);
void		    RG_TextureSelectorDestroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_TEXSEL_H_ */
