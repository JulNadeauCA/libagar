/*	Public domain	*/

#ifndef DEBUG
# include <agar/config/debug.h>
# define _AGAR_DEBUG_
#endif
#ifndef LOCKDEBUG
# include <agar/config/lockdebug.h>
# define _AGAR_LOCKDEBUG_
#endif
#ifndef THREADS
# include <agar/config/threads.h>
# define _AGAR_THREADS_
#endif
#ifndef _AGAR_HAVE_SYS_TYPES_H
# include <agar/config/_mk_have_sys_types_h.h>
# define _AGAR_HAVE_SYS_TYPES_H_
#endif
#ifndef _AGAR_HAVE_64BIT_H
# include <agar/config/have_64bit.h>
# define _AGAR_HAVE_64BIT_H_
# ifdef HAVE_64BIT
#  define AG_HAVE_64BIT
# endif
#endif
#ifndef _AGAR_HAVE_LONG_DOUBLE_H_
# include <agar/config/have_long_double.h>
# define _AGAR_HAVE_LONG_DOUBLE_H_
# ifdef HAVE_LONG_DOUBLE
#  define AG_HAVE_LONG_DOUBLE
# endif
#endif
#ifndef _AGAR_HAVE_STDLIB_H
# include <agar/config/_mk_have_stdlib_h.h>
# define _AGAR_HAVE_STDLIB_H_
#endif
#ifndef _AGAR_HAVE_UNISTD_H
# include <agar/config/_mk_have_unistd_h.h>
# define _AGAR_HAVE_UNISTD_H_
#endif

#include <agar/core/threads.h>

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef _MK_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef _MK_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>	/* For use of FILE in headers */
#include <SDL.h>	/* For use of SDL types in headers */

#include <agar/core/limits.h>

#include <agar/core/queue.h>	/* For FOO_ENTRY macros */
#include <agar/core/cpuinfo.h>	/* For arch extension defines */

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
