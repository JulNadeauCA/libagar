/*	$Csoft$	*/
/*	Public domain	*/

#include <config/have_snprintf.h>

#include <sys/types.h>

#ifdef HAVE_SNPRINTF
#include <stdio.h>
#else
int	snprintf(char *, size_t, const char *, ...);
#endif

