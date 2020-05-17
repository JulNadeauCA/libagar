/*	Public domain	*/

#ifndef _AGAR_GUI_ICONMGR_H_
#define _AGAR_GUI_ICONMGR_H_
#include <agar/gui/begin.h>

/* Description of icon stored in data segment. */
typedef struct ag_static_icon {
	Uint w, h;			/* Dimensions in pixels */
	Uint32 Rmask;			/* Red mask */
	Uint32 Gmask;			/* Green mask */
	Uint32 Bmask;			/* Blue mask */
	Uint32 Amask;			/* Alpha mask */
	const Uint32 *_Nonnull data;	/* Pixel data */
	AG_Surface *_Nullable s;	/* Initialized surface */
} AG_StaticIcon;

__BEGIN_DECLS
void AG_InitStaticIcon(AG_StaticIcon *_Nonnull);
void AG_FreeStaticIcon(AG_StaticIcon *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_ICONMGR_H_ */
