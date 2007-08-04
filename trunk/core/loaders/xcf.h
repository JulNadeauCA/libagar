/*	Public domain	*/

#include "begin_code.h"
__BEGIN_DECLS
int AG_XCFLoad(AG_Netbuf *, off_t,
               void (*)(SDL_Surface *, const char *, void *),
	       void *);
__END_DECLS
#include "close_code.h"
