/*	$Csoft$	*/
/*	Public domain	*/

#include <config/have_vsnprintf.h>

#include <sys/types.h>

#ifdef HAVE_VSNPRINTF
#include <stdio.h>
#else
int	vsnprintf(char *, size_t, const char *, va_list);
#endif

