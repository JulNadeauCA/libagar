/*	$Csoft: strlcat.h,v 1.3 2002/12/24 10:29:21 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcat.h>

#ifndef HAVE_STRLCAT
#include <sys/types.h>
size_t	strlcat(char *, const char *, size_t);
#else
#include <string.h>
#endif

