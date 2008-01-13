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
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
# include <agar/config/_mk_have_unsigned_typedefs.h>
# define _AGAR_HAVE_UNSIGNED_TYPEDEFS_
#endif

#ifndef HAVE_BOUNDED_ATTRIBUTE
# include <agar/config/have_bounded_attribute.h>
# define _AGAR_HAVE_BOUNDED_ATTRIBUTE_
#endif
#ifndef HAVE_FORMAT_ATTRIBUTE
# include <agar/config/have_format_attribute.h>
# define _AGAR_HAVE_FORMAT_ATTRIBUTE_
#endif
#ifndef HAVE_NONNULL_ATTRIBUTE
# include <agar/config/have_nonnull_attribute.h>
# define _AGAR_HAVE_NONNULL_ATTRIBUTE_
#endif
#ifndef HAVE_PACKED_ATTRIBUTE
# include <agar/config/have_packed_attribute.h>
# define _AGAR_HAVE_PACKED_ATTRIBUTE_
#endif
#ifndef HAVE_ALIGNED_ATTRIBUTE
# include <agar/config/have_aligned_attribute.h>
# define _AGAR_HAVE_ALIGNED_ATTRIBUTE_
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

#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define Uchar unsigned char
#define Uint unsigned int
#define Ulong unsigned long
#endif

#ifdef HAVE_BOUNDED_ATTRIBUTE
#define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
#else
#define BOUNDED_ATTRIBUTE(t, a, b)
#endif
#ifdef HAVE_FORMAT_ATTRIBUTE
#define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
#else
#define FORMAT_ATTRIBUTE(t, a, b)
#endif
#ifdef HAVE_NONNULL_ATTRIBUTE
#define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
#else
#define NONNULL_ATTRIBUTE(a)
#endif
#ifdef HAVE_PACKED_ATTRIBUTE
#define PACKED_ATTRIBUTE __attribute__((__packed__))
#else
#define PACKED_ATTRIBUTE
#endif
#ifdef HAVE_ALIGNED_ATTRIBUTE
#define ALIGNED_ATTRIBUTE(a) __attribute__((__aligned__ (a)))
#else
#define ALIGNED_ATTRIBUTE(a)
#endif

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
# define _AGAR_DEFINED_CC_DECLS_
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#define _AGAR_DEFINED_MAXPATHLEN_
#endif

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

