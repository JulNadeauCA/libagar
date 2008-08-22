/*	Public domain	*/

#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <agar/config/have_snprintf.h>
#ifdef HAVE_SNPRINTF
# include <stdio.h>
# if defined(_WIN32)
#  define AG_Snprintf _snprintf
# else
#  define AG_Snprintf snprintf
# endif
#else
# ifdef __cplusplus
extern "C" {
# endif
int AG_Snprintf(char *, size_t, const char *, ...);
# ifdef __cplusplus
}
# endif
#endif /* !HAVE_SNPRINTF */
