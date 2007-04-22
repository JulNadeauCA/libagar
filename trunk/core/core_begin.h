/*	$Csoft$	*/
/*	Public domain	*/

#ifndef THREADS
# include <agar/config/threads.h>
# define _AGAR_THREADS_
#endif
#ifndef _AGAR_HAVE_SYS_TYPES_H
# include <agar/config/_mk_have_sys_types_h.h>
# define _AGAR_HAVE_SYS_TYPES_H_
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

#include <agar/core/threads.h>

#ifdef _AGAR_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define Uchar unsigned char
#define Uint unsigned int
#define Ulong unsigned long
#endif

#include <SDL.h>

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

#ifdef WIN32
#define AG_PATHSEPC '\\'
#define AG_PATHSEP "\\"
#else
#define AG_PATHSEPC '/'
#define AG_PATHSEP "/"
#endif

#include <agar/compat/queue.h>
