/*	Public domain	*/

#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <agar/config/have_gethostname.h>
#ifdef HAVE_GETHOSTNAME
# include <unistd.h>
#else
int gethostname(char *, size_t);
#endif
