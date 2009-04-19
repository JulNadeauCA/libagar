/*	Public domain	*/

/*
 * Definitions of primitive integer types used throughout Agar. Also
 * defines AG_HAVE_64BIT and AG_HAVE_LONG_DOUBLE.
 */

#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
# undef _MK_HAVE_SYS_TYPES_H
# include <sys/types.h>
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

#ifndef Uint
#define Uint unsigned int
#endif
#ifndef Uchar
#define Uchar unsigned char
#endif
#ifndef Ulong
#define Ulong unsigned long
#endif
#ifndef Sint8
#define Sint8 int8_t
#endif
#ifndef Uint8
#define Uint8 u_int8_t
#endif
#ifndef Sint16
#define Sint16 int16_t
#endif
#ifndef Uint16
#define Uint16 u_int16_t
#endif
#ifndef Sint32
#define Sint32 int32_t
#endif
#ifndef Uint32
#define Uint32 u_int32_t
#endif
#ifdef HAVE_64BIT
# ifndef Sint64
# define Sint64 int64_t
# endif
# ifndef Uint64
# define Uint64 u_int64_t
# endif
#else /* !HAVE_64BIT */
typedef struct ag_fake_int64 { Uint32 _pad1; Uint32 _pad2; };
# ifndef Sint64
# define Sint64 struct ag_fake_int64
# endif
# ifndef Uint64
# define Uint64 struct ag_fake_int64
# endif
#endif /* HAVE_64BIT */
