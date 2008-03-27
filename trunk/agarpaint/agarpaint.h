#ifndef _AGARPAINT_H_
#define _AGARPAINT_H_

#include <agar/config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
typedef unsigned int Uchar;
typedef unsigned long Ulong;
#endif

#include "config/enable_nls.h"
#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) gettext(String)
# define gettext_noop(String) (String)
# define N_(String) gettext_noop(String)
#else
# undef _
# undef N_
# define _(s) (s)
# define N_(s) (s)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#endif /* _AGARPAINT_H_ */
