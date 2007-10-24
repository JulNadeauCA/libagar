/*	Public domain	*/

#ifndef _AGAR_ICONMGR_H_
#define _AGAR_ICONMGR_H_
#include "begin_code.h"

typedef struct ag_static_icon {
	Uint w, h;
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;
	const Uint32 *data;
	SDL_Surface *s;
} AG_StaticIcon;

__BEGIN_DECLS
void		 AG_InitStaticIcon(AG_StaticIcon *);
SDL_Surface	*AG_ObjectIcon(void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_ICONMGR_H_ */
