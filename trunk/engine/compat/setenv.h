/*	$Csoft: setenv.h,v 1.2 2002/09/06 00:50:43 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_getenv.h>

#ifndef HAVE_SETENV
int	setenv(const char *, const char *, int);
void	unsetenv(const char *);
#endif

