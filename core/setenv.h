/*	Public domain	*/

#include <agar/config/have_setenv.h>

#ifndef HAVE_SETENV
int	setenv(const char *, const char *, int);
void	unsetenv(const char *);
#endif
