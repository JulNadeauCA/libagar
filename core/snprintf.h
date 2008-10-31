/*	Public domain	*/

#ifndef	_AGAR_CORE_SNPRINTF_H_
#define	_AGAR_CORE_SNPRINTF_H_

#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

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
int AG_Snprintf(char *, size_t, const char *, ...);
__END_DECLS
# include <agar/core/close.h>
#endif /* !HAVE_SNPRINTF */

#endif /* _AGAR_CORE_SNPRINTF_H_ */
