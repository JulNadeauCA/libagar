/*	$Csoft: surface.h,v 1.1 2005/01/23 11:55:20 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
SDL_Surface	*AG_ReadSurface(AG_Netbuf *, SDL_PixelFormat *);
void		 AG_WriteSurface(AG_Netbuf *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
