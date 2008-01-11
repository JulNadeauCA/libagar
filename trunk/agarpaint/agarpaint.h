#ifndef _AGARPAINT_H_
#define _AGARPAINT_H_

#include <agar/core/strlcpy.h>
#include <agar/core/strlcat.h>

#include <agar/config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
typedef unsigned int Uchar;
typedef unsigned long Ulong;
#endif

#if 0
# include <libintl/libintl.h>
# define _(String) gettext(String)
# define gettext_noop(String) (String)
# define N_(String) gettext_noop(String)
#else
# undef _
# undef N_
# undef textdomain
# undef bindtextdomain
# define _(s) (s)
# define N_(s) (s)
# define textdomain(d)
# define bindtextdomain(p, d)
#endif

#define Malloc AG_Malloc
#define Free AG_Free
#define Strlcpy AG_Strlcpy
#define Strlcat AG_Strlcat

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#endif /* _AGARPAINT_H_ */
