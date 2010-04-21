/*	Public domain	*/

#include <agar/config/ag_debug.h>
#include <agar/config/ag_legacy.h>
#include <agar/config/ag_threads.h>

#include <agar/core/threads.h>

#include <agar/config/_mk_have_stdlib_h.h>
#ifdef _MK_HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <agar/config/_mk_have_unistd_h.h>
#ifdef _MK_HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>

#include <agar/core/limits.h>
#include <agar/core/queue.h>
#include <agar/core/cpuinfo.h>

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
#endif /* !AG_BIG_ENDIAN && !AG_LITTLE_ENDIAN */
