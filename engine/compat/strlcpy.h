/*	$Csoft: strlcpy.h,v 1.2 2002/09/06 00:50:43 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcpy.h>

#ifndef HAVE_STRLCPY
#include <sys/types.h>
size_t	strlcpy(char *, const char *, size_t);
#endif

