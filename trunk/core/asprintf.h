/*	Public domain	*/

#ifndef	_AGAR_CORE_ASPRINTF_H_
#define	_AGAR_CORE_ASPRINTF_H_

#include <stdarg.h>
#include <agar/core/begin.h>
__BEGIN_DECLS

void AG_Asprintf(char **, const char *, ...);
int  AG_TryAsprintf(char **, const char *, ...);

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Asprintf AG_Asprintf
# define TryAsprintf AG_TryAsprintf
#endif /* _AGAR_INTERNAL */

__END_DECLS
#include <agar/core/close.h>

#endif /* _AGAR_CORE_ASPRINTF_H_ */
