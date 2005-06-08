/*	$Csoft: color.h,v 1.1 2004/04/30 07:00:23 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
Uint32	AG_ReadColor(AG_Netbuf *, SDL_PixelFormat *);
void	AG_WriteColor(AG_Netbuf *, SDL_PixelFormat *, Uint32);
__END_DECLS

#include "close_code.h"
