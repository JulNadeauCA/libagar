/*	$Csoft: strsep.h,v 1.3 2002/12/24 10:29:21 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strsep.h>

#ifndef HAVE_STRSEP
char	*strsep(char **, const char *);
#endif

