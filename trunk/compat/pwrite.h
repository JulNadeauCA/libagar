/*	$Csoft: pwrite.h,v 1.2 2003/05/29 01:52:32 vedge Exp $	*/
/*	Public domain	*/

#include <sys/types.h>

#ifdef HAVE_PWRITE
#include <unistd.h>
#else
ssize_t	pwrite(int, const void *, size_t, off_t);
#endif

