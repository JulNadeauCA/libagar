/*	$Csoft: gethostname.h,v 1.3 2003/03/02 07:29:13 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_gethostname.h>

#include <sys/types.h>

#ifdef HAVE_GETHOSTNAME
# include <unistd.h>
#else
int	gethostname(char *name, size_t namelen);
#endif

