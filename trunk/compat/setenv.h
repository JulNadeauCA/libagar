/*	$Csoft: setenv.h,v 1.4 2003/03/14 04:16:12 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_setenv.h>

#ifndef HAVE_SETENV
int	setenv(const char *, const char *, int);
void	unsetenv(const char *);
#endif

