/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_gethostname.h>
#include <config/_mk_have_sys_types_h.h>
#else
#include <agar/config/have_gethostname.h>
#include <agar/config/_mk_have_sys_types_h.h>
#endif

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_GETHOSTNAME
# include <unistd.h>
#else
int gethostname(char *, size_t);
#endif
