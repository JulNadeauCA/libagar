/*	$Csoft: vasprintf.h,v 1.2 2002/08/20 09:36:06 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mcconfig.h>

#ifndef HAVE_VASPRINTF
int	vasprintf(char **, const char *, va_list);
#endif

