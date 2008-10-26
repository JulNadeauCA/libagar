/*	Public domain	*/

#include <agar/gui/begin.h>
__BEGIN_DECLS
int AG_XCFLoad(AG_DataSource *, off_t,
               void (*)(AG_Surface *, const char *, void *),
	       void *);
__END_DECLS
#include <agar/gui/close.h>
