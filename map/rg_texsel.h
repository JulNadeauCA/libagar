/*	Public domain	*/

#ifndef _AGAR_RG_TEXSEL_H_
#define _AGAR_RG_TEXSEL_H_

#include <agar/map/rg_tileset.h>
#include <agar/gui/tlist.h>

#include <agar/map/begin.h>

typedef struct rg_texture_selector {
	struct ag_tlist tl;                /* AG_Tlist -> RG_TextureSelector */
	RG_Tileset *tset;		   /* Attached tileset */
	char texname[RG_TEXTURE_NAME_MAX]; /* Default texture name binding */
	Uint flags;
	Uint32 _pad;
} RG_TextureSelector;

__BEGIN_DECLS
extern AG_WidgetClass rgTextureSelectorClass;
RG_TextureSelector *RG_TextureSelectorNew(void *, RG_Tileset *, Uint);
__END_DECLS

#include <agar/map/close.h>
#endif /* _AGAR_RG_TEXSEL_H_ */
