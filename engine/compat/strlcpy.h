/*	$Csoft: strlcpy.h,v 1.4 2003/04/02 04:06:27 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcpy.h>

#ifndef HAVE_STRLCPY
#include <sys/types.h>
size_t	strlcpy(char *, const char *, size_t)
	    BOUNDED_ATTRIBUTE(__string__, 1, 3);
#else
#include <string.h>
#endif

