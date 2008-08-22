/*	Public domain	*/

#include <agar/config/_mk_have_sys_types_h.h>
#include <agar/config/have_vsnprintf.h>

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdarg.h>

#ifdef HAVE_VSNPRINTF
# include <stdio.h>
# define AG_Vsnprintf vsnprintf
#else
# ifdef __cplusplus
extern "C" {
# endif
int AG_Vsnprintf(char *, size_t, const char *, va_list);
# ifdef __cplusplus
}
# endif
#endif
