/*	Public domain	*/

#ifndef	_AGAR_CORE_VASPRINTF_H_
#define	_AGAR_CORE_VASPRINTF_H_

#include <stdarg.h>

#include <agar/begin.h>
__BEGIN_DECLS
int AG_Vasprintf(char **, const char *, va_list);
__END_DECLS
#include <agar/close.h>

#endif /* _AGAR_CORE_VASPRINTF_H_ */
