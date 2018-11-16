/*	Public domain	*/

#ifndef	_AGAR_CORE_VSNPRINTF_H_
#define	_AGAR_CORE_VSNPRINTF_H_
#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdarg.h>

#include <agar/core/begin.h>
__BEGIN_DECLS

int AG_TryVsnprintf(char *_Nonnull, AG_Size, const char *_Nonnull, va_list);

static __inline__ void
AG_Vsnprintf(char *_Nonnull s, AG_Size len, const char *_Nonnull fmt, va_list args)
    /* _Printf_Like_Attribute(3,0) */
{
	if (AG_TryVsnprintf(s, len, fmt, args) == -1)
		AG_FatalError(NULL);
}

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Vsnprintf AG_Vsnprintf
# define TryVsnprintf AG_TryVsnprintf
#endif /* _AGAR_INTERNAL */

__END_DECLS
#include <agar/core/close.h>
#endif /* _AGAR_CORE_VSNPRINTF_H_ */
