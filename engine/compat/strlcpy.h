/*	$Csoft: strlcpy.h,v 1.3 2002/12/24 10:29:21 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcpy.h>

#ifndef HAVE_STRLCPY
#include <sys/types.h>
size_t	strlcpy(char *, const char *, size_t);
#else
#include <string.h>
#endif

