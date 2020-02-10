/*	Public domain	*/

#ifndef _AGAR_CORE_CORE_H_
#define _AGAR_CORE_CORE_H_

#if !defined(AG_BIG_ENDIAN) && !defined(AG_LITTLE_ENDIAN)
# define AG_BIG_ENDIAN 4321
# define AG_LITTLE_ENDIAN 1234
# include <agar/config/_mk_big_endian.h>
# include <agar/config/_mk_little_endian.h>
# if defined(_MK_BIG_ENDIAN)
#  define AG_BYTEORDER AG_BIG_ENDIAN
# elif defined(_MK_LITTLE_ENDIAN)
#  define AG_BYTEORDER AG_LITTLE_ENDIAN
# else
#  error "Byte order is unknown"
# endif
# undef _MK_BIG_ENDIAN
# undef _MK_LITTLE_ENDIAN
#endif

#if !defined(_AGAR_INTERNAL)

# include <agar/core/options.h>
# include <agar/core/error.h>
# include <agar/core/threads.h>
# include <agar/core/types.h>
# include <agar/core/attributes.h>
# include <agar/core/limits.h>
# include <agar/core/queue.h>
# include <agar/core/vec.h>
# include <agar/core/cpuinfo.h>
# include <agar/core/core_init.h>
# include <agar/core/agsi.h>

#else /* _AGAR_INTERNAL */

/* For inlinables (TODO move to inline_*.h) */
# include <agar/config/_mk_have_stdlib_h.h>
# ifdef _MK_HAVE_STDLIB_H
#  include <stdlib.h>
# endif
# include <agar/config/_mk_have_unistd_h.h>
# ifdef _MK_HAVE_UNISTD_H
#  include <unistd.h>
# endif
# include <string.h>
# include <stdio.h>

# include <agar/core/types.h>
# include <agar/core/options.h>
# include <agar/core/core_init.h>
# include <agar/core/agsi.h>
# include <agar/core/error.h>
# include <agar/core/queue.h>
# include <agar/core/vec.h>
# include <agar/core/limits.h>
# include <agar/core/threads.h>
# include <agar/core/string.h>
# include <agar/core/agsi.h>
# include <agar/core/snprintf.h>
# include <agar/core/vsnprintf.h>
# include <agar/core/vasprintf.h>

/* NOTE: Inlinables must use AG_MIN() and AG_MAX() */
# ifndef MIN
# define MIN(a,b) (((a)<(b))?(a):(b))
# endif
# ifndef MAX
# define MAX(a,b) (((a)>(b))?(a):(b))
# endif
# ifndef MIN3
# define MIN3(a,b,c) MIN((a),MIN((b),(c)))
# endif
# ifndef MAX3
# define MAX3(a,b,c) MAX((a),MAX((b),(c)))
# endif

# ifdef _MSC_VER
#  pragma warning(disable: 4018)
#  pragma warning(disable: 4267)
#  pragma warning(disable: 4244)
# endif

# include <agar/core/byteswap.h>

# ifdef AG_SERIALIZATION
#  include <agar/core/data_source.h>
#  include <agar/core/load_integral.h>
#  ifdef AG_HAVE_FLOAT
#   include <agar/core/load_real.h>
#  endif
#  include <agar/core/load_string.h>
# endif
# include <agar/core/load_version.h>

# include <agar/core/version.h>
# include <agar/core/object.h>
# include <agar/core/text.h>
# include <agar/core/tbl.h>
# include <agar/core/cpuinfo.h>
# include <agar/core/file.h>
# include <agar/core/dir.h>
# include <agar/core/dso.h>
# include <agar/core/db.h>
# include <agar/core/exec.h>
# include <agar/core/user.h>

#endif /* _AGAR_INTERNAL */

#endif /* !_AGAR_CORE_CORE_H_ */
