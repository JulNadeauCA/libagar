/*	$Csoft: setenv.h,v 1.1 2002/08/23 09:00:37 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mcconfig.h>

#ifndef HAVE_SETENV
int	setenv(const char *, const char *, int);
void	unsetenv(const char *);
#endif

