/*	$Csoft: setenv.h,v 1.3 2002/12/24 10:29:21 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_setenv.h>

#ifndef HAVE_SETENV
int	setenv(const char *, const char *, int);
void	unsetenv(const char *);
#endif

