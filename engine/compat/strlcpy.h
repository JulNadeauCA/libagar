/*	$Csoft: strlcpy.h,v 1.1 2002/08/23 10:14:35 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mcconfig.h>

#ifndef HAVE_STRLCPY
#include <sys/types.h>
size_t	strlcpy(char *, const char *, size_t);
#endif

