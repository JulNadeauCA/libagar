/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_snprintf.h>
#include <config/_mk_have_sys_types_h.h>
#else
#include <agar/config/have_snprintf.h>
#include <agar/config/_mk_have_sys_types_h.h>
#endif

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

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
__END_DECLS
#endif /* !HAVE_SNPRINTF */
