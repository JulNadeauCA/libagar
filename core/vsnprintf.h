/*	Public domain	*/

#ifndef	_AGAR_CORE_VSNPRINTF_H_
#define	_AGAR_CORE_VSNPRINTF_H_

#include <agar/config/_mk_have_sys_types_h.h>
#include <agar/config/have_vsnprintf.h>

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdarg.h>

#ifdef HAVE_VSNPRINTF
# include <stdio.h>
#ifdef _XBOX
# define AG_Vsnprintf _vsnprintf
#else
# define AG_Vsnprintf vsnprintf
#endif // _XBOX
#else
# include <agar/core/begin.h>
__BEGIN_DECLS
int AG_Vsnprintf(char *, size_t, const char *, va_list);
__END_DECLS
# include <agar/core/close.h>
#endif /* HAVE_VASPRINTF */

#endif /* _AGAR_CORE_VSNPRINTF_H_ */
