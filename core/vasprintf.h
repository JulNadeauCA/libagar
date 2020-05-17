/*	Public domain	*/

#ifndef	_AGAR_CORE_VASPRINTF_H_
#define	_AGAR_CORE_VASPRINTF_H_

#include <stdarg.h>

#include <agar/core/begin.h>
__BEGIN_DECLS
int  AG_TryVasprintf(char *_Nonnull *_Nullable, const char *_Nonnull, va_list);
void AG_Vasprintf(char *_Nonnull *_Nullable, const char *_Nonnull, va_list);
#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Vasprintf    AG_Vasprintf
# define TryVasprintf AG_TryVasprintf
#endif
__END_DECLS
#include <agar/core/close.h>
#endif /* _AGAR_CORE_VASPRINTF_H_ */
