/*	Public domain	*/

#ifndef _AGAR_RG_TEXSEL_H_
#define _AGAR_RG_TEXSEL_H_

#ifdef _AGAR_INTERNAL
#include <rg/tileset.h>
#include <gui/tlist.h>
#else
#include <agar/rg/tileset.h>
#include <agar/gui/tlist.h>
#endif

#include "begin_code.h"

typedef struct rg_texture_selector {
	struct ag_tlist tl;
	RG_Tileset *tset;		   /* Attached tileset */
	char texname[RG_TEXTURE_NAME_MAX]; /* Default texture name binding */
	Uint flags;
} RG_TextureSelector;

__BEGIN_DECLS
extern const AG_WidgetOps rgTextureSelectorOps;
RG_TextureSelector *RG_TextureSelectorNew(void *, RG_Tileset *, Uint);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_TEXSEL_H_ */
