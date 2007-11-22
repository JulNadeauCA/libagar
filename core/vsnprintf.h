/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_vsnprintf.h>
#include <config/_mk_have_sys_types_h.h>
#else
#include <agar/config/have_vsnprintf.h>
#include <agar/config/_mk_have_sys_types_h.h>
#endif

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdarg.h>

#ifdef HAVE_VSNPRINTF
# include <stdio.h>
# define AG_Vsnprintf vsnprintf
#else
int AG_Vsnprintf(char *, size_t, const char *, va_list);
#endif
