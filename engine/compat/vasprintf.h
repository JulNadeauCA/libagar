/*	$Csoft$	*/
/*	Public domain	*/

int	compat_vasprintf(char **, const char *, va_list);

#ifndef vasprintf
#define vasprintf compat_vasprintf
#endif

