/*	$Csoft: color.h,v 1.1 2004/04/30 07:00:23 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
SDL_Surface	*read_surface(struct netbuf *, SDL_PixelFormat *);
void		 write_surface(struct netbuf *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
