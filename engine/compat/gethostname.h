/*	$Csoft: gethostname.h,v 1.1 2002/11/22 23:24:59 vedge Exp $	*/
/*	Public domain	*/

#include <sys/types.h>

#if defined(__GNUC__) && defined(_SGI_SOURCE)
/* IRIX gcc 2.95 hack. */
extern int gethostname(char *, size_t);
#else
# include <unistd.h>
#endif

