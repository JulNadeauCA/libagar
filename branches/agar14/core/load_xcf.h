/*	Public domain	*/

#include "begin_code.h"
__BEGIN_DECLS
int AG_XCFLoad(AG_DataSource *, off_t,
               void (*)(SDL_Surface *, const char *, void *),
	       void *);
__END_DECLS
#include "close_code.h"