/*	$Csoft: gethostname.h,v 1.4 2003/03/25 11:39:02 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_gethostname.h>

#include <sys/types.h>

#ifdef HAVE_GETHOSTNAME
# include <unistd.h>
#else
int	gethostname(char *name, size_t namelen);
#endif

