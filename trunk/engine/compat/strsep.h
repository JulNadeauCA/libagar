/*	$Csoft: strsep.h,v 1.2 2002/09/06 00:50:43 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strsep.h>

#ifndef HAVE_STRSEP
char	*strsep(char **, const char *);
#endif

