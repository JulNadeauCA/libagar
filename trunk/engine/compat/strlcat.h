/*	$Csoft: strlcat.h,v 1.2 2002/09/06 00:50:43 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcat.h>

#ifndef HAVE_STRLCAT
#include <sys/types.h>
size_t	strlcat(char *, const char *, size_t);
#endif

