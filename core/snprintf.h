/*	Public domain	*/

#ifndef	_AGAR_CORE_SNPRINTF_H_
#define	_AGAR_CORE_SNPRINTF_H_

#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Snprintf AG_Snprintf
#endif /* _AGAR_INTERNAL */

#include <agar/config/have_snprintf.h>
#ifdef HAVE_SNPRINTF
# include <stdio.h>
# if defined(_WIN32) && defined(_MSC_VER)
#  define AG_Snprintf _snprintf
# else
#  define AG_Snprintf snprintf
# endif
#else
# include <agar/core/begin.h>
__BEGIN_DECLS
AG_Size AG_Snprintf(char *_Nonnull, AG_Size, const char *_Nonnull, ...);
__END_DECLS
# include <agar/core/close.h>
#endif /* !HAVE_SNPRINTF */
#endif /* _AGAR_CORE_SNPRINTF_H_ */
