/*	Public domain	*/

#ifndef	_AGAR_CORE_VASPRINTF_H_
#define	_AGAR_CORE_VASPRINTF_H_
#include <stdarg.h>
#include <agar/core/begin.h>
__BEGIN_DECLS

int AG_TryVasprintf(char *_Nonnull *_Nullable, const char *_Nonnull, va_list);
		/* _Printf_Like_Attribute(2,0); */

static __inline__ void
AG_Vasprintf(char *_Nonnull *_Nullable s, const char *_Nonnull fmt, va_list args)
    /* _Printf_Like_Attribute(2,0); */
{
	if (AG_TryVasprintf(s, fmt, args) == -1) 
		AG_FatalError(NULL);
}

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Vasprintf AG_Vasprintf
# define TryVasprintf AG_TryVasprintf
#endif /* _AGAR_INTERNAL */

__END_DECLS
#include <agar/core/close.h>
#endif /* _AGAR_CORE_VASPRINTF_H_ */
