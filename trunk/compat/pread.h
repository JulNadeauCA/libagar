/*	$Csoft: pread.h,v 1.2 2003/04/12 01:45:33 vedge Exp $	*/
/*	Public domain	*/

#include <sys/types.h>

#ifdef HAVE_PREAD
#include <unistd.h>
#else
ssize_t	pread(int, void *, size_t, off_t);
#endif

