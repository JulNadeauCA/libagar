/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_setenv.h>
#else
#include <agar/config/have_setenv.h>
#endif

#ifndef HAVE_SETENV
int	setenv(const char *, const char *, int);
void	unsetenv(const char *);
#endif
