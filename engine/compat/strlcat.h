/*	$Csoft: strlcat.h,v 1.4 2003/04/02 04:06:27 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcat.h>

#ifndef HAVE_STRLCAT
#include <sys/types.h>
size_t	strlcat(char *, const char *, size_t)
	    BOUNDED_ATTRIBUTE(__string__, 1, 3);
#else
#include <string.h>
#endif
