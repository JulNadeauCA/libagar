/*	$Csoft: pread.h,v 1.1 2003/03/13 22:43:54 vedge Exp $	*/
/*	Public domain	*/

#include <sys/types.h>

#ifdef HAVE_PREAD
#include <unistd.h>
#else
ssize_t	pread(int, void *, size_t, off_t);
#endif

