/*	$Csoft: string.h,v 1.2 2003/09/12 03:09:04 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
Uint32	read_color(struct netbuf *, SDL_PixelFormat *);
void	write_color(struct netbuf *, SDL_PixelFormat *, Uint32);
__END_DECLS

#include "close_code.h"
